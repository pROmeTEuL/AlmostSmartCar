#include <QCoreApplication>
#include <pigpio.h>
#include "server.h"

int main(int argc, char *argv[])
{
    gpioCfgClock(5, 0, 0);
    gpioCfgClock(5, 1, 0);
    // initializam pigpio
    if (gpioInitialise() < 0)
            return -1;
    QCoreApplication a(argc, argv);
    Server server;
    return a.exec();
}
