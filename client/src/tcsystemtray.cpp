#include "tcsystemtray.h"

#include <QIcon>
#include <QStyle>
#include <QApplication>

tcSystemTray::tcSystemTray(QObject* parent)
	: QObject(parent)
	, mpIcon(NULL)
	, mePlayState(eeStop)
{
}

void tcSystemTray::initialize()
{
	mpIcon = new QSystemTrayIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop), this);
	mpMenu = new QMenu();
	mpStart = mpMenu->addAction("Start Client");
	mpStop = mpMenu->addAction("Stop Client");
	mpRestart = mpMenu->addAction("Restart Client");
	mpMenu->addSeparator();
	mpExit = mpMenu->addAction("Exit");
	mpIcon->setContextMenu(mpMenu);

	connect(mpStart, SIGNAL(triggered()), SIGNAL(start()));
	connect(mpStop, SIGNAL(triggered()), SIGNAL(stop()));
	connect(mpRestart, SIGNAL(triggered()), SIGNAL(restart()));
	connect(mpExit, SIGNAL(triggered()), SIGNAL(exit()));

	mpIcon->show();
}

void tcSystemTray::setState(tePlayState state)
{
	if(state != mePlayState)
	{
		switch(state)
		{
		case eePause:
			mpIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));
			break;
		case eePlay:
			mpIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
			break;
		case eeStop:
			mpIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaStop));
			break;
		}
	}
}

void tcSystemTray::onStarted()
{
	mpStart->setEnabled(false);
	mpStop->setEnabled(true);
}

void tcSystemTray::onStopped()
{
	mpStart->setEnabled(true);
	mpStop->setEnabled(false);
}

void tcSystemTray::writeNotification(QString msg)
{
	mpIcon->showMessage(TITLE, msg);	
}

void tcSystemTray::writeToolTip(QString msg)
{
	mpIcon->setToolTip(msg);
}