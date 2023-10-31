#pragma once

#include <QtWidgets/QMainWindow>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<algorithm>
#include <QMessageBox>

#include "ui_portComm.h"
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
    
private:
    Ui::portCommClass ui;
    QSerialPort *serial;
    QStringList baud;
    QStringList data;
    QStringList stop;
    QStringList parity;
    void init_setting();
    void init_combobox();
    void init_connect();
    QString string2hex(const QString& str);
public slots:
    void slot_reCheckPort_pressed();
    bool send();
    bool receive();
    bool bus_receive();
    void port_open();
    bool bus_send();
    void cutShowMode(bool a);
public:
    int calculateCrc(const unsigned char* data, int length);
    unsigned char checkReceiveCrc(const unsigned char* data, int length);
    
};
