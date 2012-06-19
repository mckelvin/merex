/*
   @Author:McKelvin
   @Date:2011-11-01
   */
#include <QtGui/QApplication>
#include "mainwindow.h"

#ifndef TEST
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("MER_EX_APP");
    MainWindow w;
    w.show();
    return a.exec();

}
#endif
