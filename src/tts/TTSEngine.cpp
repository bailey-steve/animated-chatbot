#include "tts/TTSEngine.h"
#include "tts/PhonemeExtractor.h"
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QAudioFormat>
#include <QMediaMetaData>
#include <spdlog/spdlog.h>

namespace Chatbot {

TTSEngine::TTSEngine(QObject *parent)
    : QObject(parent)
    , m_mediaPlayer(std::make_unique<QMediaPlayer>(this))
    , m_audioOutput(std::make_unique<QAudioOutput>(this))
    , m_phonemeExtractor(std::make_unique<PhonemeExtractor>())
    , m_piperPath("./third_party/piper/piper")
    , m_modelPath("./third_party/voices/en_US-lessac-medium.onnx")
    , m_espeakDataPath("./third_party/piper/espeak-ng-data")
    , m_voiceSpeed(1.0)
    , m_currentPhonemeIndex(-1)
    , m_isPlaying(false)
{
    // Set up media player
    m_mediaPlayer->setAudioOutput(m_audioOutput.get());
    m_audioOutput->setVolume(1.0);

    // Connect signals
    connect(m_mediaPlayer.get(), &QMediaPlayer::mediaStatusChanged,
            this, &TTSEngine::onMediaStatusChanged);
    connect(m_mediaPlayer.get(), &QMediaPlayer::playbackStateChanged,
            this, &TTSEngine::onPlaybackStateChanged);
    connect(m_mediaPlayer.get(), &QMediaPlayer::positionChanged,
            this, &TTSEngine::onPositionChanged);

    // Configure phoneme extractor
    m_phonemeExtractor->setPhonemizePath("./third_party/piper/piper_phonemize");
    m_phonemeExtractor->setEspeakDataPath(m_espeakDataPath);

    spdlog::info("TTSEngine initialized");
}

TTSEngine::~TTSEngine() {
    stop();
    spdlog::info("TTSEngine destroyed");
}

void TTSEngine::setPiperPath(const QString& path) {
    m_piperPath = path;
    spdlog::info("Piper path set to: {}", path.toStdString());
}

void TTSEngine::setModelPath(const QString& path) {
    m_modelPath = path;
    spdlog::info("Model path set to: {}", path.toStdString());
}

void TTSEngine::setVoiceSpeed(double speed) {
    m_voiceSpeed = speed;
    spdlog::info("Voice speed set to: {}", speed);
}

void TTSEngine::synthesize(const QString& text) {
    if (m_isPlaying) {
        spdlog::warn("Already playing audio, stopping current playback");
        stop();
    }

    if (text.trimmed().isEmpty()) {
        spdlog::warn("Empty text for synthesis");
        emit errorOccurred("Text cannot be empty");
        return;
    }

    spdlog::info("Starting synthesis for: {}", text.toStdString());
    emit synthesisStarted();

    // Generate unique filename for this utterance
    QString tempDir = QDir::tempPath();
    QString audioFile = tempDir + "/chatbot_tts_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".wav";

    // Generate audio file
    if (!generateAudio(text, audioFile)) {
        emit errorOccurred("Failed to generate audio");
        return;
    }

    // Get audio duration (read WAV header)
    QFile wavFile(audioFile);
    double audioDuration = 0.0;
    if (wavFile.open(QIODevice::ReadOnly)) {
        // WAV file size to duration estimation (assuming 22050 Hz, 16-bit, mono)
        qint64 fileSize = wavFile.size();
        qint64 dataSize = fileSize - 44;  // Subtract header size
        audioDuration = static_cast<double>(dataSize) / (22050.0 * 2.0);  // 2 bytes per sample
        wavFile.close();
        spdlog::debug("Audio duration: {} seconds", audioDuration);
    }

    // Extract phoneme timeline
    m_currentTimeline = extractPhonemeTimeline(text, audioDuration);
    m_currentPhonemeIndex = 0;

    // Set media source and play
    m_mediaPlayer->setSource(QUrl::fromLocalFile(audioFile));
    m_mediaPlayer->play();
}

void TTSEngine::stop() {
    if (m_isPlaying) {
        m_mediaPlayer->stop();
        m_isPlaying = false;
        m_currentPhonemeIndex = -1;
        spdlog::info("Playback stopped");
    }
}

bool TTSEngine::isPlaying() const {
    return m_isPlaying;
}

bool TTSEngine::generateAudio(const QString& text, const QString& outputPath) {
    spdlog::debug("Generating audio to: {}", outputPath.toStdString());

    // Create QProcess to run Piper
    QProcess process;
    process.setProgram(m_piperPath);

    QStringList args;
    args << "--model" << m_modelPath;
    args << "--output_file" << outputPath;

    if (m_voiceSpeed != 1.0) {
        args << "--length_scale" << QString::number(1.0 / m_voiceSpeed);
    }

    process.setArguments(args);

    // Start process
    process.start();
    if (!process.waitForStarted()) {
        spdlog::error("Failed to start Piper: {}", process.errorString().toStdString());
        return false;
    }

    // Write text to stdin
    process.write(text.toUtf8());
    process.closeWriteChannel();

    // Wait for completion (max 30 seconds)
    if (!process.waitForFinished(30000)) {
        spdlog::error("Piper timeout");
        process.kill();
        return false;
    }

    // Check exit code
    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        spdlog::error("Piper failed: {}", errorOutput.toStdString());
        return false;
    }

