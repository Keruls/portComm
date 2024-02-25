#pragma once

#include <QtWidgets/QMainWindow>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<algorithm>
#include <QMessageBox> 
#include "ui_portComm.h"
#include "busTcp.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

class portComm : public QMainWindow
{
    Q_OBJECT
    //Q_PROPERTY()

public:
    portComm(QWidget *parent = nullptr);
    ~portComm();
signals:
public slots:
    void onPortOpen();
    void onShowResponse(QString text);
    bool onStrSend();
    bool onStrReceive();
    bool onBusReceive();
    bool onBusSend();
    void onReceiveHex(bool a);
    void onReceiveStr(bool a);
public:
    int calculateCrc(const unsigned char* data, int length);
    unsigned char checkReceiveCrc(const unsigned char* data, int length);
private:
    Ui::portCommClass ui;
    QSerialPort* serial;
    QStringList baud;
    QStringList data;
    QStringList stop;
    QStringList parity;
    BusTcp* mybus;
private:
    void initSetting();
    void initUi();
    void initConnect();
    QString string2Hex(const QString& str);
};
