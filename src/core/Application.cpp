#include "core/Application.h"
#include "ui/MainWindow.h"
#include "ui/AvatarViewport.h"
#include "chat/ChatEngine.h"
#include "tts/TTSEngine.h"
#include "avatar/AvatarEngine.h"
#include "emotion/EmotionDetector.h"
#include "personality/PersonalityManager.h"
#include <spdlog/spdlog.h>

namespace Chatbot {

Application* Application::s_instance = nullptr;

Application::Application(int argc, char *argv[])
    : QObject(nullptr)
    , m_argc(argc)
    , m_argv(argv)
{
    if (s_instance) {
        spdlog::error("Application instance already exists!");
        throw std::runtime_error("Application instance already exists!");
    }
    s_instance = this;

    // Initialize spdlog
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Initializing Chatbot Application v1.0.0");

    // Create QApplication
    m_qApp = std::make_unique<QApplication>(m_argc, m_argv);
    m_qApp->setApplicationName("Chatbot");
    m_qApp->setApplicationVersion("1.0.0");
    m_qApp->setOrganizationName("Chatbot");

    initializeComponents();
    setupConnections();
}

Application::~Application() {
    spdlog::info("Shutting down Chatbot Application");
    s_instance = nullptr;
}

Application* Application::instance() {
    return s_instance;
}

void Application::initializeComponents() {
    spdlog::info("Initializing components...");

    // Create ChatEngine
    m_chatEngine = std::make_unique<ChatEngine>();
    spdlog::info("ChatEngine initialized");

    // Create TTSEngine
    m_ttsEngine = std::make_unique<TTSEngine>();
    spdlog::info("TTSEngine initialized");

    // Create EmotionDetector
    m_emotionDetector = std::make_unique<EmotionDetector>();
    spdlog::info("EmotionDetector initialized");

    // Create PersonalityManager
    m_personalityManager = std::make_unique<PersonalityManager>();
    m_personalityManager->loadPersonalities();
    spdlog::info("PersonalityManager initialized");

    // Create MainWindow
    m_mainWindow = std::make_unique<MainWindow>();
    spdlog::info("MainWindow initialized");

    // Populate personality selector
    QStringList personalities = m_personalityManager->getAvailablePersonalities();
    QComboBox* selector = m_mainWindow->getPersonalitySelector();
    for (const QString& personality : personalities) {
        selector->addItem(personality);
    }
    // Set to current personality
    QString currentPersonality = m_personalityManager->getCurrentPersonalityName();
    int index = selector->findText(currentPersonality);
    if (index >= 0) {
        selector->setCurrentIndex(index);
    }
}

void Application::setupConnections() {
    spdlog::info("Setting up component connections...");

    // Connect MainWindow to ChatEngine
    QObject::connect(m_mainWindow.get(), &MainWindow::userMessageSubmitted,
                    m_chatEngine.get(), &ChatEngine::sendMessage);

    // Connect ChatEngine to MainWindow
    QObject::connect(m_chatEngine.get(), &ChatEngine::responseReceived,
                    m_mainWindow.get(), &MainWindow::addBotMessage);

    QObject::connect(m_chatEngine.get(), &ChatEngine::errorOccurred,
                    m_mainWindow.get(), &MainWindow::addSystemMessage);

    QObject::connect(m_chatEngine.get(), &ChatEngine::processingStarted,
                    m_mainWindow.get(), [this]() {
                        m_mainWindow->addSystemMessage("Thinking...");
                    });

    // Connect ChatEngine to TTSEngine (speak bot responses)
    QObject::connect(m_chatEngine.get(), &ChatEngine::responseReceived,
                    m_ttsEngine.get(), &TTSEngine::synthesize);

    // Connect TTSEngine to MainWindow (status messages)
    QObject::connect(m_ttsEngine.get(), &TTSEngine::synthesisStarted,
                    m_mainWindow.get(), [this]() {
                        m_mainWindow->addSystemMessage("Speaking...");
                    });

    QObject::connect(m_ttsEngine.get(), &TTSEngine::errorOccurred,
                    m_mainWindow.get(), &MainWindow::addSystemMessage);

    // Connect TTSEngine to AvatarEngine (lip-sync)
    auto avatarEngine = m_mainWindow->getAvatarViewport()->getAvatarEngine();
    if (avatarEngine) {
        QObject::connect(m_ttsEngine.get(), &TTSEngine::currentPhoneme,
                        avatarEngine, [avatarEngine](const Phoneme& phoneme, int index) {
                            avatarEngine->applyPhoneme(phoneme.symbol);
                        });

        QObject::connect(m_ttsEngine.get(), &TTSEngine::playbackFinished,
                        avatarEngine, [avatarEngine]() {
                            // Return to silence/rest position when done speaking
                            avatarEngine->applyPhoneme("");
                        });
        spdlog::info("Lip-sync connections established");

        // Connect ChatEngine to EmotionDetector to AvatarEngine (emotional expressions)
        QObject::connect(m_chatEngine.get(), &ChatEngine::responseReceived,
                        this, [this, avatarEngine](const QString& response) {
                            // Detect emotion from bot response text
                            Emotion emotion = m_emotionDetector->detectEmotion(response);
                            // Apply emotion to avatar
                            avatarEngine->applyEmotion(emotion);
                        });
        spdlog::info("Emotion detection connections established");
    }

    // Connect MainWindow personality selector to PersonalityManager and ChatEngine
    QObject::connect(m_mainWindow.get(), &MainWindow::personalitySelected,
                    this, [this, avatarEngine](const QString& personalityName) {
                        // Change personality in manager
                        if (m_personalityManager->setPersonality(personalityName)) {
                            Personality personality = m_personalityManager->getCurrentPersonality();

                            // Update ChatEngine system prompt
                            m_chatEngine->setSystemPrompt(personality.systemPrompt);

                            // Update avatar default emotion
                            if (avatarEngine) {
                                avatarEngine->applyEmotion(personality.defaultEmotion);
                            }

                            // Show system message
                            QString message = QString("Switched to %1 personality").arg(personalityName);
                            m_mainWindow->addSystemMessage(message);

                            spdlog::info("Personality switched to: {}", personalityName.toStdString());
                        }
                    });
    spdlog::info("Personality system connections established");

    spdlog::info("Connections established");
}

int Application::run() {
    spdlog::info("Starting application...");

    // Show main window
    m_mainWindow->show();

    spdlog::info("Application running");
    return m_qApp->exec();
}

} // namespace Chatbot
