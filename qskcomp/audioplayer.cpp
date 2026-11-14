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

    connect(m_player, &QMediaPlayer::positionChanged, this,
        [this](qint64 ms) { Q_EMIT positionChanged(ms); });
    connect(m_player, &QMediaPlayer::durationChanged, this,
        [this](qint64 ms) { Q_EMIT durationChanged(ms); });

    // 播放失败（源 404/拒连/格式不支持）：通知宿主。错误时 QMediaPlayer
    // 自身会转 Stopped，playbackStateChanged 的 finished 派发照常走，
    // 此处只补错误事件；底层 msg 进日志，宿主展示友好文案。
    connect(m_player, &QMediaPlayer::errorOccurred, this,
        [this](QMediaPlayer::Error err, const QString& msg) {
            qWarning().noquote() << "[AudioPlayer] playError" << int(err)
                                 << m_playingId << msg;
            if (m_playingId.isEmpty()) return;
            Q_EMIT playError(m_playingId);
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

void AudioPlayer::playUrl(const QString& id, const QUrl& url)
{
    m_player->stop();   // 单实例：新播必停旧（触发 finished 清旧行）
    if (id.isEmpty() || url.isEmpty()) {
        return;
    }
    m_playingId = id;
    m_pausedId.clear();
    m_player->setSource(url);
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
