#ifndef TSUSERDATA_H
#define TSUSERDATA_H

#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>

struct tsSongData
{
    QString gsPath;
    int gnHash;
    int gnTrackNum;
    int gnPlaylistId;

    tsSongData():gnHash(0){}
};

enum tePlayState
{
    eePlay = 0,
    eeStop,
    eePause
};

struct tsControlData
{
    int gnVolume;
    tePlayState gePlayState;

    tsControlData():gnVolume(0), gePlayState(eeStop){}
};

struct tsUserData
{
    int gnUserId;
    tsSongData gsSongData;
    tsControlData gsControlData;
    qint64 gnPosition;
    tsUserData():gnUserId(-1),gnPosition(0){}
};

#endif // TSUSERDATA_H
