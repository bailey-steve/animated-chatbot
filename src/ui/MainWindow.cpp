#include "ui/MainWindow.h"
#include "ui/AvatarViewport.h"
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include <QSplitter>
#include <spdlog/spdlog.h>

namespace Chatbot {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_avatarViewport(nullptr)
    , m_chatPanel(nullptr)
    , m_chatLayout(nullptr)
    , m_chatDisplay(nullptr)
    , m_personalityWidget(nullptr)
    , m_personalityLayout(nullptr)
    , m_personalityLabel(nullptr)
    , m_personalitySelector(nullptr)
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
    resize(1200, 700);  // Wider to accommodate avatar + chat

    // Center window on screen
    const QRect screenGeometry = QApplication::primaryScreen()->geometry();
    const int x = (screenGeometry.width() - width()) / 2;
    const int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Create central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // Create main layout (horizontal)
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);

    // Create splitter for avatar and chat
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // Create avatar viewport
    m_avatarViewport = new AvatarViewport(this);
    m_splitter->addWidget(m_avatarViewport);

    // Create chat panel
    m_chatPanel = new QWidget(this);
    m_chatLayout = new QVBoxLayout(m_chatPanel);
    m_chatLayout->setContentsMargins(0, 0, 0, 0);
    m_chatLayout->setSpacing(10);

    // Create personality selector
    m_personalityWidget = new QWidget(this);
    m_personalityLayout = new QHBoxLayout(m_personalityWidget);
    m_personalityLayout->setContentsMargins(0, 0, 0, 0);
    m_personalityLayout->setSpacing(10);

    m_personalityLabel = new QLabel("Personality:", this);
    m_personalityLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    m_personalityLayout->addWidget(m_personalityLabel);

    m_personalitySelector = new QComboBox(this);
    m_personalitySelector->setStyleSheet(
        "QComboBox {"
        "  border: 1px solid #ddd;"
        "  border-radius: 5px;"
        "  padding: 5px 10px;"
        "  font-size: 14px;"
        "  min-width: 150px;"
        "}"
        "QComboBox:hover {"
        "  border: 1px solid #4CAF50;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "}"
    );
    m_personalityLayout->addWidget(m_personalitySelector);
    m_personalityLayout->addStretch(); // Push to left

    m_chatLayout->addWidget(m_personalityWidget);

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
    m_chatLayout->addWidget(m_chatDisplay, 1); // Stretch factor 1

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

    m_chatLayout->addWidget(m_inputWidget);

    m_splitter->addWidget(m_chatPanel);

    // Set initial splitter sizes (40% avatar, 60% chat)
    m_splitter->setStretchFactor(0, 2);
    m_splitter->setStretchFactor(1, 3);

    m_mainLayout->addWidget(m_splitter);

    // Add welcome message
    addSystemMessage("Welcome to Chatbot! Type a message to start chatting.");
}

void MainWindow::setupConnections() {
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(m_inputField, &QLineEdit::returnPressed, this, &MainWindow::onInputReturnPressed);
    connect(m_personalitySelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPersonalityChanged);
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

void MainWindow::onPersonalityChanged(int index) {
    if (index >= 0) {
        QString personalityName = m_personalitySelector->currentText();
        spdlog::info("Personality changed to: {}", personalityName.toStdString());
        emit personalitySelected(personalityName);
    }
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
