#include "emotion/EmotionDetector.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace Chatbot {

QString emotionToString(Emotion emotion) {
    switch (emotion) {
        case Emotion::Neutral:     return "Neutral";
        case Emotion::Happy:       return "Happy";
        case Emotion::Sad:         return "Sad";
        case Emotion::Surprised:   return "Surprised";
        case Emotion::Thoughtful:  return "Thoughtful";
        case Emotion::Worried:     return "Worried";
        default:                   return "Unknown";
    }
}

EmotionDetector::EmotionDetector()
    : m_lastConfidence(0.0f)
{
    initializeKeywords();
    spdlog::info("EmotionDetector initialized with keyword-based analysis");
}

void EmotionDetector::initializeKeywords() {
    // Happy keywords
    m_emotionKeywords[Emotion::Happy] = {
        "happy", "great", "wonderful", "excellent", "fantastic", "amazing",
        "glad", "joy", "delighted", "pleased", "excited", "love",
        "awesome", "perfect", "brilliant", "congratulations", "celebrate",
        "fun", "enjoy", "smile", "laugh", "nice", "good"
    };

    // Sad keywords
    m_emotionKeywords[Emotion::Sad] = {
        "sad", "sorry", "unfortunate", "regret", "disappointed", "miss",
        "loss", "difficult", "hard", "tough", "struggle", "pain",
        "hurt", "cry", "unhappy", "depressed", "down", "blue",
        "terrible", "awful", "bad", "poor"
    };

    // Surprised keywords
    m_emotionKeywords[Emotion::Surprised] = {
        "wow", "amazing", "incredible", "unbelievable", "shocking", "unexpected",
        "surprise", "astonish", "remarkable", "extraordinary", "stunning",
        "whoa", "really", "seriously", "no way", "can't believe"
    };

    // Thoughtful keywords
    m_emotionKeywords[Emotion::Thoughtful] = {
        "think", "consider", "perhaps", "maybe", "possibly", "might",
        "could", "wonder", "question", "curious", "interesting", "hmm",
        "let me", "analyze", "examine", "ponder", "reflect", "contemplate",
        "understand", "learn", "explore", "investigate"
    };

    // Worried keywords
    m_emotionKeywords[Emotion::Worried] = {
        "worried", "concern", "afraid", "fear", "anxious", "nervous",
        "stress", "trouble", "problem", "issue", "danger", "risk",
        "careful", "caution", "warning", "alert", "uncertain", "unsure",
        "doubt", "hesitant", "worry"
    };
}

Emotion EmotionDetector::detectEmotion(const QString& text) const {
    QString lowerText = text.toLower();

    // Count keyword matches for each emotion
    QMap<Emotion, int> scores;
    for (auto it = m_emotionKeywords.constBegin(); it != m_emotionKeywords.constEnd(); ++it) {
        scores[it.key()] = countKeywordMatches(lowerText, it.value());
    }

    // Find emotion with highest score
    Emotion detectedEmotion = Emotion::Neutral;
    int maxScore = 0;

    for (auto it = scores.constBegin(); it != scores.constEnd(); ++it) {
        if (it.value() > maxScore) {
            maxScore = it.value();
            detectedEmotion = it.key();
        }
    }

    // Calculate confidence based on score
    // Higher scores = higher confidence, cap at 1.0
    m_lastConfidence = std::min(1.0f, maxScore * 0.3f);

    // If confidence is too low, default to neutral
    if (m_lastConfidence < 0.2f) {
        detectedEmotion = Emotion::Neutral;
        m_lastConfidence = 1.0f; // High confidence in neutrality
    }

    spdlog::debug("Detected emotion: {} (confidence: {:.2f}, score: {})",
                  emotionToString(detectedEmotion).toStdString(),
                  m_lastConfidence, maxScore);

    return detectedEmotion;
}

int EmotionDetector::countKeywordMatches(const QString& text, const QStringList& keywords) const {
    int count = 0;
    for (const QString& keyword : keywords) {
        // Count occurrences of this keyword (case-insensitive)
        int index = 0;
        while ((index = text.indexOf(keyword, index, Qt::CaseInsensitive)) != -1) {
            count++;
            index += keyword.length();
        }
    }
    return count;
}

} // namespace Chatbot
