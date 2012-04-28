#ifndef TCCLIENTCTRL_H
#define TCCLIENTCTRL_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QProcess>

#include "tcstreamthread.h"
#include "ifacedb.h"
#include "IRInterface.h"
#include "tcsystemtray.h"

#define CTRL_POLL_INTERVAL 50

class tcClientCtrl : public QObject
{
    Q_OBJECT
public:
    explicit tcClientCtrl(QObject *parent = 0);
	~tcClientCtrl();

    void initialize(QString config_file);
    
protected:
	void startAdinTool();
	void stopAdinTool();
	void handleAdinTool();
	void handleBadges();

protected slots:
    void poll();
    void onSongInfo(qint64 position, qint64 length);
    void onSongOver();
	void start(bool show_msg = true);
	void stop(bool show_msg = true);
	void restart();
	void adinPrint();

signals:
	void started();
	void stopped();

protected:
    tcStreamThread* mpStream;
    tcIfaceDB* mpDB;
	IRInterface* mpIR;
	tcSystemTray* mpTray;
    QTimer* mpPollTimer;
	QProcess* mpAdinTool;

    tsUserData msUser;
	QString msConfigFile;
    QString msMediaServer;
	quint16 mnJuliusPort;
	QString msJuliusServer;
	QString msAdinToolBin;
};

#endif // TCCLIENTCTRL_H
