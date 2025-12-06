#ifndef CHATBOT_EVENTBUS_H
#define CHATBOT_EVENTBUS_H

#include <QObject>
#include <QString>

namespace Chatbot {

/**
 * EventBus provides a centralized communication mechanism between components
 * using Qt's signal/slot system.
 */
class EventBus : public QObject {
    Q_OBJECT

public:
    static EventBus& instance() {
        static EventBus instance;
        return instance;
    }

    // Delete copy constructor and assignment
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

signals:
    // Chat events
    void userMessageSent(const QString& message);
    void botResponseReceived(const QString& response);
    void chatError(const QString& error);

    // TTS events
    void ttsStarted();
    void ttsFinished();
    void ttsError(const QString& error);

    // Avatar events
    void avatarAnimationStarted();
    void avatarAnimationFinished();

    // Emotion events
    void emotionDetected(const QString& emotion);

    // Personality events
    void personalityChanged(const QString& personalityId);

private:
    EventBus() : QObject(nullptr) {}
    ~EventBus() override = default;
};

} // namespace Chatbot

#endif // CHATBOT_EVENTBUS_H
