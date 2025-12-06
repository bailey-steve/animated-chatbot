#ifndef CHATBOT_VISEMEMAPPER_H
#define CHATBOT_VISEMEMAPPER_H

#include <QString>
#include <QMap>
#include <memory>

namespace Chatbot {

// Viseme data structure representing mouth shape
struct Viseme {
    int id;
    QString name;
    QString description;
    float mouthWidth;    // 0.0 to 1.0
    float mouthHeight;   // 0.0 to 1.0
    float jawOpen;       // 0.0 to 1.0
};

// Manages phoneme-to-viseme mapping
class VisemeMapper {
public:
    VisemeMapper();
    ~VisemeMapper() = default;

    // Load viseme mapping from JSON configuration
    bool loadMapping(const QString& configPath);

    // Convert phoneme symbol to viseme
    Viseme getVisemeForPhoneme(const QString& phoneme) const;

    // Get viseme by name
    Viseme getVisemeByName(const QString& name) const;

    // Check if mapping is loaded
    bool isLoaded() const { return m_loaded; }

    // Get silence/rest viseme
    Viseme getSilenceViseme() const;

private:
    void initializeDefaultMapping();

private:
    bool m_loaded;
    QMap<QString, Viseme> m_visemes;           // Viseme name -> Viseme data
    QMap<QString, QString> m_phonemeToViseme;  // Phoneme symbol -> Viseme name
};

} // namespace Chatbot

#endif // CHATBOT_VISEMEMAPPER_H
