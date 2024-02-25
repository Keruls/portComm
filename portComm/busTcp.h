#pragma once
#include<QModbusTcpClient>
#include<QModbusDataUnit>
#include<QModbusTcpServer>
#include<QDebug>
#include<QCoreApplication>
#include<QTimer>
class BusTcp :public QObject
{
	Q_OBJECT
public:
	enum StorageArea {
		Holding_Register, 
		Input_Register, 
		Coil_Status, 
		Input_Status
	};
public:
	BusTcp(QObject* parent = nullptr);
	~BusTcp();
	bool sendRead(int index, int salve_id, int start_adress, quint16 num);
	bool sendWrite(int index, int salve_id, int start_adress, quint16 num);
	bool getConnectState();
signals:
	void connectState( bool connect_state);
	void receiveData(QString rec_data);
public slots:
	// connect to slave ,true means start connect, false means start failed.
	bool connectSlave(QString net_adress, QString net_port);
	void disconnectSlave();
	void onStateChanged(QModbusDevice::State state);
	// analysis response
	void onReplyFinish();	
private:
	//connect state with slave.
	bool connect_state;
	QModbusTcpClient* bus_client;
	QModbusReply* reply;
	//heartbeat.
	QTimer* timer;
};

