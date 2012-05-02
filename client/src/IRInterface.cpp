#include "IRInterface.h"
#include <qdebug.h>
#include <qsettings.h>
#include <qset.h>

IRInterface::IRInterface(QObject* parent)
	: QObject(parent)
	, serialPort(NULL)
{
}

IRInterface::~IRInterface()
{
	if(serialPort != NULL)
	{
		delete serialPort;
	}
}

void IRInterface::initialize(QString config_file)
{
	QSettings settings(config_file, QSettings::IniFormat);
	serial_port_name = settings.value("ir/serial_port").toString();
	min = settings.value("ir/min_badge_id").toInt();
	max = settings.value("ir/max_badge_id").toInt();

	QString serialSettings = QString("%1,8,n,1").arg(BAUD_RATE); 

	serialPort = new TNX::QSerialPort(serial_port_name, serialSettings);

	if (!serialPort->open())
	{
		qDebug() << QString("Error: Failed to open Serial Port: %1").arg(serial_port_name);
		serialPort->deleteLater();
		serialPort = NULL;
	}
	else
	{
		serialPort->flushInBuffer();

		qDebug() << "IR Interface Initialization Complete";
	}
}

QList<int> IRInterface::getBadgesIds()
{
	if(serialPort == NULL)
	{
		return QList<int>();
	}
	int i;
	qint64 num_bytes = serialPort->bytesAvailable();
	QByteArray data = serialPort->read(num_bytes);
	QSet<int> ids;
	for( i = 0; i < data.size(); i++) {
		int val = 0;
		char byte = data.at(i);
		memcpy(&val, &byte, 1);
		if(val >= min && val <= max)
		{
			ids.insert(val);
		}
	}

	return ids.toList();
}