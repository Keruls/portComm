#ifndef TOOLS_H
#define TOOLS_H
#include <QString>
#include <QMessageBox>
namespace tools
{
	void warnningBox(QString warn_str);
	//Check if the raw string is in correct format like "xxxx:xx:xxx:x"
	bool isHexFormatString(QString raw_str, int unit_max_length = 0, QChar split_char = ':');
	//Check if the raw string is in correct format like "xxxx:xx:xxx:x" and the chars only '1'¡¢'0'¡¢' '¡¢':'
	bool isBinaryFormatString(QString raw_str, QChar split_char = ':');
}
#endif // !TOOLS_H