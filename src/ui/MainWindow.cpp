#include "ui/MainWindow.h"
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <spdlog/spdlog.h>

namespace Chatbot {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_chatDisplay(nullptr)
    , m_inputWidget(nullptr)
    , m_inputLayout(nullptr)
    , m_inputField(nullptr)
    , m_sendButton(nullptr)
{
    setupUI();
    setupConnections();

    spdlog::info("MainWindow created");
}

MainWindow::~MainWindow() {
    spdlog::info("MainWindow destroyed");
}

void MainWindow::setupUI() {
    // Set window properties
    setWindowTitle("Chatbot - Animated 3D Assistant");
    resize(800, 600);

    // Center window on screen
    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    const int x = (screenGeometry.width() - width()) / 2;
    const int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Create central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // Create main layout
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // Create chat display
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setStyleSheet(
        "QTextEdit {"
        "  background-color: #ffffff;"
        "  color: #000000;"
        "  border: 1px solid #ddd;"
        "  border-radius: 5px;"
        "  padding: 10px;"
        "  font-size: 14px;"
        "  font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
    );
    m_mainLayout->addWidget(m_chatDisplay, 1); // Stretch factor 1

    // Create input widget
    m_inputWidget = new QWidget(this);
    m_inputLayout = new QHBoxLayout(m_inputWidget);
    m_inputLayout->setContentsMargins(0, 0, 0, 0);
    m_inputLayout->setSpacing(10);

    // Create input field
    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("Type your message here...");
    m_inputField->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #ddd;"
        "  border-radius: 5px;"
        "  padding: 10px;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid #4CAF50;"
        "}"
    );
    m_inputLayout->addWidget(m_inputField, 1); // Stretch factor 1

    // Create send button
    m_sendButton = new QPushButton("Send", this);
    m_sendButton->setFixedWidth(100);
    m_sendButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 10px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #3d8b40;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "}"
    );
    m_inputLayout->addWidget(m_sendButton);

    m_mainLayout->addWidget(m_inputWidget);

    // Add welcome message
    addSystemMessage("Welcome to Chatbot! Type a message to start chatting.");
}

void MainWindow::setupConnections() {
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(m_inputField, &QLineEdit::returnPressed, this, &MainWindow::onInputReturnPressed);
}

void MainWindow::onSendButtonClicked() {
    QString message = m_inputField->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    addUserMessage(message);
    emit userMessageSubmitted(message);
    clearInput();
}

void MainWindow::onInputReturnPressed() {
    onSendButtonClicked();
}

void MainWindow::clearInput() {
    m_inputField->clear();
    m_inputField->setFocus();
}

void MainWindow::addUserMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString html = QString(
        "<div style='margin-bottom: 10px;'>"
        "  <span style='color: #888; font-size: 12px;'>%1</span><br>"
        "  <span style='color: #2196F3; font-weight: bold;'>You:</span> "
        "  <span style='color: #000;'>%2</span>"
        "</div>"
    ).arg(timestamp, message.toHtmlEscaped());

    m_chatDisplay->append(html);
    m_chatDisplay->ensureCursorVisible();
}

void MainWindow::addBotMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString html = QString(
        "<div style='margin-bottom: 10px;'>"
        "  <span style='color: #888; font-size: 12px;'>%1</span><br>"
        "  <span style='color: #4CAF50; font-weight: bold;'>Bot:</span> "
        "  <span style='color: #000;'>%2</span>"
        "</div>"
    ).arg(timestamp, message.toHtmlEscaped());

    m_chatDisplay->append(html);
    m_chatDisplay->ensureCursorVisible();
}

void MainWindow::addSystemMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString html = QString(
        "<div style='margin-bottom: 10px;'>"
        "  <span style='color: #888; font-size: 12px;'>%1</span><br>"
        "  <span style='color: #FF9800; font-weight: bold;'>System:</span> "
        "  <span style='color: #333; font-style: italic;'>%2</span>"
        "</div>"
    ).arg(timestamp, message.toHtmlEscaped());

    m_chatDisplay->append(html);
    m_chatDisplay->ensureCursorVisible();
}

} // namespace Chatbot
