#ifndef CHATBOT_APPLICATION_H
#define CHATBOT_APPLICATION_H

#include <QApplication>
#include <QObject>
#include <memory>

namespace Chatbot {

// Forward declarations
class MainWindow;
class ChatEngine;
class TTSEngine;
class EmotionDetector;

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(int argc, char *argv[]);
    ~Application() override;

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Run the application
    int run();

    // Get singleton instance
    static Application* instance();

private:
    void initializeComponents();
    void setupConnections();

private:
    static Application* s_instance;

    std::unique_ptr<QApplication> m_qApp;
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<ChatEngine> m_chatEngine;
    std::unique_ptr<TTSEngine> m_ttsEngine;
    std::unique_ptr<EmotionDetector> m_emotionDetector;

    int m_argc;
    char** m_argv;
};

} // namespace Chatbot

#endif // CHATBOT_APPLICATION_H
