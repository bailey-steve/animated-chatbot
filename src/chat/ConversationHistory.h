#ifndef CHATBOT_CONVERSATIONHISTORY_H
#define CHATBOT_CONVERSATIONHISTORY_H

#include <QString>
#include <QDateTime>
#include <vector>
#include <string>

namespace Chatbot {

struct Message {
    std::string role;        // "user" or "assistant"
    std::string content;
    QDateTime timestamp;

    Message(const std::string& r, const std::string& c)
        : role(r), content(c), timestamp(QDateTime::currentDateTime()) {}
};

/**
 * ConversationHistory manages the chat message history
 */
class ConversationHistory {
public:
    ConversationHistory();
    ~ConversationHistory();

    // Add messages
    void addUserMessage(const QString& message);
    void addBotMessage(const QString& message);

    // Get messages
    const std::vector<Message>& getMessages() const { return m_messages; }
    std::vector<Message> getRecentMessages(size_t count) const;

    // Clear history
    void clear();

    // Get statistics
    size_t messageCount() const { return m_messages.size(); }
    bool isEmpty() const { return m_messages.empty(); }

private:
    std::vector<Message> m_messages;
    size_t m_maxMessages;  // Maximum messages to keep (0 = unlimited)
};

} // namespace Chatbot

#endif // CHATBOT_CONVERSATIONHISTORY_H
