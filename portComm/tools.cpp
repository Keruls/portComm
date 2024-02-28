#include "tools.h"

using namespace tools;
void tools::warnningBox(QString warn_str)
{
	QMessageBox mb;
	mb.setIcon(QMessageBox::Warning);
	mb.setText(warn_str);
	mb.exec();
}
bool tools::isHexFormatString(QString raw_str, int unit_max_length, QChar split_char)
{
	if (unit_max_length != 0)
	{
		QList<QString> temp = raw_str.split(split_char);
		foreach(auto i, temp)
		{
			if (i.size() > unit_max_length)
			{
				warnningBox("check: " + i + ", it is out of length.\n");
				return false;
			}
		}
	}
	for (QChar x : raw_str)
	{
		if ((x >= 'a' && x <= 'f') || 
			(x >= 'A' && x <= 'F') || 
			(x >= '0' && x <= '9') || 
			x == ' ' || 
			x == ':')
		{
			continue;
		}
		else
		{
			warnningBox("Please input chars(a-f/A-F) or numbers!");
			return false;
		}
	}
	return true;
}
bool tools::isBinaryFormatString(QString raw_str, QChar split_char)
{
	QVector<QChar> vaild_char = { '0', '1', ' ', ':'};
	foreach(QChar c , raw_str)
	{
		if (vaild_char.indexOf(c) == -1)
		{
			warnningBox("Check: " + (QString)c + ", The value must be '0' or '1'.");
			return false;
		}
	}
	return true;
}