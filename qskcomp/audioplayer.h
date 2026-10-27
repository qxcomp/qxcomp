#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <QObject>
#include <QString>
#include <QByteArray>

class QMediaPlayer;
class QAudioOutput;
class QTemporaryFile;

// ── qskcomp 通用字节进音频播放器（GUI 线程单例）──
// 媒体类型组件拆分（qltox 分型参考口径）：audio 型的播放服务组件，
// 与 MediaViewerPopup（Image/Gif）/ VideoPlayer(Overlay)（P3）各成一体。
// 字节进 → QTemporaryFile（TtsAudio 同款机制与后缀映射）→ QMediaPlayer。
// 单实例语义：播放新 id 前途必停旧；pause 挂起 → resume 同 id 续播不重解码。
// 注：宿主工程须链接 Qt6::Multimedia。
class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    static AudioPlayer* instance();

    // bytes → temp file → play（同 id 重复调用 = 重头播）
    void play(const QString& id, const QByteArray& bytes,
              const QString& mime);
    void pause();     // 暂停保进度（playingId → pausedId 挂起）
    void resume();    // pausedId 续播
    void stop();      // 停（和自然播完同样走 finished）

    // 播放中（暂停后为 false）；挂起待续 id（resume 判据）
    QString playingId() const { return m_playingId; }
    QString pausedId() const { return m_pausedId; }
    bool isPlaying(const QString& id) const { return m_playingId == id; }

Q_SIGNALS:
    void finished(const QString& id);   // 自然播完/stop/切播——宿主清行状态

private:
    AudioPlayer();
    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_output = nullptr;
    QTemporaryFile* m_tmpFile = nullptr;
    QString m_playingId;
    QString m_pausedId;
};

#endif // AUDIO_PLAYER_H
