#include "sonic.h"

#include <QDebug>
#include <pigpio.h>

Sonic::Sonic(int trig, int echo, char cr)
    : m_trig(trig)
    , m_echo(echo)
    , m_cr(cr)
{
    gpioSetMode(m_trig, PI_OUTPUT);
    gpioSetMode(m_echo, PI_INPUT);
    gpioWrite(m_trig, 0);
    gpioSetAlertFuncEx(m_trig, &Sonic::echoCallback, this);
    gpioSetAlertFuncEx(m_echo, &Sonic::echoCallback, this);
    trigger();
}


uint32_t Sonic::distanceCM() const
{
    QMutexLocker lock(&m_mutex);
    return m_distance;
}

void Sonic::trigger()
{
    gpioTrigger(m_trig, 10, 1);
}

void Sonic::echoCallback(int event, int level, uint32_t tick, void *userdata)
{
    auto thiz = reinterpret_cast<Sonic*>(userdata);
    if (event == thiz->m_trig) {
        if (!level) {
            thiz->m_triggered = true;
            thiz->m_startTime = 0;
        }
    } else {
        if (level) {
            thiz->m_startTime = tick;
        } else {
            if (tick < thiz->m_startTime)
                return;
            double time = tick - thiz->m_startTime;
            thiz->m_triggered = false;
            thiz->m_startTime = 0;
            QMutexLocker lock(&thiz->m_mutex);
            thiz->m_distance = time / 58; // https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
                                          // You can calculate the range through
                                          // the time interval between sending
                                          // trigger signal and receiving echo
                                          // signal. Formula: uS / 58 = centimeters
                                          // or uS / 148 =inch;
                                          // or: the range = high level time * velocity (340M/S) / 2;
                                          // we suggest to use over 60ms measurement cycle,
                                          // in order to prevent trigger signal to the echo signal.
//            qDebug() << "Care:" << thiz->m_cr << thiz->m_distance << event << level << tick << time;
        }
    }
}
