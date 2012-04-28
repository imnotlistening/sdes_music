#ifndef TCSYSTEMTRAY_H
#define TCSYSTEMTRAY_H
#include <QSystemTrayIcon>
#include <QObject>
#include <QMenu>
#include <QAction>
#include <QString>

#include "tsuserdata.h"

#define TITLE "M.U.S.I.C. Client"

class tcSystemTray : public QObject
{
	Q_OBJECT
public:
	tcSystemTray(QObject* parent = NULL);
	void initialize();
	void setState(tePlayState state);
	void writeNotification(QString msg);
	void writeToolTip(QString msg);

public slots:
	void onStarted();
	void onStopped();

signals:
	void stop();
	void start();
	void restart();
	void exit();

protected:
	QSystemTrayIcon* mpIcon;
	tePlayState mePlayState;
	QMenu* mpMenu;
	QAction* mpStop;
	QAction* mpStart;
	QAction* mpRestart;
	QAction* mpExit;
};

#endif