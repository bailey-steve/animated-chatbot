#include "avatar/VisemeMapper.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <spdlog/spdlog.h>

namespace Chatbot {

VisemeMapper::VisemeMapper()
    : m_loaded(false)
{
    initializeDefaultMapping();
}

bool VisemeMapper::loadMapping(const QString& configPath) {
    spdlog::info("Loading viseme mapping from: {}", configPath.toStdString());

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        spdlog::error("Failed to open viseme mapping file: {}", configPath.toStdString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        spdlog::error("Invalid JSON in viseme mapping file");
        return false;
    }

    QJsonObject root = doc.object();

    // Parse visemes
    if (root.contains("visemes") && root["visemes"].isObject()) {
        QJsonObject visemesObj = root["visemes"].toObject();

        for (auto it = visemesObj.begin(); it != visemesObj.end(); ++it) {
            QString visemeName = it.key();
            QJsonObject visemeData = it.value().toObject();

            Viseme viseme;
            viseme.name = visemeName;
            viseme.id = visemeData["id"].toInt();
            viseme.description = visemeData["description"].toString();
            viseme.mouthWidth = static_cast<float>(visemeData["mouth_width"].toDouble());
            viseme.mouthHeight = static_cast<float>(visemeData["mouth_height"].toDouble());
            viseme.jawOpen = static_cast<float>(visemeData["jaw_open"].toDouble());

            m_visemes[visemeName] = viseme;
        }
    }

    // Parse phoneme to viseme mapping
    if (root.contains("phoneme_to_viseme") && root["phoneme_to_viseme"].isObject()) {
        QJsonObject mappingObj = root["phoneme_to_viseme"].toObject();

        for (auto it = mappingObj.begin(); it != mappingObj.end(); ++it) {
            QString phoneme = it.key();
            QString visemeName = it.value().toString();
            m_phonemeToViseme[phoneme] = visemeName;
        }
    }

    m_loaded = true;
    spdlog::info("Loaded {} visemes and {} phoneme mappings",
                 m_visemes.size(), m_phonemeToViseme.size());
    return true;
}

Viseme VisemeMapper::getVisemeForPhoneme(const QString& phoneme) const {
    // Look up viseme name for this phoneme
    QString visemeName = m_phonemeToViseme.value(phoneme, "silence");

    // Return the viseme data
    return getVisemeByName(visemeName);
}

Viseme VisemeMapper::getVisemeByName(const QString& name) const {
    if (m_visemes.contains(name)) {
        return m_visemes[name];
    }

    // Return silence viseme as fallback
    return getSilenceViseme();
}

Viseme VisemeMapper::getSilenceViseme() const {
    if (m_visemes.contains("silence")) {
        return m_visemes["silence"];
    }

    // Fallback if silence not defined
    Viseme silence;
    silence.id = 0;
    silence.name = "silence";
    silence.description = "Rest position";
    silence.mouthWidth = 0.0f;
    silence.mouthHeight = 0.0f;
    silence.jawOpen = 0.0f;
    return silence;
}

void VisemeMapper::initializeDefaultMapping() {
    // Create a basic silence viseme as fallback
    Viseme silence;
    silence.id = 0;
    silence.name = "silence";
    silence.description = "Rest position";
    silence.mouthWidth = 0.0f;
    silence.mouthHeight = 0.0f;
    silence.jawOpen = 0.0f;

    m_visemes["silence"] = silence;
    m_phonemeToViseme[""] = "silence";
    m_phonemeToViseme[" "] = "silence";
}

} // namespace Chatbot
