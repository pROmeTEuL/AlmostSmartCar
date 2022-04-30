#include "control.h"
#include <QDebug>
#include <QTimer>
#include <cmath>

static const char Forward = 'f';
static const char Left = 'l';
static const char Right = 'r';
static const char Backward = 'b';
static const char Stop = 's';
static const char PingPong = 'p';
static const char Distance = 'd';

Control::Control(QObject *parent) : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Control::socketError);
    connect(&m_socket, &QTcpSocket::disconnected, this, &Control::socketError);
    connect(&m_socket, &QTcpSocket::connected, this, &Control::socketConnected);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Control::readServerData);
}

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
        if (x > 0) {
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
    m_socket.connectToHost("192.168.5.1", 5706);
    setConnected(true);
}

void Control::setConnected(bool newConnected)
{
    if (m_connected == newConnected)
        return;

    m_connected = newConnected;
    emit connectedChanged();
}

void Control::socketConnected()
{
    setConnected(true);
}

void Control::socketError()
{
    setConnected(false);
}

void Control::readServerData()
{
    m_serverData.append(m_socket.readAll());
    if (m_serverData.isEmpty())
        return;
    while(!m_serverData.isEmpty()) {
        switch (m_serverData[0]) {
        case Distance:
            if (m_serverData.length() < sizeof(char) + sizeof(uint32_t) * 4)
                return;
            m_serverData.remove(0, sizeof(char));
            m_frontS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            m_leftS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            m_rightS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));
            m_rearS = *reinterpret_cast<const uint32_t*>(m_serverData.constData());
            m_serverData.remove(0, sizeof(uint32_t));

            emit frontSChanged();
            emit leftSChanged();
            emit rightSChanged();
            emit rearSChanged();
            break;
        case PingPong:
            m_socket.write(&PingPong, 1);
            m_serverData.remove(0, sizeof(char));
            break;
        default:
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

