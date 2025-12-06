#ifndef CHATBOT_EMOTIONDETECTOR_H
#define CHATBOT_EMOTIONDETECTOR_H

#include <QString>
#include <QMap>
#include <QStringList>

namespace Chatbot {

// Emotion types the avatar can express
enum class Emotion {
    Neutral,
    Happy,
    Sad,
    Surprised,
    Thoughtful,
    Worried
};

// Converts Emotion enum to string
QString emotionToString(Emotion emotion);

// Emotion detection based on text analysis
class EmotionDetector {
public:
    EmotionDetector();
    ~EmotionDetector() = default;

    // Analyze text and return detected emotion
    Emotion detectEmotion(const QString& text) const;

    // Get confidence score for detected emotion (0.0 to 1.0)
    float getConfidence() const { return m_lastConfidence; }

private:
    void initializeKeywords();
    int countKeywordMatches(const QString& text, const QStringList& keywords) const;

private:
    // Keyword lists for each emotion
    QMap<Emotion, QStringList> m_emotionKeywords;
    mutable float m_lastConfidence;
};

} // namespace Chatbot

#endif // CHATBOT_EMOTIONDETECTOR_H
