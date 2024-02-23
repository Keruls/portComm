#include "portComm.h"

portComm::portComm(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	baud = { "9600","19200","38400","57600","115200" };
	data = { "5","6","7","8" };
	stop = { "1","3","2" };
	parity = { "0" };
	serial = new QSerialPort();
	mybus = new BusTcp("6603", "127.0.0.1");
	init_combobox();
	init_setting();
	init_connect();

	ui.pte_receive->setReadOnly(true);//���տ򲻿ɱ༭
	ui.tabWidget->setCurrentIndex(0);
	ui.rb_str_recive->setChecked(1);//Ĭ�Ͻ����ַ�������
	//port_open();
}
portComm::~portComm()
{}

//Ĭ�϶˿�����
void portComm::init_setting()
{
	serial->setBaudRate(QSerialPort::Baud9600);//������_9600
	serial->setParity(QSerialPort::NoParity);//У��λ_n
	serial->setDataBits(QSerialPort::Data8);//����λ_8
	serial->setStopBits(QSerialPort::OneStop);//ֹͣλ_1
}

//��ʼ��combobox
void portComm::init_combobox()
{

	//check_port();
	ui.cb_baud->addItems(baud);
	ui.cb_databit->addItems(data);
	ui.cb_stopbit->addItems(stop);
	ui.cb_paritybit->addItems(parity);

	ui.cb_baud->setCurrentText("9600");
	ui.cb_paritybit->setCurrentText("0");
	ui.cb_databit->setCurrentText("8");
	ui.cb_stopbit->setCurrentText("1");


}

void portComm::init_connect()
{

	connect(ui.pb_check_port, &QPushButton::pressed, this, [&]() {
		ui.cb_port->clear();
		QList<QSerialPortInfo> port_info = QSerialPortInfo::availablePorts();
		for (const QSerialPortInfo& i : port_info) {
			ui.cb_port->addItem(i.portName());
		}
		});//���ϵͳ�˿���ӵ�combobox��ѡ��
	connect(ui.pb_open_port, &QPushButton::pressed, this, &portComm::port_open);
	connect(ui.pb_close_port, &QPushButton::pressed, [&]() {serial->close(); ui.label_port_state->setText("closing"); });
	connect(ui.pb_send_data, &QPushButton::pressed, this, &portComm::send);
	connect(ui.pb_clear_send, &QPushButton::pressed, ui.pte_send, &QPlainTextEdit::clear);
	connect(ui.clear_recive, &QPushButton::pressed, ui.pte_receive, &QPlainTextEdit::clear);
	//�������ջ�����
	connect(serial, &QSerialPort::readyRead, this, &portComm::receive);
	connect(ui.rb_hex_recive, &QRadioButton::clicked, this, &portComm::recive_hex);
	connect(ui.rb_str_recive, &QRadioButton::clicked, this, &portComm::recive_str);
	connect(ui.pb_send_busRTU, &QPushButton::pressed, this, &portComm::bus_send);
}



//�򿪶˿�
void portComm::port_open()
{
	//������Ҫʹ�õĶ˿ں�
	serial->setPortName(ui.cb_port->currentText());
	//���ö˿�ͨ������
	if (serial->isOpen()) {
		ui.label_port_state->setText("port opening");
		qDebug() << "current port name:" << serial->portName();
	}
	else {
		serial->setBaudRate(ui.cb_baud->currentText().toInt());
		//serial->setParity(ui.cb_paritybit->currentText().toInt());
		switch (ui.cb_databit->currentText().toInt())
		{
		case 5:serial->setDataBits(QSerialPort::Data8); qDebug() << "setdatabits5"; break;
		case 6:serial->setDataBits(QSerialPort::Data6); qDebug() << "setdatabits6"; break;
		case 7:serial->setDataBits(QSerialPort::Data7); qDebug() << "setdatabits7"; break;
		case 8:serial->setDataBits(QSerialPort::Data8); qDebug() << "setdatabits8"; break;
		default: break;
		};
		switch (ui.cb_stopbit->currentText().toInt())
		{
		case 1:serial->setStopBits(QSerialPort::OneStop); qDebug() << "setstopits1"; break;
		case 3:serial->setStopBits(QSerialPort::OneAndHalfStop); qDebug() << "setstopits3"; break;
		case 2:serial->setStopBits(QSerialPort::TwoStop); qDebug() << "setstopits2"; break;
		default: break;
		};
		//�Զ�д��ʽ�򿪶˿�
		if (serial->open(QIODevice::ReadWrite))
		{
			ui.label_port_state->setText("opening");
			qDebug() << "current port name:" << serial->portName();
		}
		else {
			ui.label_port_state->setText("closing");
			qDebug() << "port open failed!";
		}
	}
}

//��Э�鷢��
bool portComm::send()
{
	QString send_text = ui.pte_send->toPlainText();
	//����ʮ��������
	if (ui.rb_hex_send->isChecked())
	{
		QByteArray send_hex;

		for (auto c : send_text) {
			int x = c.toLatin1();
			if (x >= 'a' && x <= 'f' || x >= 'A' && x <= 'F' || x >= '0' && x <= '9' || x == ' ' || x == '-') {
				send_hex = QByteArray::fromHex(send_text.toLatin1());
			}
			else {
				//������ʾ
				QMessageBox mb;
				mb.setIcon(QMessageBox::Icon::Warning);
				mb.setText("���������ֻ�a-f/A-F��Ӣ���ַ�!");
				mb.exec();
				return 0;
			}
		}
		serial->write(send_hex);
		qDebug() << "�������ݣ�" << send_hex;
		return 1;
	}
	//�����ַ���
	else
	{
		//latin-1�ַ�����ascii�ַ�������չ
		QByteArray send_byte = send_text.toLatin1();
		serial->write(send_byte);
		qDebug() << "�������ݣ�" << send_byte;
		return 1;
	}

}

