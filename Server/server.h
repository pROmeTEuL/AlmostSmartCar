#ifndef SERVER_H
#define SERVER_H

#include <QElapsedTimer>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "LiquidCrystal.h"
#include "sonic.h"

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
signals:

private:
    void acceptConnection();
    void readClientData();
    void socketError();
    void stopCar();
private:
    bool m_pong = true;
    QTcpServer m_server;
    QTcpSocket* m_socket = nullptr;
    char m_currentDirection;
    Sonic m_front;
    Sonic m_left;
    Sonic m_right;
    Sonic m_rear;
    LiquidCrystal m_lcd;
    QElapsedTimer m_sendSensorsTimer;
};

#endif // SERVER_H
