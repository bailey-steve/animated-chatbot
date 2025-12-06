#include "chat/ConversationHistory.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace Chatbot {

ConversationHistory::ConversationHistory()
    : m_maxMessages(100)  // Keep last 100 messages by default
{
    spdlog::debug("ConversationHistory created (max messages: {})", m_maxMessages);
}

ConversationHistory::~ConversationHistory() {
    spdlog::debug("ConversationHistory destroyed");
}

void ConversationHistory::addUserMessage(const QString& message) {
    m_messages.emplace_back("user", message.toStdString());
    spdlog::debug("User message added to history (total: {})", m_messages.size());

    // Trim history if needed
    if (m_maxMessages > 0 && m_messages.size() > m_maxMessages) {
        m_messages.erase(m_messages.begin());
        spdlog::debug("History trimmed to {} messages", m_messages.size());
    }
}

void ConversationHistory::addBotMessage(const QString& message) {
    m_messages.emplace_back("assistant", message.toStdString());
    spdlog::debug("Bot message added to history (total: {})", m_messages.size());

    // Trim history if needed
    if (m_maxMessages > 0 && m_messages.size() > m_maxMessages) {
        m_messages.erase(m_messages.begin());
        spdlog::debug("History trimmed to {} messages", m_messages.size());
    }
}

std::vector<Message> ConversationHistory::getRecentMessages(size_t count) const {
    if (count >= m_messages.size()) {
        return m_messages;
    }

    return std::vector<Message>(
        m_messages.end() - count,
        m_messages.end()
    );
}

void ConversationHistory::clear() {
    size_t oldSize = m_messages.size();
    m_messages.clear();
    spdlog::info("Conversation history cleared ({} messages removed)", oldSize);
}

} // namespace Chatbot