//��Э�����
bool portComm::receive()
{

	qDebug() << "receive!";
	QByteArray receive_byte = serial->readAll();
	if (ui.rb_hex_recive->isChecked()) {
		ui.pte_receive->appendPlainText(/*"receive data:" + */receive_byte.toHex());
		return 1;
	}
	else if (ui.rb_str_recive->isChecked()) {
		ui.pte_receive->appendPlainText(/*"receive data:" + */receive_byte);
		return 1;
	}
	return 0;
}

//modbusrtuЭ�鷢�ͣ�����modbusRTUЭ������֡
bool portComm::bus_send() {
	//01 03 00 01 00 01
	QByteArray byte_ary = QByteArray::fromHex(ui.pte_send_1->toPlainText().toLatin1());
	int crc = calculateCrc((unsigned char*)byte_ary.constData(), byte_ary.size());
	//��ѹ���λ��ѹ���λ
	byte_ary.append((char)(crc >> 8));
	byte_ary.append((char)(crc & 0xff));
	ui.rb_hex_recive->setChecked(true);
	serial->write(byte_ary);
	//�л�Ϊmodbus����ר�ý�����
	disconnect(serial, 0, 0, 0);
	connect(serial, &QSerialPort::readyRead, this, &portComm::bus_receive);
	ui.pte_receive->appendPlainText("send    data:" + byte_ary.toHex());
	return 1;
}

//modbusrtuЭ�����
bool portComm::bus_receive() {
	qDebug() << "bus_receive!";
	QByteArray receive_byte = serial->readAll();
	//�����ֽڵ�short int�洢crcλ�������ֽ�
	unsigned short receive_crc = receive_byte[receive_byte.size() - 2] << 8 | receive_byte[receive_byte.size() - 1];
	QByteArray receive_data;
	for (int i = 0; i < (receive_byte.size() - 2); i++) {
		receive_data.append(receive_byte[i]);
	}
	//�����ֽڵ�short int�洢crcλ�������ֽ�
	unsigned short calcu_crc = calculateCrc((unsigned char*)receive_data.constData(), receive_data.size());
	if (calcu_crc != receive_crc) {
		ui.pte_receive->appendPlainText("receive data: " + receive_byte.toHex());
		QMessageBox qmb_error;
		qmb_error.setText("calcu_crc not equal receive_crc!");
		qmb_error.setIcon(QMessageBox::Icon::Warning);
		qmb_error.exec();
		return 0;
	}
	ui.pte_receive->appendPlainText("receive data:" + receive_byte.toHex());
	//�л�Ϊ��ͨ���ݽ�����
	disconnect(serial, 0, 0, 0);
	connect(serial, &QSerialPort::readyRead, this, &portComm::receive);
	return 1;
}

//���鷵�����ݣ�modbusRTU����У��λ
unsigned char portComm::checkReceiveCrc(const unsigned char* data, int length) {
	return 1;
}

// ModBusЭ���У�CRCУ������㷽��Ϊ��
//1��Ԥ��1��16λ�ļĴ���Ϊʮ������FFFF����ȫΪ1�����ƴ˼Ĵ���ΪCRC�Ĵ�����
//2���ѵ�һ��8λ���������ݣ���ͨѶ��Ϣ֡�ĵ�һ���ֽڣ���16λ��CRC�Ĵ����ĵ�8λ����򣬰ѽ������CRC�Ĵ�����
//3����CRC�Ĵ�������������һλ������λ����0����λ����������ƺ���Ƴ�λ��
//4������Ƴ�λΪ0���ظ���3�����ٴ�����һλ���� ����Ƴ�λΪ1��CRC�Ĵ��������ʽA001(1010 0000 0000 0001)�������
//5���ظ�����3��4��ֱ������8�Σ���������8λ����ȫ�������˴���
//6���ظ�����2������5������ͨѶ��Ϣ֡��һ���ֽڵĴ���
//7������ͨѶ��Ϣ֡�����ֽڰ��������������ɺ󣬵õ���16λCRC�Ĵ����ĸߡ����ֽڽ��н�����
//modbus crc16
int portComm::calculateCrc(const unsigned char* data, int length)
{
	//step1-step6
	int crc = 0xFFFF;
	for (int i = 0; i < length; i++) {
		crc ^= data[i];
		for (int j = 0; j < 8; j++) {
			if ((crc & 0x0001) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
			}
			else {
				crc >>= 1;
			}
		}
	}
	//step7
	crc = crc << 8 | crc >> 8;
	return crc;
}

//����hex����
void portComm::recive_hex(bool a) {
	ui.rb_str_recive->setChecked(0);
	/*QString text_temp = ui.pte_receive->toPlainText();
	ui.pte_receive->setPlainText(text_temp.toUtf8().toHex());*/
}
//����str����
void portComm::recive_str(bool a) {
	ui.rb_hex_recive->setChecked(0);
	/*QString text_temp = ui.pte_receive->toPlainText();
	QByteArray byte_temp = QByteArray::fromHex(text_temp.toLatin1());
	ui.pte_receive->setPlainText(byte_temp);*/
}


//���ַ��������ַ�ת��Ϊ��Ӧ��ʮ��������
QString portComm::string2hex(const QString& str)
{
	QString target_hex = "0123456789ABCDEF";
	QString hex;
	for (QChar c : str) {
		//qDebug() << c;
		QString s;
		int n = c.toLatin1();
		while (n != 0) {
			int x = n % 16;
			s = target_hex[x] + s;
			n = n / 16;
		}
		hex.append(s + " ");
	}
	qDebug() << hex;
	return hex;
}
