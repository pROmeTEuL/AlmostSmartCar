#include "control.h"
#include <QDebug>
#include <QTimer>
#include <cmath>

// comenzi retea

static const char Forward = 'f';
static const char Left = 'l';
static const char Right = 'r';
static const char Backward = 'b';
static const char Stop = 's';
static const char PingPong = 'p';
static const char Distance = 'd';
static const char ShutDown = 'z';

Control::Control(QObject *parent) : QObject(parent)
{
    // conectam semnalele socket-ului
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Control::socketError);
    connect(&m_socket, &QTcpSocket::disconnected, this, &Control::socketError);
    connect(&m_socket, &QTcpSocket::connected, this, &Control::socketConnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Control::readServerData);
}

// functia este apelata din qml cand se schimba pozitia joystick-ului
void Control::setJoyPos(double x, double y)
{
    qDebug() << x  << y;
    if (m_socket.state() != QTcpSocket::SocketState::ConnectedState)
        return;
    // x,y = -1 .. 1
    if (std::abs(x) < 0.25 && std::abs(y) < 0.25) {
        m_socket.write(&Stop, 1);
        return;
    }
    if (std::abs(x) > std::abs(y)) {
        if (x < 0) {
            m_socket.write(&Right, 1);
        } else {
            m_socket.write(&Left, 1);
        }
    } else {
        if (y < 0) {
            m_socket.write(&Forward, 1);
        } else {
            m_socket.write(&Backward, 1);
        }
    }

}

void Control::connectToCar()
{
//  se conecteaza socket-ul la adresa de mai jos
    m_socket.connectToHost("192.168.5.1", 5706);
    setConnected(true);
}

void Control::shutdownServer()
{
//    trimitem comanda de shutdown
    m_socket.write(&ShutDown, 1);
}

void Control::setConnected(bool newConnected)
{
//    setam proprietatea "connected"
    if (m_connected == newConnected)
        return;

    m_connected = newConnected;
    emit connectedChanged();
}

void Control::socketConnected()
{
    setConnected(true);
}

//  in caz de eroarea sau deconectare setam proprietatea pe fals
void Control::socketError()
{
    setConnected(false);
}

//  aceasta functie este apelata de fiecare data cand in socket avem date de citit
void Control::readServerData()
{
    m_serverData.append(m_socket.readAll());
    if (m_serverData.isEmpty())
        return;
    while(!m_serverData.isEmpty()) {
//         aici prelucrez comenzile
        switch (m_serverData[0]) {
        case Distance: //   comanda de distanta are urmatoarele valori:
            // 1 byte id-ul comenzii
            if (m_serverData.length() < sizeof(char) + sizeof(uint32_t) * 4)
                return;
            m_serverData.remove(0, sizeof(char));
            // 1 uint32 pt distanta senzorului din fata
            m_frontS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            // ---||--- stanga
            m_leftS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            // ---||--- dreapta
            m_rightS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            // ---||--- spate
            m_rearS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            // anunt qml-ul ca s-au schimbat valorile
            emit frontSChanged();
            emit leftSChanged();
            emit rightSChanged();
            emit rearSChanged();
            break;
        case PingPong: // comanda ping
            // raspundem imediat cu pong
            m_socket.write(&PingPong, 1);
            m_serverData.remove(0, sizeof(char));
            break;
        default: // in cazul in care primim date corupte
            qWarning() << "unknown command";
            m_serverData.remove(0, sizeof(char));
            break;
        }
    }
}

bool Control::connected() const
{
    return m_connected;
}

int Control::frontS() const
{
    return m_frontS;
}

int Control::leftS() const
{
    return m_leftS;
}

int Control::rightS() const
{
    return m_rightS;
}

int Control::rearS() const
{
    return m_rearS;
}

