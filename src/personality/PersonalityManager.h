#ifndef CHATBOT_PERSONALITYMANAGER_H
#define CHATBOT_PERSONALITYMANAGER_H

#include "emotion/EmotionDetector.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>

namespace Chatbot {

// Personality configuration data
struct Personality {
    QString name;
    QString description;
    QString systemPrompt;
    QString voiceStyle;
    Emotion defaultEmotion;

    // Personality traits (0.0 to 1.0)
    float warmth;
    float formality;
    float verbosity;
    float humor;
};

// Manages loading and switching between personalities
class PersonalityManager : public QObject {
    Q_OBJECT

public:
    explicit PersonalityManager(QObject *parent = nullptr);
    ~PersonalityManager() override = default;

    // Load all personalities from config directory
    bool loadPersonalities(const QString& configDir = "./config/personalities");

    // Get list of available personality names
    QStringList getAvailablePersonalities() const;

    // Get personality by name
    Personality getPersonality(const QString& name) const;

    // Get current personality
    Personality getCurrentPersonality() const { return m_currentPersonality; }
    QString getCurrentPersonalityName() const { return m_currentPersonalityName; }

    // Set current personality
    bool setPersonality(const QString& name);

signals:
    void personalityChanged(const QString& personalityName);
    void personalityLoaded(const QString& personalityName);
    void errorOccurred(const QString& error);

private:
    bool loadPersonalityFile(const QString& filePath);
    Emotion stringToEmotion(const QString& emotionStr) const;

private:
    QMap<QString, Personality> m_personalities;
    QString m_currentPersonalityName;
    Personality m_currentPersonality;
};

} // namespace Chatbot

#endif // CHATBOT_PERSONALITYMANAGER_H