    // Verify output file exists
    if (!QFile::exists(outputPath)) {
        spdlog::error("Audio file not created: {}", outputPath.toStdString());
        return false;
    }

    spdlog::debug("Audio generated successfully");
    return true;
}

PhonemeTimeline TTSEngine::extractPhonemeTimeline(const QString& text, double audioDuration) {
    PhonemeTimeline timeline;
    timeline.text = text;
    timeline.totalDuration = audioDuration;

    // Extract raw phoneme data
    auto rawData = m_phonemeExtractor->extractPhonemes(text);
    if (!rawData.has_value()) {
        spdlog::error("Failed to extract phonemes");
        return timeline;
    }

    // Calculate timing for each phoneme (simple equal distribution)
    // TODO: More sophisticated timing using speech rate analysis
    size_t numPhonemes = rawData->phonemes.size();
    if (numPhonemes == 0) {
        return timeline;
    }

    double phonemeDuration = audioDuration / static_cast<double>(numPhonemes);
    timeline.phonemes.reserve(numPhonemes);

    for (size_t i = 0; i < numPhonemes; ++i) {
        Phoneme phoneme;
        phoneme.symbol = rawData->phonemes[i];
        phoneme.id = (i < rawData->phonemeIds.size()) ? rawData->phonemeIds[i] : 0;
        phoneme.startTime = i * phonemeDuration;
        phoneme.duration = phonemeDuration;
        timeline.phonemes.push_back(phoneme);
    }

    spdlog::info("Created phoneme timeline with {} phonemes", numPhonemes);
    return timeline;
}

void TTSEngine::updateCurrentPhoneme(double currentTime) {
    if (m_currentTimeline.phonemes.empty()) {
        return;
    }

    // Find the phoneme that should be active at currentTime
    for (size_t i = 0; i < m_currentTimeline.phonemes.size(); ++i) {
        const auto& phoneme = m_currentTimeline.phonemes[i];
        double endTime = phoneme.startTime + phoneme.duration;

        if (currentTime >= phoneme.startTime && currentTime < endTime) {
            if (static_cast<int>(i) != m_currentPhonemeIndex) {
                m_currentPhonemeIndex = static_cast<int>(i);
                emit currentPhoneme(phoneme, m_currentPhonemeIndex);
                spdlog::debug("Current phoneme: {} at {:.3f}s", phoneme.symbol.toStdString(), currentTime);
            }
            break;
        }
    }
}

void TTSEngine::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    spdlog::debug("Media status changed: {}", static_cast<int>(status));

    if (status == QMediaPlayer::LoadedMedia) {
        // Media loaded, emit playback started with timeline
        emit playbackStarted(m_currentTimeline);
    } else if (status == QMediaPlayer::EndOfMedia) {
        m_isPlaying = false;
        m_currentPhonemeIndex = -1;
        emit playbackFinished();
        spdlog::info("Playback finished");
    }
}

void TTSEngine::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    spdlog::debug("Playback state changed: {}", static_cast<int>(state));

    if (state == QMediaPlayer::PlayingState) {
        m_isPlaying = true;
    } else if (state == QMediaPlayer::StoppedState) {
        m_isPlaying = false;
    }
}

void TTSEngine::onPositionChanged(qint64 position) {
    // Update current phoneme based on playback position
    double currentTime = position / 1000.0;  // Convert ms to seconds
    updateCurrentPhoneme(currentTime);
}

} // namespace Chatbot
