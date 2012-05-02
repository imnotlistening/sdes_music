#include "ifacedb.h"

#include <qsqlquery.h>
#include <qdebug.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <QSettings>

const QString tcIfaceDB::ksUsersQuery = "SELECT client_name, USER_ID, Pause_Stop, Volume, Song_Hash, Track_Path, Position, Track_Num, Playlist_ID from server_commands";
const QString tcIfaceDB::ksPositionQuery = "UPDATE `music_controller` SET `Position`=? WHERE `USER_ID`=?";
const QString tcIfaceDB::ksMaxQuery =  "SELECT  max(Track_Num) FROM music.playlists where Playlist_ID = ?";
const QString tcIfaceDB::ksTrackQuery = "UPDATE `music_controller` SET `Track_Num`=? WHERE `USER_ID` =?";
const QString tcIfaceDB::ksBadgeUpQuery = "UPDATE `ir_badges` SET client_name=? where IR_ID=?";
const QString tcIfaceDB::ksBadgeSelQuery = "SELECT client_name,manual_override FROM `ir_badges` where IR_ID=?";
const QString tcIfaceDB::ksBadgeInQuery = "INSERT INTO `ir_badges` VALUES(?,?,?,?)";
const QString tcIfaceDB::ksPriorityQuery = "CALL IR_PRIORITY()";
const QString tcIfaceDB::ksClientsSelQuery = "SELECT client_name FROM clients where client_name=?";
const QString tcIfaceDB::ksClientsInQuery = "INSERT INTO music.clients (`client_name`, `client_port`) VALUES (?, ?)";
const QString tcIfaceDB::ksClientsUpQuery = "UPDATE clients SET timestamp=CURRENT_TIMESTAMP() where client_name=?";
const QString tcIfaceDB::ksServerStateQuery = "SELECT server_started FROM clients where client_name=?";


tcIfaceDB::tcIfaceDB(QObject* parent)
    : QObject(parent)
    , mpDB(NULL)
    , mbReportPosition(false)
{
}

tcIfaceDB::~tcIfaceDB()
{
	if(mpDB != NULL)
	{
		delete mpDB;
	}
}

void tcIfaceDB::initialize(QString config_file)
{
    // Open our config file
    QSettings settings(config_file, QSettings::IniFormat);
    QString serverIp = settings.value("database/ip_addr").toString();
    QString userName = settings.value("database/user_name").toString();
    QString password = settings.value("database/password").toString();

    qDebug() << "Available SQL Drivers: " <<QSqlDatabase::drivers();
    // Set DB Connection
    mpDB = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL"));
    mpDB->setHostName(serverIp);
    mpDB->setDatabaseName("music");
    if(!mpDB->open(userName, password))
    {
        qDebug() << "Failed to open Database";
        return;
    }

    msRoomId = settings.value("client/name").toString();

    qDebug() << "Initialization Complete";
}

bool tcIfaceDB::getDBUser(tsUserData &user_data)
{
    if(mpDB == NULL)
    {
        qDebug() << "Database pointer is NULL";
        return false;
    }
    if(!mpDB->isOpen() || !mpDB->isValid())
    {
        qDebug() << "Database is not open or is invalid";
        return false;
    }

    bool roomFound = false;
    QList<int> usersFound;
    QSqlQuery query(*mpDB);
    query.exec(ksUsersQuery);
    while (query.next())
    {
        int user = query.value(1).toInt();
        usersFound.push_back(user);

        // check if this is this room
        QString room = query.value(0).toString();
        if(room == msRoomId)
        {

            // this user isn't in the list add it
            user_data.gsControlData.gePlayState = (tePlayState)query.value(2).toInt();
            user_data.gsControlData.gnVolume = query.value(3).toInt();
            user_data.gnUserId = user;
            user_data.gsSongData.gnHash = query.value(4).toInt();
            user_data.gsSongData.gsPath = query.value(5).toString();
            user_data.gnPosition = query.value(6).toLongLong();
            user_data.gsSongData.gnTrackNum = query.value(7).toInt();
            user_data.gsSongData.gnPlaylistId = query.value(8).toInt();

            roomFound = true;

            // We only report back to the server our position
            // if we are the first room for this user
            mbReportPosition = usersFound.count(user) == 1;
        }
    }

    if(!roomFound)
    {
        mbReportPosition = false;
    }
    return true;
}

void tcIfaceDB::requestNextSong(int user, int track_number, int playlist_id)
{
    QSqlQuery query(*mpDB);
    query.prepare(ksMaxQuery);
    query.addBindValue(playlist_id);
    query.exec();
    if(query.next())
    {
        int max = query.value(0).toInt();
        track_number++;
        if(track_number  > max)
        {
            track_number = 1;
        }
		
        query.prepare(ksTrackQuery);
        query.addBindValue(track_number);
        query.addBindValue(user);
        query.exec();
    }
}

void tcIfaceDB::updatePosition(int user_id, qint64 position)
{
    if(!mbReportPosition)
    {
        return;
    }

    QSqlQuery query(*mpDB);
    query.prepare(ksPositionQuery);
    query.addBindValue(position);
    query.addBindValue(user_id);
    query.exec();
}


void tcIfaceDB::badgesFound(QList<int> badges)
{
	bool changed = false;
	for(int i = 0; i < badges.size(); i++)
	{
		QSqlQuery query(*mpDB);
		query.prepare(ksBadgeSelQuery);
		query.addBindValue(badges[i]);
		query.exec();
		query.next();
		if(query.isValid())
		{
			bool manualOR = query.value(1).toBool();
			QString loc = query.value(0).toString();
			if(!manualOR && loc != msRoomId)
			{
				query.prepare(ksBadgeUpQuery);
				query.addBindValue(msRoomId);
				query.addBindValue(badges[i]);
				query.exec();
				changed = true;
			}
		}
		else
		{
			query.prepare(ksBadgeInQuery);
			query.addBindValue(badges[i]);
			query.addBindValue(msRoomId);
			query.addBindValue(0); //manual_override off
			query.addBindValue(0); //garbage primary key
			query.exec();
			query.next();
			changed = true;
		}
	}
	//if(changed)
	//{
		QSqlQuery query(*mpDB);
		query.prepare(ksPriorityQuery);
		query.exec();
	//}
}

void tcIfaceDB::sendHB(quint16 port)
{
	QSqlQuery query(*mpDB);
	query.prepare(ksClientsSelQuery);
	query.addBindValue(msRoomId);
	query.exec();
	query.next();
	if(query.isValid())
	{
		query.prepare(ksClientsUpQuery);
		query.addBindValue(msRoomId);
		query.exec();
	}
	else
	{
		query.prepare(ksClientsInQuery);
		query.addBindValue(msRoomId);
		query.addBindValue(port);
		query.exec();
	}
}

bool tcIfaceDB::isServerStarted()
{
	QSqlQuery query(*mpDB);
	query.prepare(ksServerStateQuery);
	query.addBindValue(msRoomId);
	query.exec();
	query.next();
	
	return query.value(0).toBool();
}