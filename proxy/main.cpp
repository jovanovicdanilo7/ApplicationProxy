#include <QCoreApplication>
#include <iostream>
#include "proxy.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    AppProxy proxy;
    proxy.start();

    return a.exec();
}
