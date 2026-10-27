#include "audioplayer.h"

#include <QAudioOutput>
#include <QDir>
#include <QMediaPlayer>
#include <QTemporaryFile>
#include <QUrl>

AudioPlayer* AudioPlayer::instance()
{
    static AudioPlayer s;
    return &s;
}

AudioPlayer::AudioPlayer()
{
    m_player = new QMediaPlayer(this);
    m_output = new QAudioOutput(this);
    m_player->setAudioOutput(m_output);
    // 自然播完/stop → StoppedState：清会话 + 通知宿主清行状态。
    // pause 只是挂起（PausedState），不走这里
    connect(m_player, &QMediaPlayer::playbackStateChanged, this,
        [this](QMediaPlayer::PlaybackState state) {
            if (state != QMediaPlayer::StoppedState) {
                return;
            }
            const QString ended = m_playingId;
            if (m_pausedId.isEmpty() && ended.isEmpty()) {
                return;   // 从未起播/已清 —— 无事
            }
            m_pausedId.clear();
            m_playingId.clear();
            Q_EMIT finished(ended);
        });
}

void AudioPlayer::play(const QString& id, const QByteArray& bytes,
    const QString& mime)
{
    m_player->stop();   // 单实例：新播必停旧（触发 finished 清旧行）
    if (m_tmpFile) {
        m_tmpFile->close();
        delete m_tmpFile;
        m_tmpFile = nullptr;
    }
    if (id.isEmpty() || bytes.isEmpty()) {
        return;
    }
    // 后缀映射（TtsAudio ttsaudio.cpp:58-63 口径 + 语音消息常见 ogg/m4a）
    QString suffix = QStringLiteral(".mp3");
    if (mime.contains(QStringLiteral("wav"))) {
        suffix = QStringLiteral(".wav");
    } else if (mime.contains(QStringLiteral("ogg"))) {
        suffix = QStringLiteral(".ogg");
    } else if (mime.contains(QStringLiteral("m4a"))) {
        suffix = QStringLiteral(".m4a");
    }
    m_tmpFile = new QTemporaryFile(
        QDir::tempPath() + QStringLiteral("/qsk_aud_XXXXXX") + suffix, this);
    m_tmpFile->setAutoRemove(true);
    if (!m_tmpFile->open()) {
        delete m_tmpFile;
        m_tmpFile = nullptr;
        return;
    }
    m_tmpFile->write(bytes);
    m_tmpFile->flush();
    m_playingId = id;
    m_pausedId.clear();
    m_player->setSource(QUrl::fromLocalFile(m_tmpFile->fileName()));
    m_player->play();
}

void AudioPlayer::pause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState
        && !m_playingId.isEmpty()) {
        m_pausedId = m_playingId;
        m_playingId.clear();
        m_player->pause();
    }
}

void AudioPlayer::resume()
{
    if (m_pausedId.isEmpty()) {
        return;
    }
    m_playingId = m_pausedId;
    m_pausedId.clear();
    m_player->play();
}

void AudioPlayer::stop()
{
    m_player->stop();   // StoppedState 派发 finished
}
