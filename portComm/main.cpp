#include "portComm.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    portComm w;
    w.show();
    return a.exec();
}
