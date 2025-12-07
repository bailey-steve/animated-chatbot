#include "personality/PersonalityManager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <spdlog/spdlog.h>

namespace Chatbot {

PersonalityManager::PersonalityManager(QObject *parent)
    : QObject(parent)
{
    spdlog::info("PersonalityManager initialized");
}

bool PersonalityManager::loadPersonalities(const QString& configDir) {
    spdlog::info("Loading personalities from: {}", configDir.toStdString());

    QDir dir(configDir);
    if (!dir.exists()) {
        QString error = QString("Personalities directory not found: %1").arg(configDir);
        spdlog::error(error.toStdString());
        emit errorOccurred(error);
        return false;
    }

    // Get all JSON files
    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    if (files.isEmpty()) {
        QString error = "No personality files found in directory";
        spdlog::warn(error.toStdString());
        emit errorOccurred(error);
        return false;
    }

    int loadedCount = 0;
    for (const QFileInfo& fileInfo : files) {
        if (loadPersonalityFile(fileInfo.absoluteFilePath())) {
            loadedCount++;
        }
    }

    spdlog::info("Loaded {} personalities", loadedCount);

    // Set default personality (first one loaded, or "Friendly" if available)
    if (!m_personalities.isEmpty()) {
        QString defaultName = m_personalities.contains("Friendly") ? "Friendly" : m_personalities.firstKey();
        setPersonality(defaultName);
    }

    return loadedCount > 0;
}

bool PersonalityManager::loadPersonalityFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        spdlog::error("Failed to open personality file: {}", filePath.toStdString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        spdlog::error("Invalid JSON in personality file: {}", filePath.toStdString());
        return false;
    }

    QJsonObject obj = doc.object();

    Personality personality;
    personality.name = obj["name"].toString();
    personality.description = obj["description"].toString();
    personality.systemPrompt = obj["system_prompt"].toString();
    personality.voiceStyle = obj["voice_style"].toString();
    personality.defaultEmotion = stringToEmotion(obj["default_emotion"].toString());

    // Load personality traits
    QJsonObject traits = obj["personality_traits"].toObject();
    personality.warmth = static_cast<float>(traits["warmth"].toDouble(0.5));
    personality.formality = static_cast<float>(traits["formality"].toDouble(0.5));
    personality.verbosity = static_cast<float>(traits["verbosity"].toDouble(0.5));
    personality.humor = static_cast<float>(traits["humor"].toDouble(0.5));

    m_personalities[personality.name] = personality;
    emit personalityLoaded(personality.name);

    spdlog::info("Loaded personality: {} - {}",
                 personality.name.toStdString(),
                 personality.description.toStdString());

    return true;
}

QStringList PersonalityManager::getAvailablePersonalities() const {
    return m_personalities.keys();
}

Personality PersonalityManager::getPersonality(const QString& name) const {
    if (m_personalities.contains(name)) {
        return m_personalities[name];
    }

    // Return default/empty personality if not found
    Personality empty;
    empty.name = "Unknown";
    empty.description = "Unknown personality";
    empty.systemPrompt = "You are a helpful assistant.";
    empty.defaultEmotion = Emotion::Neutral;
    empty.warmth = 0.5f;
    empty.formality = 0.5f;
    empty.verbosity = 0.5f;
    empty.humor = 0.5f;
    return empty;
}

bool PersonalityManager::setPersonality(const QString& name) {
    if (!m_personalities.contains(name)) {
        QString error = QString("Personality not found: %1").arg(name);
        spdlog::error(error.toStdString());
        emit errorOccurred(error);
        return false;
    }

    m_currentPersonalityName = name;
    m_currentPersonality = m_personalities[name];

    spdlog::info("Switched to personality: {}", name.toStdString());
    emit personalityChanged(name);

    return true;
}

Emotion PersonalityManager::stringToEmotion(const QString& emotionStr) const {
    QString lower = emotionStr.toLower();

    if (lower == "happy") return Emotion::Happy;
    if (lower == "sad") return Emotion::Sad;
    if (lower == "surprised") return Emotion::Surprised;
    if (lower == "thoughtful") return Emotion::Thoughtful;
    if (lower == "worried") return Emotion::Worried;

    return Emotion::Neutral;
}

} // namespace Chatbot
