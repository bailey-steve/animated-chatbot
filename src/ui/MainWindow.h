#ifndef CHATBOT_MAINWINDOW_H
#define CHATBOT_MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

namespace Chatbot {

// Forward declaration
class AvatarViewport;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Add message to chat display
    void addUserMessage(const QString& message);
    void addBotMessage(const QString& message);
    void addSystemMessage(const QString& message);

    // Get avatar viewport
    AvatarViewport* getAvatarViewport() const { return m_avatarViewport; }

signals:
    void userMessageSubmitted(const QString& message);

private slots:
    void onSendButtonClicked();
    void onInputReturnPressed();

private:
    void setupUI();
    void setupConnections();
    void clearInput();

private:
    // UI Components
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    QSplitter* m_splitter;

    // Avatar viewport
    AvatarViewport* m_avatarViewport;

    // Chat panel
    QWidget* m_chatPanel;
    QVBoxLayout* m_chatLayout;
    QTextEdit* m_chatDisplay;

    // Input area
    QWidget* m_inputWidget;
    QHBoxLayout* m_inputLayout;
    QLineEdit* m_inputField;
    QPushButton* m_sendButton;
};

} // namespace Chatbot

#endif // CHATBOT_MAINWINDOW_H
