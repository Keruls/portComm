#ifndef PORTCOMM_H
#define PORTCOMM_H

#include <QtWidgets/QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <algorithm>
#include <QMessageBox> 
#include <QThread>

#include "ui_portComm.h"
#include "busTcp.h"
#include "tools.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

class portComm : public QMainWindow
{
	Q_OBJECT
		//Q_PROPERTY()
public:
	portComm(QWidget* parent = nullptr);
	~portComm();
public slots:
	//open serial port
	void onPortOpen();
	//send string data
	bool onStrSend();
	//recive string data
	bool onStrReceive();
	//modbusrtuЭ�����
	bool onBusReceive();
	//modbusrtuЭ�鷢�ͣ�����modbusRTUЭ������֡
	bool onBusSend();
	//checked to recive hex data
	void onReceiveHex(bool a);
	//checked to recive string data
	void onReceiveStr(bool a);
public:
	int calculateCrc(const unsigned char* data, int length);
	//���鷵�����ݣ�modbusRTU����У��λ
	unsigned char checkReceiveCrc(const unsigned char* data, int length);
private:
	//default serial port setting
	void initSetting();
	void initUi();
	void initConnect();
	//useless function
	QString string2Hex(const QString& str);
private:
	Ui::portCommClass ui;
	QSerialPort* serial;
	QStringList baud;
	QStringList data;
	QStringList stop;
	QStringList parity;
	BusTcp* mybus;
};
#endif