#ifndef CHATBOT_CHATENGINE_H
#define CHATBOT_CHATENGINE_H

#include <QObject>
#include <QString>
#include <QThread>
#include <memory>

namespace Chatbot {

class ConversationHistory;

/**
 * ChatEngine handles communication with the LLM (Ollama API)
 * and manages conversation state.
 */
class ChatEngine : public QObject {
    Q_OBJECT

public:
    explicit ChatEngine(QObject *parent = nullptr);
    ~ChatEngine() override;

    // Configuration
    void setOllamaUrl(const QString& url);
    void setModel(const QString& model);
    void setSystemPrompt(const QString& prompt);

    // Chat operations
    void sendMessage(const QString& message);
    void clearHistory();

    // Status
    bool isProcessing() const { return m_isProcessing; }
    QString currentModel() const { return m_model; }

signals:
    void responseReceived(const QString& response);
    void errorOccurred(const QString& error);
    void processingStarted();
    void processingFinished();

private:
    void processMessageAsync(const QString& message);
    QString callOllamaAPI(const QString& prompt);

private:
    QString m_ollamaUrl;
    QString m_model;
    QString m_systemPrompt;
    bool m_isProcessing;

    std::unique_ptr<ConversationHistory> m_history;
    std::unique_ptr<QThread> m_workerThread;
};

} // namespace Chatbot

#endif // CHATBOT_CHATENGINE_H
