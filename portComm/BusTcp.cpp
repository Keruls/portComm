#include "busTcp.h"

BusTcp::BusTcp(QObject* parent) :QObject(parent) {
	bus_client = new QModbusTcpClient();
	read_reply = nullptr;
	write_reply = nullptr;
	connect_state = false;
	timer = new QTimer();
	connect(timer, &QTimer::timeout, this, [&]() {if (!connect_state) bus_client->connectDevice(); });// listen, reConnect to slave.
	connect(bus_client, &QModbusDevice::stateChanged, this, &BusTcp::onStateChanged);
}
BusTcp::~BusTcp()
{
	if (read_reply) { read_reply->deleteLater(); read_reply = nullptr; }
	if (write_reply) { write_reply->deleteLater(); write_reply = nullptr; }
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

bool BusTcp::getConnectState() 
{ 
	//if (!connect_state) qDebug() << "Please connect to the slave first.";
	return connect_state; 
}

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

	read_reply = bus_client->sendReadRequest(read_unit, salve_id);
	if (!read_reply)
	{
		qDebug() << "Send read request fail.";
		return false;
	}
	connect(read_reply, &QModbusReply::finished, this, &BusTcp::onReadReplyFinished);
	return true;
}

void BusTcp::onReadReplyFinished()
{
	if (read_reply->error() == QModbusDevice::NoError)
	{
		const QModbusDataUnit data = read_reply->result();
		int start_address = data.startAddress();
		QString rec_data;
		for (int i = 0; i < data.valueCount(); ++i)
		{
			quint16 value = data.value(i);
			rec_data += "Register" + QString::number(i + start_address, 10) + ": " + QString::number(value, 10) + "\n";
		}
		qDebug() << rec_data;
		emit receiveData(rec_data);
	}
	else 
		qDebug() << "Response error: " << read_reply->errorString();
	read_reply->deleteLater();//clear reply data after read handle finish.
	read_reply = nullptr;
}

bool BusTcp::sendWrite(int index, int salve_id, int start_address, quint16 num, QString data)
{
	if (!connect_state)
	{
		qDebug() << "Please connect to the slave first.";
		return false;
	}

	QModbusDataUnit data_unit;
	switch (index)
	{
	case 0: writeHoldingRegister(data_unit, start_address, data, salve_id); break;
	case 1: writeInputRegisters(data_unit, start_address, data, salve_id); break;
	case 2: writeCoils(data_unit, start_address, data, salve_id); break;
	case 3: writeDiscreteInputs(data_unit, start_address, data, salve_id); break;
	}

	write_reply = bus_client->sendWriteRequest(data_unit, salve_id);
	if (!write_reply)
	{
		qDebug() << "Send write request fail.";
		return false;
	}
	connect(write_reply, &QModbusReply::finished, this, &BusTcp::onWriteReplyFinished);
	return true;
}

void BusTcp::onWriteReplyFinished()
{
	if (write_reply->error() != QModbusDevice::NoError)
	{
		qDebug() << "response error: " << write_reply->errorString();
	}
	write_reply->deleteLater();
}

bool BusTcp::writeHoldingRegister(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id)
{
	if (!tools::isHexFormatString(data, 4, ':')) return false;
	QList<quint16> write_data;
	QList<QString> split_list = data.split(':');
	foreach(QString i, split_list) {
		qDebug() << i;
		write_data.append(i.toUShort(nullptr, 16));
	}
	data_unit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, start_address, write_data);
	return true;
}

bool BusTcp::writeCoils(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id)
{
	if (!tools::isBinaryFormatString(data)) return false;
	QList<quint16> write_data;
	foreach(QChar c, data)
	{
		if (c == ':') continue;
		write_data.append(c.toLatin1() - '0');//int 49('1')- int 48('0') = int 0 / int 48('0')- int 48('0') = int 0, int range(0~65535)int = (uint16_t = quint16 = unsign short int)
	}
	data_unit = QModbusDataUnit(QModbusDataUnit::Coils, start_address, write_data);
	return false;
}

bool BusTcp::writeInputRegisters(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id)
{
	tools::warnningBox("You shouldn't write InputRegister!");

	// If you insist on doing this...
	//writeHoldingRegister(data_unit, start_address, data, salve_id);
	//data_unit.setRegisterType(QModbusDataUnit::HoldingRegisters);
	
	return false;
}

bool BusTcp::writeDiscreteInputs(QModbusDataUnit& data_unit, int start_address, QString data, int salve_id)
{
	tools::warnningBox("You shouldn't write DiscreteInputs!");

	// If you insist on doing this...
	// writeCoils(data_unit, start_address, data, salve_id);
	//data_unit.setRegisterType(QModbusDataUnit::InputRegisters);
	return false;
}
