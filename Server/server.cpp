#include "server.h"

#include <pigpio.h>

#include <QTimer>
#include <QCoreApplication>

static const char Forward = 'f';
static const char Left = 'l';
static const char Right = 'r';
static const char Backward = 'b';
static const char Stop = 's';
static const char PingPong = 'p';
static const char Distance = 'd';
static const char Shutdown = 'z';

static const int rs = 22;
static const int en = 23;
static const int d0 = 24;
static const int d1 = 10;
static const int d2 = 9;
static const int d3 = 25;
static const int d4 = 11;
static const int d5 = 8;
static const int d6 = 5;
static const int d7 = 6;

enum {
    LeftEngEn = 20,
    LeftEngFr = 21,
    LeftEngBk = 26,
    RightEngEn = 13,
    RightEngFr = 19,
    RightEngBk = 16,
    FrontTrig = 2,
    FrontEcho = 3,
    LeftTrig = 14,
    LeftEcho = 15,
    RightTrig = 4,
    RightEcho = 17,
    RearTrig = 18,
    RearEcho = 27,
};

static const uint8_t UpChBuff[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100,
};

static const uint8_t LeftChBuff[8] = {
  0b00000,
  0b00100,
  0b01100,
  0b11111,
  0b01100,
  0b00100,
  0b00000,
};

static const uint8_t RightChBuff[8] = {
  0b00000,
  0b00100,
  0b00110,
  0b11111,
  0b00110,
  0b00100,
  0b00000,
};

static const uint8_t DownChBuff[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
};

static const uint8_t UpCh = 0;
static const uint8_t LeftCh = 1;
static const uint8_t RightCh = 2;
static const uint8_t DownCh = 3;

Server::Server(QObject *parent)
    : QObject(parent)
    , m_front(FrontTrig, FrontEcho, 'f')
    , m_left(LeftTrig, LeftEcho, 'l')
    , m_right(RightTrig, RightEcho, 'r')
    , m_rear(RearTrig, RearEcho, 'b')
    , m_lcd(rs, en, d0, d1, d2, d3, d4, d5, d6, d7)
    , m_currentDirection(Stop)
{
    connect(&m_server, &QTcpServer::newConnection, this, &Server::acceptConnection);
    gpioSetMode(LeftEngEn, PI_OUTPUT);
    gpioSetMode(LeftEngFr, PI_OUTPUT);
    gpioSetMode(LeftEngBk, PI_OUTPUT);
    gpioSetMode(RightEngEn, PI_OUTPUT);
    gpioSetMode(RightEngFr, PI_OUTPUT);
    gpioSetMode(RightEngBk, PI_OUTPUT);
    m_server.listen(QHostAddress::Any, 5706);
    m_lcd.begin(16, 2);
    m_lcd.clear();
    m_lcd.createChar(UpCh, UpChBuff);
    m_lcd.createChar(LeftCh, LeftChBuff);
    m_lcd.createChar(RightCh, RightChBuff);
    m_lcd.createChar(DownCh, DownChBuff);
    m_lcd.setCursor(0, 0);
    m_lcd.write("Disconnected");
    auto pingPongTimer = new QTimer(this);
    connect(pingPongTimer, &QTimer::timeout, this, [this]{
        if (!m_socket)
            return;
        if (!m_pong) {
            stopCar();
            delete m_socket;
            m_socket = nullptr;
            return;
        }
        m_pong = false;
        m_socket->write(&PingPong, sizeof(char));
    });
    pingPongTimer->start(500);
    auto surroundCheckTimer = new QTimer(this);
    connect(surroundCheckTimer, &QTimer::timeout, this, [this] {
        const uint32_t fD = m_front.distanceCM();
        const uint32_t lD = m_left.distanceCM();
        const uint32_t rD = m_right.distanceCM();
        const uint32_t bD = m_rear.distanceCM();
        m_front.trigger();
        m_left.trigger();
        m_right.trigger();
        m_rear.trigger();
        if (m_socket && m_sendSensorsTimer.elapsed() >= 500) {
            m_sendSensorsTimer.restart();
            QByteArray buff;
            buff.append(Distance);
            buff.append(reinterpret_cast<const char*>(&fD), sizeof(fD));
            buff.append(reinterpret_cast<const char*>(&lD), sizeof(lD));
            buff.append(reinterpret_cast<const char*>(&rD), sizeof(rD));
            buff.append(reinterpret_cast<const char*>(&bD), sizeof(bD));
            m_socket->write(buff);
        }
        switch (m_currentDirection) {
        case Forward:
            if (fD < 50)
                stopCar();
            break;
        case Left:
        case Right:
            if (lD < 20 || rD < 20)
                stopCar();
            break;
        case Backward:
            if (bD < 50)
                stopCar();
            break;
        }
    });
    surroundCheckTimer->start(50);
    stopCar();
}

