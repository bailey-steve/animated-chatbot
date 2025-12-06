#ifndef CHATBOT_TTSENGINE_H
#define CHATBOT_TTSENGINE_H

#include <QObject>
#include <QString>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <memory>
#include <vector>

namespace Chatbot {

// Forward declarations
class PhonemeExtractor;

// Phoneme data structure with timing
struct Phoneme {
    QString symbol;      // Phoneme symbol (e.g., "h", "ə", "l")
    int id;              // Phoneme ID from espeak
    double startTime;    // Start time in seconds
    double duration;     // Duration in seconds
};

// Timeline of phonemes for a spoken utterance
struct PhonemeTimeline {
    std::vector<Phoneme> phonemes;
    double totalDuration;  // Total audio duration in seconds
    QString text;          // Original text
};

class TTSEngine : public QObject {
    Q_OBJECT

public:
    explicit TTSEngine(QObject *parent = nullptr);
    ~TTSEngine() override;

    // Delete copy constructor and assignment operator
    TTSEngine(const TTSEngine&) = delete;
    TTSEngine& operator=(const TTSEngine&) = delete;

    // Configuration
    void setPiperPath(const QString& path);
    void setModelPath(const QString& path);
    void setVoiceSpeed(double speed);  // 1.0 = normal, 0.5 = slow, 2.0 = fast

    // Synthesis control
    void synthesize(const QString& text);
    void stop();
    bool isPlaying() const;

signals:
    // Emitted when synthesis starts
    void synthesisStarted();

    // Emitted when audio playback starts, includes phoneme timeline
    void playbackStarted(const PhonemeTimeline& timeline);

    // Emitted periodically during playback with current phoneme
    void currentPhoneme(const Phoneme& phoneme, int index);

    // Emitted when playback completes
    void playbackFinished();

    // Emitted on errors
    void errorOccurred(const QString& error);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onPositionChanged(qint64 position);

private:
    // Generate audio file using Piper
    bool generateAudio(const QString& text, const QString& outputPath);

    // Extract phoneme timeline
    PhonemeTimeline extractPhonemeTimeline(const QString& text, double audioDuration);

    // Update current phoneme based on playback position
    void updateCurrentPhoneme(double currentTime);

private:
    std::unique_ptr<QMediaPlayer> m_mediaPlayer;
    std::unique_ptr<QAudioOutput> m_audioOutput;
    std::unique_ptr<PhonemeExtractor> m_phonemeExtractor;

    QString m_piperPath;
    QString m_modelPath;
    QString m_espeakDataPath;
    double m_voiceSpeed;

    PhonemeTimeline m_currentTimeline;
    int m_currentPhonemeIndex;
    bool m_isPlaying;
};

} // namespace Chatbot

#endif // CHATBOT_TTSENGINE_H
