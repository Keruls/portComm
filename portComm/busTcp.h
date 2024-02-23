#pragma once
#include<QModbusTcpClient>
#include<QModbusDataUnit>
#include<QModbusTcpServer>
#include<QDebug>
#include<QCoreApplication>
class BusTcp :public QObject
{
	Q_OBJECT
public:
	BusTcp( QString netPort, QString netAdress, QObject* parent = nullptr);
	~BusTcp();
private:
	QModbusTcpClient* busC;
	QString netPort;
	QString netAdress;
public:
	bool do_busConnect();
	bool read_HoldingRegisters(int salveId, int start_adress, int reg_num);
};