void Server::acceptConnection()
{
    delete m_socket;
    m_socket = m_server.nextPendingConnection();
    m_pong = true;
    m_sendSensorsTimer.restart();
    connect(m_socket, &QTcpSocket::readyRead, this, &Server::readClientData);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Server::socketError);
    connect(m_socket, &QTcpSocket::disconnected, this, &Server::socketError);
    m_lcd.clear();
    m_lcd.setCursor(0, 0);
    m_lcd.write("Connected");
}

void Server::readClientData()
{
    const auto buff = m_socket->readAll();
    if (buff.isEmpty())
        return;
    char lastCommand = 0;
    for (const auto c : buff) {
        switch (c) {
        case PingPong:
            m_pong = true;
            break;
        case Shutdown:
            stopCar();
            m_lcd.clear();
            m_lcd.setCursor(0, 0);
            m_lcd.write("Goodnight...");
            system("poweroff");
            qApp->quit();
            break;
        default:
            lastCommand = c;
            break;
        }
    }
    m_lcd.setCursor(8, 1);
    switch(lastCommand) {
    case Stop:
        stopCar();
        break;
    case Forward:
        if (m_front.distanceCM() < 50) {
            stopCar();
            break;
        }
        m_lcd.write(UpCh);
        m_currentDirection = Forward;
        gpioWrite(LeftEngFr, 1);
        gpioWrite(LeftEngBk, 0);
        gpioWrite(RightEngFr, 1);
        gpioWrite(RightEngBk, 0);

        gpioWrite(LeftEngEn, 1);
        gpioWrite(RightEngEn, 1);
        break;
    case Left:
        if (m_left.distanceCM() < 20 || m_right.distanceCM() < 20) {
            stopCar();
            break;
        }
        m_lcd.write(LeftCh);
        m_currentDirection = Left;
        gpioWrite(LeftEngFr, 1);
        gpioWrite(LeftEngBk, 0);
        gpioWrite(RightEngFr, 0);
        gpioWrite(RightEngBk, 1);

        gpioWrite(LeftEngEn, 1);
        gpioWrite(RightEngEn, 1);
        break;
    case Right:
        if (m_right.distanceCM() < 20 || m_left.distanceCM() < 20) {
            stopCar();
            break;
        }
        m_lcd.write(RightCh);
        m_currentDirection = Right;
        gpioWrite(LeftEngFr, 0);
        gpioWrite(LeftEngBk, 1);
        gpioWrite(RightEngFr, 1);
        gpioWrite(RightEngBk, 0);

        gpioWrite(LeftEngEn, 1);
        gpioWrite(RightEngEn, 1);
        break;
    case Backward:
        if (m_rear.distanceCM() < 50) {
            stopCar();
            break;
        }
        m_lcd.write(DownCh);
        m_currentDirection = Backward;
        gpioWrite(LeftEngFr, 0);
        gpioWrite(LeftEngBk, 1);
        gpioWrite(RightEngFr, 0);
        gpioWrite(RightEngBk, 1);

        gpioWrite(LeftEngEn, 1);
        gpioWrite(RightEngEn, 1);
        break;

    }
}

void Server::socketError()
{
    stopCar();
    m_lcd.clear();
    m_lcd.setCursor(0 ,0);
    m_lcd.write("Disconnected");
}

void Server::stopCar()
{
    gpioWrite(LeftEngEn, 0);
    gpioWrite(RightEngEn, 0);
    m_currentDirection = Stop;
    m_lcd.setCursor(8, 1);
    m_lcd.write("X");
}
