#ifndef IFACEDB_H
#define IFACEDB_H

#include <qobject.h>
#include <qsqldatabase.h>
#include <qmap.h>
#include <qstring.h>
#include <QList>
#include "tsuserdata.h"

#define DB_POLL_INTERVAL 500

class tcIfaceDB : public QObject
{
	Q_OBJECT
public:
    tcIfaceDB(QObject *parent);
	~tcIfaceDB();

    void initialize(QString config_file);

    bool getDBUser(tsUserData& user_data);
    void requestNextSong(int user, int track_number, int playlist_id);
	void badgesFound(QList<int> badges);
	void sendHB(quint16 port);
	bool isServerStarted();

public:
    void updatePosition(int user_id, qint64 position);

protected:
    QSqlDatabase* mpDB;
    QString msRoomId;
    bool mbReportPosition;

    static const QString ksUsersQuery;
    static const QString ksPositionQuery;
    static const QString ksMaxQuery;
    static const QString ksTrackQuery;
    static const QString ksBadgeUpQuery;
    static const QString ksBadgeSelQuery;
    static const QString ksBadgeInQuery;
    static const QString ksPriorityQuery;
    static const QString ksClientsSelQuery;
    static const QString ksClientsInQuery;
    static const QString ksClientsUpQuery;
	static const QString ksServerStateQuery;
};

#endif
