#ifndef TM4C_ELAPSED_TIME_H
#define TM4C_ELAPSED_TIME_H

#include <stdint.h>
#include "timerLib.h"

// Librería de medición de tiempo transcurrido basada en Timer
// Inspirada en elapsedMillis/elapsedMicros de PJRC, pero usando un Timer backend
// Uso típico:
//   Timer t; t.begin(sysclkHz, TIMER0_BASE);
//   elapsedMillis  em(t);
//   elapsedMicros  eu(t);
//   elapsedSeconds es(t);
//   if (em >= 1000) { /* 1 s */ em = 0; }

class elapsedMicros {
private:
    Timer* m_timer;         // backend
    uint64_t m_usStart;     // offset lógico (como en PJRC)

public:
    // Construye tomando instantánea actual en us
    explicit elapsedMicros(Timer& timer)
        : m_timer(&timer), m_usStart(timer.micros()) {}

    // Construye y fija como si ya hubieran pasado 'val' microsegundos
    elapsedMicros(Timer& timer, uint64_t val)
        : m_timer(&timer), m_usStart(timer.micros() - val) {}

    // Copia: comparte mismo backend y offset
    elapsedMicros(const elapsedMicros& orig)
        : m_timer(orig.m_timer), m_usStart(orig.m_usStart) {}

    // Conversión implícita a microsegundos transcurridos
    operator uint64_t() const { return m_timer ? (m_timer->micros() - m_usStart) : 0ull; }

    // Asignación desde otro elapsedMicros: copia backend y offset
    elapsedMicros& operator=(const elapsedMicros& rhs) {
        m_timer = rhs.m_timer;
        m_usStart = rhs.m_usStart;
        return *this;
    }

    // Fija el valor actual (en us) como si ya hubiesen transcurrido 'val'
    elapsedMicros& operator=(uint64_t val) {
        if (m_timer) m_usStart = m_timer->micros() - val; else m_usStart = 0ull;
        return *this;
    }

    // Ajustes relativos del offset (mantienen semántica PJRC)
    elapsedMicros& operator-=(uint64_t val) { m_usStart += val; return *this; }
    elapsedMicros& operator+=(uint64_t val) { m_usStart -= val; return *this; }

    // Operadores que devuelven una copia con offset ajustado
    elapsedMicros operator-(uint64_t val) const { elapsedMicros r(*this); r.m_usStart += val; return r; }
    elapsedMicros operator+(uint64_t val) const { elapsedMicros r(*this); r.m_usStart -= val; return r; }
};

class elapsedMillis {
private:
    Timer* m_timer;
    uint64_t m_msStart;

public:
    explicit elapsedMillis(Timer& timer)
        : m_timer(&timer), m_msStart(timer.millis()) {}

    elapsedMillis(Timer& timer, uint64_t val)
        : m_timer(&timer), m_msStart(timer.millis() - val) {}

    elapsedMillis(const elapsedMillis& orig)
        : m_timer(orig.m_timer), m_msStart(orig.m_msStart) {}

    operator uint64_t() const { return m_timer ? (m_timer->millis() - m_msStart) : 0ull; }

    elapsedMillis& operator=(const elapsedMillis& rhs) {
        m_timer = rhs.m_timer;
        m_msStart = rhs.m_msStart; return *this;
    }

    elapsedMillis& operator=(uint64_t val) {
        if (m_timer) m_msStart = m_timer->millis() - val; else m_msStart = 0ull; return *this;
    }

    elapsedMillis& operator-=(uint64_t val) { m_msStart += val; return *this; }
    elapsedMillis& operator+=(uint64_t val) { m_msStart -= val; return *this; }

    elapsedMillis operator-(uint64_t val) const { elapsedMillis r(*this); r.m_msStart += val; return r; }
    elapsedMillis operator+(uint64_t val) const { elapsedMillis r(*this); r.m_msStart -= val; return r; }
};

class elapsedSeconds {
private:
    Timer* m_timer;
    uint64_t m_msStart;         // se guarda en ms para coincidir con PJRC
    static constexpr uint64_t kMsPerSec = 1000ull;

public:
    explicit elapsedSeconds(Timer& timer)
        : m_timer(&timer), m_msStart(timer.millis()) {}

    elapsedSeconds(Timer& timer, uint64_t seconds)
        : m_timer(&timer), m_msStart(timer.millis() - seconds * kMsPerSec) {}

    elapsedSeconds(const elapsedSeconds& orig)
        : m_timer(orig.m_timer), m_msStart(orig.m_msStart) {}

    // Número de segundos transcurridos (entero)
    operator uint64_t() const {
        if (!m_timer) return 0ull;
        uint64_t dms = (m_timer->millis() - m_msStart);
        return dms / kMsPerSec;
    }

    elapsedSeconds& operator=(const elapsedSeconds& rhs) {
        m_timer = rhs.m_timer;
        m_msStart = rhs.m_msStart; return *this;
    }

    elapsedSeconds& operator=(uint64_t seconds) {
        if (m_timer) m_msStart = m_timer->millis() - seconds * kMsPerSec; else m_msStart = 0ull; return *this;
    }

    elapsedSeconds& operator-=(uint64_t seconds) { m_msStart += seconds * kMsPerSec; return *this; }
    elapsedSeconds& operator+=(uint64_t seconds) { m_msStart -= seconds * kMsPerSec; return *this; }

    elapsedSeconds operator-(uint64_t seconds) const { elapsedSeconds r(*this); r.m_msStart += seconds * kMsPerSec; return r; }
    elapsedSeconds operator+(uint64_t seconds) const { elapsedSeconds r(*this); r.m_msStart -= seconds * kMsPerSec; return r; }
};

#endif // TM4C_ELAPSED_TIME_H
