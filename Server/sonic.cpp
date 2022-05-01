#include "sonic.h"

#include <QDebug>
#include <pigpio.h>


// initializarea senzorului
Sonic::Sonic(int trig, int echo, char cr)
    : m_trig(trig)
    , m_echo(echo)
    , m_cr(cr)
{
    gpioSetMode(m_trig, PI_OUTPUT);
    gpioSetMode(m_echo, PI_INPUT);
    gpioWrite(m_trig, 0);
//     se creaza un alt thread iar "echoCallback" va fi chemat din acel thread
    gpioSetAlertFuncEx(m_trig, &Sonic::echoCallback, this);
    gpioSetAlertFuncEx(m_echo, &Sonic::echoCallback, this);
    trigger();
}

// intoarce ultima distanta masurata
uint32_t Sonic::distanceCM() const
{
    QMutexLocker lock(&m_mutex); // ne sincronizam cu threadul folosit de AlertFunc ^^^
    return m_distance;
}

// decalnsam o noua masurare
void Sonic::trigger()
{
    gpioTrigger(m_trig, 10, 1); // trimitem o unda sonica
}

// aceasta functie este apelata de catre pigpio pentru fiecare care schimbare de nivel al tensiunii
// pentru fiecare pin monitorizat (cu gpioSetAlertFuncEx)
void Sonic::echoCallback(int pinNr, int level, uint32_t tick, void *userdata)
{
    auto thiz = reinterpret_cast<Sonic*>(userdata);
    if (pinNr == thiz->m_trig) {
        if (!level) {
            thiz->m_triggered = true; // nu de putine ori Echo este chemat de mai multe ori pentru un singue Trigger,
                                      // folosind m_triggered, vom retine doar prima valoare
            thiz->m_startTime = 0;
        }
    } else {
        if (!thiz->m_triggered)
            return;
        if (level) {
            thiz->m_startTime = tick; // retinem timpul de inceput al echo-ului
        } else {
            thiz->m_triggered = false;
            if (tick < thiz->m_startTime) // la fiecare 72 de minute tick-ul se reseteaza la 0, asa ca ignoram aceasta cautare
                return;
            double time = tick - thiz->m_startTime; // in time avem timpul unui Echo complet (dus-intors)
            thiz->m_startTime = 0;
            uint32_t dist = time / 58;  // https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf
                                        // You can calculate the range through the time interval between sending
                                        // trigger signal and receiving echo signal.
                                        // Formula: uS / 58 = centimeters
                                        // or uS / 148 =inch;
                                        // or: the range = high level time * velocity (340M/S) / 2;
                                        // we suggest to use over 60ms measurement cycle,
                                        // in order to prevent trigger signal to the echo signal.
            if (dist > 400) // conform documentatiei, senzorul nu poate masura corect distante mai mari de 4 metrii
                dist = 400;
            QMutexLocker lock(&thiz->m_mutex); // ne sincronizam cu threadul folosit de Qt
            thiz->m_distance = dist;
//            qDebug() << "Care:" << thiz->m_cr << thiz->m_distance << event << level << tick << time;
        }
    }
}
