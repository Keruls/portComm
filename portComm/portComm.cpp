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

	ui.pte_receive->setReadOnly(true);//接收框不可编辑
	ui.tabWidget->setCurrentIndex(0);
	ui.rb_str_recive->setChecked(1);//默认接收字符串数据
	//port_open();
}
portComm::~portComm()
{}

//默认端口设置
void portComm::init_setting()
{
	serial->setBaudRate(QSerialPort::Baud9600);//波特率_9600
	serial->setParity(QSerialPort::NoParity);//校验位_n
	serial->setDataBits(QSerialPort::Data8);//数据位_8
	serial->setStopBits(QSerialPort::OneStop);//停止位_1
}

//初始化combobox
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
		});//检测系统端口添加到combobox的选项
	connect(ui.pb_open_port, &QPushButton::pressed, this, &portComm::port_open);
	connect(ui.pb_close_port, &QPushButton::pressed, [&]() {serial->close(); ui.label_port_state->setText("closing"); });
	connect(ui.pb_send_data, &QPushButton::pressed, this, &portComm::send);
	connect(ui.pb_clear_send, &QPushButton::pressed, ui.pte_send, &QPlainTextEdit::clear);
	connect(ui.clear_recive, &QPushButton::pressed, ui.pte_receive, &QPlainTextEdit::clear);
	//监听接收缓冲区
	connect(serial, &QSerialPort::readyRead, this, &portComm::receive);
	connect(ui.rb_hex_recive, &QRadioButton::clicked, this, &portComm::recive_hex);
	connect(ui.rb_str_recive, &QRadioButton::clicked, this, &portComm::recive_str);
	connect(ui.pb_send_busRTU, &QPushButton::pressed, this, &portComm::bus_send);
}



//打开端口
void portComm::port_open()
{
	//设置需要使用的端口号
	serial->setPortName(ui.cb_port->currentText());
	//设置端口通信属性
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
		//以读写方式打开端口
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

//无协议发送
bool portComm::send()
{
	QString send_text = ui.pte_send->toPlainText();
	//发送十六进制数
	if (ui.rb_hex_send->isChecked())
	{
		QByteArray send_hex;

		for (auto c : send_text) {
			int x = c.toLatin1();
			if (x >= 'a' && x <= 'f' || x >= 'A' && x <= 'F' || x >= '0' && x <= '9' || x == ' ' || x == '-') {
				send_hex = QByteArray::fromHex(send_text.toLatin1());
			}
			else {
				//弹窗提示
				QMessageBox mb;
				mb.setIcon(QMessageBox::Icon::Warning);
				mb.setText("请输入数字或（a-f/A-F）英文字符!");
				mb.exec();
				return 0;
			}
		}
		serial->write(send_hex);
		qDebug() << "发送数据：" << send_hex;
		return 1;
	}
	//发送字符串
	else
	{
		//latin-1字符集是ascii字符集的扩展
		QByteArray send_byte = send_text.toLatin1();
		serial->write(send_byte);
		qDebug() << "发送数据：" << send_byte;
		return 1;
	}

}

//无协议接收
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

//modbusrtu协议发送，构建modbusRTU协议数据帧
bool portComm::bus_send() {
	//01 03 00 01 00 01
	QByteArray byte_ary = QByteArray::fromHex(ui.pte_send_1->toPlainText().toLatin1());
	int crc = calculateCrc((unsigned char*)byte_ary.constData(), byte_ary.size());
	//先压入高位再压入低位
	byte_ary.append((char)(crc >> 8));
	byte_ary.append((char)(crc & 0xff));
	ui.rb_hex_recive->setChecked(true);
	serial->write(byte_ary);
	//切换为modbus数据专用解析槽
	disconnect(serial, 0, 0, 0);
	connect(serial, &QSerialPort::readyRead, this, &portComm::bus_receive);
	ui.pte_receive->appendPlainText("send    data:" + byte_ary.toHex());
	return 1;
}

//modbusrtu协议接收
bool portComm::bus_receive() {
	qDebug() << "bus_receive!";
	QByteArray receive_byte = serial->readAll();
	//用两字节的short int存储crc位的两个字节
	unsigned short receive_crc = receive_byte[receive_byte.size() - 2] << 8 | receive_byte[receive_byte.size() - 1];
	QByteArray receive_data;
	for (int i = 0; i < (receive_byte.size() - 2); i++) {
		receive_data.append(receive_byte[i]);
	}
	//用两字节的short int存储crc位的两个字节
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
	//切换为普通数据解析槽
	disconnect(serial, 0, 0, 0);
	connect(serial, &QSerialPort::readyRead, this, &portComm::receive);
	return 1;
}

//检验返回数据（modbusRTU）的校验位
unsigned char portComm::checkReceiveCrc(const unsigned char* data, int length) {
	return 1;
}

// ModBus协议中，CRC校验码计算方法为：
//1．预置1个16位的寄存器为十六进制FFFF（即全为1）；称此寄存器为CRC寄存器；
//2．把第一个8位二进制数据（既通讯信息帧的第一个字节）与16位的CRC寄存器的低8位相异或，把结果放于CRC寄存器；
//3．把CRC寄存器的内容右移一位（朝低位）用0填补最高位，并检查右移后的移出位；
//4．如果移出位为0：重复第3步（再次右移一位）； 如果移出位为1：CRC寄存器与多项式A001(1010 0000 0000 0001)进行异或；
//5．重复步骤3和4，直到右移8次，这样整个8位数据全部进行了处理；
//6．重复步骤2到步骤5，进行通讯信息帧下一个字节的处理；
//7．将该通讯信息帧所有字节按上述步骤计算完成后，得到的16位CRC寄存器的高、低字节进行交换；
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

//接收hex数据
void portComm::recive_hex(bool a) {
	ui.rb_str_recive->setChecked(0);
	/*QString text_temp = ui.pte_receive->toPlainText();
	ui.pte_receive->setPlainText(text_temp.toUtf8().toHex());*/
}
//接收str数据
void portComm::recive_str(bool a) {
	ui.rb_hex_recive->setChecked(0);
	/*QString text_temp = ui.pte_receive->toPlainText();
	QByteArray byte_temp = QByteArray::fromHex(text_temp.toLatin1());
	ui.pte_receive->setPlainText(byte_temp);*/
}


//将字符串各个字符转换为对应的十六进制数
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
