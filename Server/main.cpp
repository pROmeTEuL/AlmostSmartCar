#include <QCoreApplication>
#include <pigpio.h>
#include "server.h"

int main(int argc, char *argv[])
{
    gpioCfgClock(10, 0, 0);
    gpioCfgClock(10, 1, 0);
    if (gpioInitialise() < 0)
            return -1;
    QCoreApplication a(argc, argv);
    Server server;
    return a.exec();
}
