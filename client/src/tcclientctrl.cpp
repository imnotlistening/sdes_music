#include "tcclientctrl.h"
#include "tsuserdata.h"

#include <gst/gst.h>
#include <glib.h>

#include <QSettings>
#include <QtDebug>
#include <QtGui\QApplication>

tcClientCtrl::tcClientCtrl(QObject *parent) :
    QObject(parent)
  , mpStream(NULL)
  , mpDB(NULL)
  , mpPollTimer(NULL)
  , mnJuliusPort(0)
  , mpAdinTool(NULL)
  , mpTray(NULL)
{
}

tcClientCtrl::~tcClientCtrl()
{
	if(mpDB != NULL)
	{
		delete mpDB;
	}

	if(mpIR != NULL)
	{
		delete mpIR;
	}

	if(mpPollTimer != NULL)
	{
		mpPollTimer->stop();
		delete mpPollTimer;
	}

	if(mpStream != NULL)
	{
		delete mpStream;
	}

	if(mpAdinTool != NULL)
	{
		mpAdinTool->terminate();
		delete mpAdinTool;
	}
}

void tcClientCtrl::initialize(QString config_file)
{
    gst_init(NULL, NULL);
	msConfigFile = config_file;

	mpTray = new tcSystemTray(this);
	mpTray->initialize();
	connect(this, SIGNAL(started()), mpTray, SLOT(onStarted()));
	connect(this, SIGNAL(stopped()), mpTray, SLOT(onStopped()));
	mpTray->writeToolTip("Addintool: Stopped");

	start();
    qDebug() << "tcClientCtrl Initialized";
}

void tcClientCtrl::poll()
{
    tsUserData data;

    if(mpDB->getDBUser(data))
    {
        // Append the server address to the song path
        if(!data.gsSongData.gsPath.isEmpty())
        {
			data.gsSongData.gsPath = (QString("http://%1%2").arg(msMediaServer).arg(data.gsSongData.gsPath)).replace(" ", "%20");
        }
        // if the stream is null start a new one
        if(mpStream == NULL)
        {
            qDebug() << "Creating new Stream";
            mpStream = new tcStreamThread(this);
            connect(mpStream, SIGNAL(songInfo(qint64,qint64)), this, SLOT(onSongInfo(qint64,qint64)));
            connect(mpStream, SIGNAL(songOver()), this, SLOT(onSongOver()));
            mpStream->checkUser(&data);
        }
        else if(data.gnUserId != msUser.gnUserId)
        {
            qDebug() << "Restarting stream";
            // This is a different user to restart the stream
            if(mpStream->isRunning())
            {
                mpStream->quit();
				while(mpStream->isRunning());
            }
            disconnect(mpStream, SIGNAL(songInfo(qint64,qint64)), this, SLOT(onSongInfo(qint64,qint64)));
            disconnect(mpStream, SIGNAL(songOver()), this, SLOT(onSongOver()));
            mpStream->deleteLater();

            mpStream = new tcStreamThread(this);
            connect(mpStream, SIGNAL(songInfo(qint64,qint64)), this, SLOT(onSongInfo(qint64,qint64)));
            connect(mpStream, SIGNAL(songOver()), this, SLOT(onSongOver()));
            mpStream->checkUser(&data);
        }
        else
        {
            mpStream->checkUser(&data);
        }
    }
    else if(mpStream != NULL)
    {
        qDebug() << "Deleting stream";
        // The user is no longer here so kill the stream
        if(mpStream->isRunning())
        {
            mpStream->quit();
        }
        disconnect(mpStream, SIGNAL(songInfo(qint64,qint64)), this, SLOT(onSongInfo(qint64,qint64)));
        mpStream->deleteLater();
        mpStream = NULL;
    }

    msUser = data;

	// Set the state of our icon
	mpTray->setState(msUser.gsControlData.gePlayState);

	// Handle starting/stopping the adintool
	// and send our heart beat to the DB
	handleAdinTool();

	// Handle any badges in the room
	handleBadges();
}

void tcClientCtrl::adinPrint()
{
	QString err = mpAdinTool->readAllStandardError();
	QString out = mpAdinTool->readAllStandardOutput();
	qDebug() << err;
	qDebug() << out;
}
void tcClientCtrl::onSongInfo(qint64 position, qint64 length)
{
   mpDB->updatePosition(msUser.gnUserId, position);
}

