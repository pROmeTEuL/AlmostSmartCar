#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include <QTcpSocket>
#include <sstream>

class Control : public QObject
{
    Q_OBJECT
    // proprietatile clasei control
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(int frontS READ frontS NOTIFY frontSChanged)
    Q_PROPERTY(int leftS READ leftS NOTIFY leftSChanged)
    Q_PROPERTY(int rightS READ rightS NOTIFY rightSChanged)
    Q_PROPERTY(int rearS READ rearS NOTIFY rearSChanged)

public:
    explicit Control(QObject *parent = nullptr);
    bool connected() const;
    int frontS() const;
    int leftS() const;
    int rightS() const;
    int rearS() const;

public slots: // functii care pot fi apelate din QML (prin "_control")
    void setJoyPos(double x, double y);
    void connectToCar();
    void shutdownServer();

private:
    void setConnected(bool newConnected);
    void socketConnected();
    void socketError();
    void readServerData();

signals:
    void connectedChanged();
    void frontSChanged();
    void rearSChanged();
    void leftSChanged();
    void rightSChanged();

private:
    QTcpSocket m_socket;
    bool m_connected = false;
    QByteArray m_serverData;
    int m_frontS = 0;
    int m_rearS = 0;
    int m_leftS = 0;
    int m_rightS = 0;
};

#endif // CONTROL_H
