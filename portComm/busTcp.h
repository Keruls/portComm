#ifndef BUSTCP_H
#define BUSTCP_H

#include<QModbusTcpClient>
#include<QModbusDataUnit>
#include<QModbusTcpServer>
#include<QDebug>
#include<QChar>
#include<QCoreApplication>
#include<QTimer>

#include "tools.h"

class BusTcp :public QObject
{
	Q_OBJECT
public:
	BusTcp(QObject* parent = nullptr);
	~BusTcp();
	bool sendRead(int index, int salve_id, int start_adress, quint16 num);
	bool sendWrite(int index, int salve_id, int start_adress, quint16 num, QString data);
	bool getConnectState();
	// Write multiple data to consecutive addresses or single data to target address of a single HoldingRegister.
	bool writeHoldingRegister(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id = 0);
	// useless function
	bool writeInputRegisters(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id = 0);
	// Write multiple bit to consecutive addresses or single bit to target address of a single coils.
	bool writeCoils(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id = 0);
	// useless function
	bool writeDiscreteInputs(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id = 0);
signals:
	void connectState(bool connect_state);
	void receiveData(QString rec_data);
public slots:
	// connect to slave ,true means start connect, false means start failed.
	bool connectSlave(QString net_adress, QString net_port);
	//active close connect, no longer automatically reConnect to slave.
	void disconnectSlave();
	void onStateChanged(QModbusDevice::State state);
	// analysis response data.
	void onReadReplyFinished();
	// analysis exist error.
	void onWriteReplyFinished();
private:
	//connect state with slave.
	bool connect_state;
	QModbusTcpClient* bus_client;
	QModbusReply* read_reply;
	QModbusReply* write_reply;
	//heartbeat.
	QTimer* timer;
};
#endif