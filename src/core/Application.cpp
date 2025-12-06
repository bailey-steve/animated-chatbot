#include "core/Application.h"
#include "ui/MainWindow.h"
#include "chat/ChatEngine.h"
#include "tts/TTSEngine.h"
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

    // Create MainWindow
    m_mainWindow = std::make_unique<MainWindow>();
    spdlog::info("MainWindow initialized");
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
