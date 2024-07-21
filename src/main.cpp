#include "confluenti.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication myAppli(argc, argv);
    ConfluentI myWindow;
    myWindow.setWindowTitle("ConfluentI 2.0");
    myWindow.setWindowIcon(QIcon(":/qrc_icons/icons/EkipaR_logo.jpg"));
    myWindow.showMaximized();
    return myAppli.exec();
}

