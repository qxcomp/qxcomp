#ifndef QPROCESS34_H
#define QPROCESS34_H

/*
 * QProcess34 —— Qt3/Qt4 兼容的 QProcess
 *
 * Qt3: QProcess 原生就有 addArgument()/start()/isRunning()/tryTerminate()/
 *      readStdout()/readStderr()/normalExit()/processIdentifier()/
 *      setWorkingDirectory(QDir)，此处仅补统一的信号 connect 包装。
 *
 * Qt4: 上述便捷 API 全部移除（start 要 program+args，信号改名为
 *      readyReadStandardOutput / finished(int,ExitStatus)），故在子类里补回
 *      同名方法；addArgument 收集的参数由 start() 拆成 program + args 传入。
 *
 * 无 Q_OBJECT，不需要 moc，纯头文件即可用。
 */

#include <qprocess.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

class QProcess34 : public QProcess {
public:
    explicit QProcess34(QObject* parent = 0) : QProcess(parent) {}

    /* 信号名适配：Qt3 readyReadStdout / readyReadStderr / processExited
     *             Qt4 readyReadStandardOutput / readyReadStandardError / finished */
    static void connectStdout(QProcess* p, const QObject* recv, const char* slot) {
#ifdef QT3_BUILD
        QObject::connect(p, SIGNAL(readyReadStdout()), recv, slot);
#else
        QObject::connect(p, SIGNAL(readyReadStandardOutput()), recv, slot);
#endif
    }
    static void connectStderr(QProcess* p, const QObject* recv, const char* slot) {
#ifdef QT3_BUILD
        QObject::connect(p, SIGNAL(readyReadStderr()), recv, slot);
#else
        QObject::connect(p, SIGNAL(readyReadStandardError()), recv, slot);
#endif
    }
    static void connectExited(QProcess* p, const QObject* recv, const char* slot) {
#ifdef QT3_BUILD
        QObject::connect(p, SIGNAL(processExited()), recv, slot);
#else
        /* Qt 规则：slot 可比 signal 签名短（忽略多余参数），无参槽两端通用 */
        QObject::connect(p, SIGNAL(finished(int,QProcess::ExitStatus)), recv, slot);
#endif
    }

#ifndef QT3_BUILD
    void addArgument(const QString& arg) { m_args.append(arg); }

    bool start() {
        if (m_args.isEmpty()) { return false; }
        QString program = m_args.takeFirst();
        QProcess::start(program, m_args);
        return true;
    }

    bool isRunning() const { return state() == QProcess::Running; }

    void tryTerminate() {
        if (state() == QProcess::Running) { terminate(); }
    }

    QByteArray readStdout() { return readAllStandardOutput(); }
    QByteArray readStderr() { return readAllStandardError(); }

    bool normalExit() const { return exitStatus() == QProcess::NormalExit; }

    qint64 processIdentifier() const { return pid(); }

    void setWorkingDirectory(const QDir& dir) {
        QProcess::setWorkingDirectory(dir.absolutePath());
    }
private:
    QStringList m_args;
#endif
};

#endif /* QPROCESS34_H */
