#ifndef TCSTREAMTHREAD_H
#define TCSTREAMTHREAD_H

#include <QThread>
#include <QTimer>

#include "tcstreamwrapper.h"
#include "tsuserdata.h"

#define STREAM_POLL_INTERVAL 100

class tcStreamThread : public QThread
{
    Q_OBJECT
public:
    explicit tcStreamThread(QObject* parent = 0);
	~tcStreamThread();

    void run();

    bool started(){return mbStarted;}
    void checkUser(tsUserData* user_data);

signals:
    void s_setVolume(int vol);
    void s_setPlayState(int play_state);
    void s_setPosition(qint64 position);
    void songInfo(qint64 position, qint64 length);
    void songOver();

protected:
    void syncData(tsUserData *user_data);

protected slots:
    void poll();

protected:
    tcStreamWrapper* mpStream;
    QTimer* mpPollTimer;
    bool mbStarted;
    qint64 mnPosition;

    tsSongData msSongData;
    tsControlData msControls;
};

#endif // TCSTREAMTHREAD_H
