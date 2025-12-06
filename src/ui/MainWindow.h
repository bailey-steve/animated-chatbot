#ifndef CHATBOT_MAINWINDOW_H
#define CHATBOT_MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace Chatbot {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Add message to chat display
    void addUserMessage(const QString& message);
    void addBotMessage(const QString& message);
    void addSystemMessage(const QString& message);

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
    QVBoxLayout* m_mainLayout;

    // Chat display
    QTextEdit* m_chatDisplay;

    // Input area
    QWidget* m_inputWidget;
    QHBoxLayout* m_inputLayout;
    QLineEdit* m_inputField;
    QPushButton* m_sendButton;
};

} // namespace Chatbot

#endif // CHATBOT_MAINWINDOW_H
