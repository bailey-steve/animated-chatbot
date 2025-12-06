#include "tts/PhonemeExtractor.h"
#include <QProcess>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace Chatbot {

PhonemeExtractor::PhonemeExtractor()
    : m_phonemizePath("./third_party/piper/piper_phonemize")
    , m_espeakDataPath("./third_party/piper/espeak-ng-data")
{
}

void PhonemeExtractor::setPhonemizePath(const QString& path) {
    m_phonemizePath = path;
}

void PhonemeExtractor::setEspeakDataPath(const QString& path) {
    m_espeakDataPath = path;
}

std::optional<RawPhonemeData> PhonemeExtractor::extractPhonemes(const QString& text, const QString& language) {
    spdlog::debug("Extracting phonemes for text: {}", text.toStdString());

    // Create QProcess to run piper_phonemize
    QProcess process;
    process.setProgram(m_phonemizePath);
    process.setArguments({
        "-l", language,
        "--espeak_data", m_espeakDataPath
    });

    // Start process and write text to stdin
    process.start();
    if (!process.waitForStarted()) {
        spdlog::error("Failed to start piper_phonemize: {}", process.errorString().toStdString());
        return std::nullopt;
    }

    // Write text to stdin and close it
    process.write(text.toUtf8());
    process.closeWriteChannel();

    // Wait for process to finish
    if (!process.waitForFinished(5000)) {  // 5 second timeout
        spdlog::error("piper_phonemize timeout");
        process.kill();
        return std::nullopt;
    }

    // Check exit code
    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        spdlog::error("piper_phonemize failed: {}", errorOutput.toStdString());
        return std::nullopt;
    }

    // Read JSON output
    QString jsonOutput = process.readAllStandardOutput();
    return parsePhonemeJson(jsonOutput);
}

std::optional<RawPhonemeData> PhonemeExtractor::parsePhonemeJson(const QString& jsonOutput) {
    try {
        json j = json::parse(jsonOutput.toStdString());

        RawPhonemeData data;

        // Parse phoneme IDs
        if (j.contains("phoneme_ids") && j["phoneme_ids"].is_array()) {
            for (const auto& id : j["phoneme_ids"]) {
                data.phonemeIds.push_back(id.get<int>());
            }
        }

        // Parse phoneme symbols
        if (j.contains("phonemes") && j["phonemes"].is_array()) {
            for (const auto& phoneme : j["phonemes"]) {
                data.phonemes.push_back(QString::fromStdString(phoneme.get<std::string>()));
            }
        }

        // Parse text fields
        if (j.contains("processed_text")) {
            data.processedText = QString::fromStdString(j["processed_text"].get<std::string>());
        }

        if (j.contains("text")) {
            data.originalText = QString::fromStdString(j["text"].get<std::string>());
        }

        spdlog::debug("Extracted {} phonemes", data.phonemes.size());
        return data;

    } catch (const json::exception& e) {
        spdlog::error("Failed to parse phoneme JSON: {}", e.what());
        return std::nullopt;
    }
}

} // namespace Chatbot
