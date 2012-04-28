#ifndef TCSTREAMWRAPPER_H
#define TCSTREAMWRAPPER_H

#include <QObject>
#include <stream.h>
#include <QString>

#include "tsuserdata.h"

class tcStreamWrapper : public QObject
{
    Q_OBJECT
public:
    explicit tcStreamWrapper(QString song, QObject* parent = 0);
    ~tcStreamWrapper();

    qint64 position();
    qint64 length();
    bool end_of_stream();

public slots:
   void setVolume(int vol);
   void setPlayState(int play_state);
   void setPosition(qint64 pos);

protected:
    music_rtp_pipeline* mpPLine;
};

#endif // TCSTREAMWRAPPER_H
