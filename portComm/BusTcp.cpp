#include "busTcp.h"

BusTcp::BusTcp(QObject* parent) :QObject(parent) {
	bus_client = new QModbusTcpClient();
	reply = nullptr;
	connect_state = false;
	timer = new QTimer();
	connect(timer, &QTimer::timeout, this, [&]() {if (!connect_state) bus_client->connectDevice(); });// listen, reConnect to slave.
	connect(bus_client, &QModbusDevice::stateChanged, this, &BusTcp::onStateChanged);
}
BusTcp::~BusTcp()
{
	if (connect_state) bus_client->disconnectDevice();		
	bus_client->deleteLater();
	bus_client = nullptr;
}

bool BusTcp::connectSlave(QString net_adress, QString net_port)
{
	bus_client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, net_adress);
	bus_client->setConnectionParameter(QModbusDevice::NetworkPortParameter, net_port);
	if (!bus_client->connectDevice()) {
		qDebug() << "start connection failed." << bus_client->errorString();
		return false;
	}
	return true;
}
void BusTcp::onStateChanged(QModbusDevice::State state)
{
	if (state == QModbusDevice::ConnectedState) 
	{
		qDebug() << "Device connected!";
		connect_state = true;
		emit connectState(true);
		if (!timer->isActive()) 
		{
			timer->start(3000);
		}
	}
	else
	{
		qDebug() << "Device is unconnected state:" << state;
		connect_state = false;
		emit connectState(false);
	}
}
bool BusTcp::getConnectState() { return connect_state; }
void BusTcp::disconnectSlave() { 
	if (connect_state) {
		bus_client->disconnectDevice();
		timer->stop();//active close connect, no longer reConnect to slave.
	}
}
bool BusTcp::sendRead(int index, int salve_id, int start_adress, quint16 num)
{
	if (!connect_state) 
	{
		qDebug() << "Please connect to slave first.";
		return false;
	}

	QModbusDataUnit read_unit;
	switch (index) 
	{
	case 0: read_unit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, start_adress, num); break;
	case 1: read_unit = QModbusDataUnit(QModbusDataUnit::InputRegisters, start_adress, num); break;
	case 2: read_unit = QModbusDataUnit(QModbusDataUnit::Coils, start_adress, num); break;
	case 3: read_unit = QModbusDataUnit(QModbusDataUnit::DiscreteInputs, start_adress, num); break;
	}

	reply = bus_client->sendReadRequest(read_unit, salve_id);
	connect(reply, &QModbusReply::finished, this, &BusTcp::onReplyFinish);
	if (!reply) 
	{
		qDebug() << "Send request fail.";
		return false;
	}
	return true;
}
void BusTcp::onReplyFinish()
{
	if (reply->error() == QModbusDevice::NoError) 
	{
		const QModbusDataUnit data = reply->result();
		quint16 start_address = data.startAddress();
		QString rec_data;
		for (quint16 i = 0; i < data.valueCount(); ++i) 
		{
			quint16 value = data.value(i);
			rec_data += "Register" + QString::number(i + start_address, 10) + ": " + QString::number(value, 10) + "\n";
		}
		qDebug() << rec_data;
		emit receiveData(rec_data);
	}
	else qDebug() << "Response error: " << reply->errorString();
	reply->deleteLater();//clear reply data after read handle finish.
	reply = nullptr;
}

bool BusTcp::sendWrite(int index, int salve_id, int start_adress, quint16 num)
{
	return 1;
}