#include "busTcp.h"

BusTcp::BusTcp(QString netPort, QString netAdress, QObject* parent):
	QObject(parent),
	netPort(netPort),netAdress(netAdress){
	busC = NULL;
}

BusTcp::~BusTcp()
{
}

bool BusTcp::do_busConnect()
{
	busC->setConnectionParameter(QModbusDevice::NetworkPortParameter, netPort);
	busC->setConnectionParameter(QModbusDevice::NetworkAddressParameter, netAdress);
	if (!busC->connectDevice()) {
		qDebug() << "link error!!!:" << busC->errorString();
		return false;
	}
	return true;
}

bool BusTcp::read_HoldingRegisters(int salveId, int start_adress, int reg_num)
{
	QModbusDataUnit read_unit(QModbusDataUnit::HoldingRegisters, start_adress, reg_num);
	QModbusReply* reply = busC->sendReadRequest(read_unit, salveId);
	if (!reply->isFinished()) {
		QEventLoop e;
		connect(reply, &QModbusReply::finished, &e,&QEventLoop::quit);
		e.exec();
	}
	return false;
}