void tcClientCtrl::onSongOver()
{
    mpDB->requestNextSong(msUser.gnUserId, msUser.gsSongData.gnTrackNum, msUser.gsSongData.gnPlaylistId);
}

void tcClientCtrl::startAdinTool()
{
	// This is a bit heavy handed but
	// forcefully kill any adintool
	// that is already running
#ifdef WIN32
	QProcess kill;
	kill.start("taskkill /F /im adintool.exe");
#else
	QProcess kill;
	kill.start("pkill adintool");
#endif

	mpAdinTool = new QProcess(this);
	connect(mpAdinTool, SIGNAL(readyReadStandardError()), SLOT(adinPrint()));
	connect(mpAdinTool, SIGNAL(readyReadStandardOutput()), SLOT(adinPrint()));
	
	QString exec = QString("%1 -in mic -out adinnet -port %2 -server %3 -tailmargin 1200 -zmean").arg(msAdinToolBin).arg(mnJuliusPort).arg(msJuliusServer);
	mpAdinTool->start(exec);
	mpTray->writeToolTip("Addintool: Started");
}

void tcClientCtrl::stopAdinTool()
{
	mpAdinTool->terminate();
	mpAdinTool->deleteLater();
	mpAdinTool = NULL;
	mpTray->writeToolTip("Addintool: Stopped");
}

void tcClientCtrl::handleAdinTool()
{
	// Make sure are added to the client table
	// and update our last heard from
	mpDB->sendHB(mnJuliusPort);

	// See if the julius server is started
	// for this client. If it is then 
	// start the adintool client
	if(mpDB->isServerStarted())
	{
		if(mpAdinTool == NULL)
		{
			startAdinTool();
		}
	}
	else
	{
		if(mpAdinTool != NULL)
		{
			stopAdinTool();
		}
	}
}

void tcClientCtrl::handleBadges()
{
	// See if we have any badges in the room
	// tell the server if we do
	QList<int> badges = mpIR->getBadgesIds();
	if(!badges.empty())
	{
		mpDB->badgesFound(badges);
	}
}

void tcClientCtrl::start(bool show_msg)
{
    QSettings settings(msConfigFile, QSettings::IniFormat);
    msMediaServer = settings.value("client/media_ipaddr").toString();
	mnJuliusPort = settings.value("julius/port").toUInt();
	msJuliusServer = settings.value("julius/ip_addr").toString();
	msAdinToolBin = settings.value("julius/adin_bin").toString();

	connect(mpTray, SIGNAL(exit()), (QApplication*)parent(), SLOT(quit()));
	connect(mpTray, SIGNAL(start()), SLOT(start()));
	connect(mpTray, SIGNAL(stop()), SLOT(stop()));
	connect(mpTray, SIGNAL(restart()), SLOT(restart()));

    mpDB = new tcIfaceDB(this);
    mpDB->initialize(msConfigFile);

	mpIR = new IRInterface(this);
	mpIR->initialize(msConfigFile);

    mpPollTimer = new QTimer(this);
    mpPollTimer->setSingleShot(false);
    mpPollTimer->setInterval(CTRL_POLL_INTERVAL);
    connect(mpPollTimer, SIGNAL(timeout()), this, SLOT(poll()));
    mpPollTimer->start();

	emit started();

	if(show_msg)
		mpTray->writeNotification("Client Started");	
}

void tcClientCtrl::stop(bool show_msg)
{
	if(mpDB != NULL)
	{
		mpDB->deleteLater();
		mpDB = NULL;
	}

	if(mpIR != NULL)
	{
		mpIR->deleteLater();
		mpIR = NULL;
	}

	if(mpPollTimer != NULL)
	{
		mpPollTimer->stop();
		mpPollTimer->deleteLater();
		mpPollTimer = NULL;
	}

	if(mpStream != NULL)
	{
		mpStream->deleteLater();
		mpStream = NULL;
	}

	if(mpAdinTool != NULL)
	{
		mpAdinTool->terminate();
		mpAdinTool->deleteLater();
		mpTray->writeToolTip("Addintool: Stopped");
	}

	emit stopped();

	if(show_msg)
		mpTray->writeNotification("Client Stopped");	
}
void tcClientCtrl::restart()
{
	stop(false);
	start(false);

	mpTray->writeNotification("Client Restarted");	
}