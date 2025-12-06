#ifndef CHATBOT_PHONEMEEXTRACTOR_H
#define CHATBOT_PHONEMEEXTRACTOR_H

#include <QString>
#include <vector>
#include <optional>

namespace Chatbot {

// Raw phoneme data from piper_phonemize (no timing)
struct RawPhonemeData {
    std::vector<int> phonemeIds;
    std::vector<QString> phonemes;
    QString processedText;
    QString originalText;
};

class PhonemeExtractor {
public:
    PhonemeExtractor();
    ~PhonemeExtractor() = default;

    // Set path to piper_phonemize binary
    void setPhonemizePath(const QString& path);

    // Set path to espeak-ng-data directory
    void setEspeakDataPath(const QString& path);

    // Extract phonemes from text (calls piper_phonemize as subprocess)
    std::optional<RawPhonemeData> extractPhonemes(const QString& text, const QString& language = "en-us");

    // Parse JSON output from piper_phonemize
    static std::optional<RawPhonemeData> parsePhonemeJson(const QString& jsonOutput);

private:
    QString m_phonemizePath;
    QString m_espeakDataPath;
};

} // namespace Chatbot

#endif // CHATBOT_PHONEMEEXTRACTOR_H
