#ifndef IR_INTERFACE_H
#define IR_INTERFACE_H

#include <qobject.h>
#include <qstring.h>
#include <qserialport.h>
#include <qlist.h>

#define BAUD_RATE 9600
#define SERIAL_TIMEOUT_MS 2000
#define USER_UPDATE_TIMEOUT_MS 2000
class IRInterface : public QObject
{
    Q_OBJECT
public:
    IRInterface(QObject* parent = 0);
	~IRInterface();
    void initialize(QString config_file);
	QList<int> getBadgesIds();

protected:
	TNX::QSerialPort * serialPort;
	int min;
	int max;
	QString serial_port_name;
};

#endif