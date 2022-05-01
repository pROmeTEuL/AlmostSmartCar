#ifndef SONIC_H
#define SONIC_H

#include <QMutex>

// clasa pt masurarea distantei folosind senzorul HC-SR04
class Sonic
{
public:
    Sonic(int trig, int echo, char cr);
    uint32_t distanceCM() const;
    void trigger();

private:
    static void echoCallback(int event, int level, uint32_t tick, void *userdata);

private:
    mutable QMutex m_mutex;
    bool m_triggered= false;
    uint32_t m_distance = 0;
    uint32_t m_startTime = 0;
    int m_trig;
    int m_echo;
    char m_cr;
};

#endif // SONIC_H
