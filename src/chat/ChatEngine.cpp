#include "chat/ChatEngine.h"
#include "chat/ConversationHistory.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

using json = nlohmann::json;

namespace Chatbot {

ChatEngine::ChatEngine(QObject *parent)
    : QObject(parent)
    , m_ollamaUrl("http://localhost:11434")
    , m_model("llama3.2:3b")
    , m_systemPrompt("You are a helpful, friendly assistant.")
    , m_isProcessing(false)
    , m_history(std::make_unique<ConversationHistory>())
{
    spdlog::info("ChatEngine initialized with model: {}", m_model.toStdString());
}

ChatEngine::~ChatEngine() {
    spdlog::info("ChatEngine destroyed");
}

void ChatEngine::setOllamaUrl(const QString& url) {
    m_ollamaUrl = url;
    spdlog::info("Ollama URL set to: {}", url.toStdString());
}

void ChatEngine::setModel(const QString& model) {
    m_model = model;
    spdlog::info("Model changed to: {}", model.toStdString());
}

void ChatEngine::setSystemPrompt(const QString& prompt) {
    m_systemPrompt = prompt;
    spdlog::info("System prompt updated");
}

void ChatEngine::clearHistory() {
    m_history->clear();
    spdlog::info("Conversation history cleared");
}

void ChatEngine::sendMessage(const QString& message) {
    if (m_isProcessing) {
        spdlog::warn("Already processing a message, ignoring new request");
        emit errorOccurred("Already processing a message. Please wait.");
        return;
    }

    if (message.trimmed().isEmpty()) {
        spdlog::warn("Empty message received");
        emit errorOccurred("Message cannot be empty");
        return;
    }

    spdlog::info("Processing user message: {}", message.toStdString());
    processMessageAsync(message);
}

void ChatEngine::processMessageAsync(const QString& message) {
    m_isProcessing = true;
    emit processingStarted();

    // Add user message to history
    m_history->addUserMessage(message);

    // Run API call in a separate thread
    QFuture<QString> future = QtConcurrent::run([this, message]() {
        return callOllamaAPI(message);
    });

    // Watch for completion
    QFutureWatcher<QString>* watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher]() {
        QString response = watcher->result();

        if (response.isEmpty()) {
            spdlog::error("Empty response from Ollama API");
            emit errorOccurred("Failed to get response from LLM");
        } else {
            // Add bot response to history
            m_history->addBotMessage(response);
            spdlog::info("Response received from LLM");
            emit responseReceived(response);
        }

        m_isProcessing = false;
        emit processingFinished();
        watcher->deleteLater();
    });

    watcher->setFuture(future);
}

QString ChatEngine::callOllamaAPI(const QString& prompt) {
    try {
        // Build request URL
        QString apiUrl = m_ollamaUrl + "/api/generate";

        // Build conversation history into the prompt
        std::string fullPrompt;
        auto messages = m_history->getMessages();

        // Add previous conversation (excluding the current prompt which is already in history)
        if (messages.size() > 1) {
            fullPrompt += "Previous conversation:\n";
            for (size_t i = 0; i < messages.size() - 1; i++) {
                const auto& msg = messages[i];
                if (msg.role == "user") {
                    fullPrompt += "User: " + msg.content + "\n";
                } else {
                    fullPrompt += "Assistant: " + msg.content + "\n";
                }
            }
            fullPrompt += "\nCurrent question:\n";
        }
        fullPrompt += prompt.toStdString();

        // Build JSON request
        json requestJson;
        requestJson["model"] = m_model.toStdString();
        requestJson["prompt"] = fullPrompt;
        requestJson["system"] = m_systemPrompt.toStdString();
        requestJson["stream"] = false;

        spdlog::debug("Sending request to Ollama: {}", apiUrl.toStdString());

        // Make HTTP POST request
        cpr::Response response = cpr::Post(
            cpr::Url{apiUrl.toStdString()},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{requestJson.dump()}
        );

        // Check response
        if (response.status_code != 200) {
            spdlog::error("Ollama API error: HTTP {}", response.status_code);
            spdlog::error("Response: {}", response.text);
            return QString();
        }

        // Parse JSON response
        json responseJson = json::parse(response.text);

        if (responseJson.contains("response")) {
            std::string llmResponse = responseJson["response"];
            return QString::fromStdString(llmResponse);
        } else {
            spdlog::error("No 'response' field in Ollama API response");
            return QString();
        }

    } catch (const std::exception& e) {
        spdlog::error("Exception in callOllamaAPI: {}", e.what());
        return QString();
    }
}

} // namespace Chatbot
