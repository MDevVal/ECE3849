#ifndef TM4C_TIMER_H
#define TM4C_TIMER_H

#include <stdint.h>
#include <stdbool.h>

extern "C" {
  #include "driverlib/sysctl.h"
  #include "driverlib/timer.h"
  #include "inc/hw_memmap.h"
}

class Timer {
public:
    Timer();

    /**
     * @brief Inicializa el timer en modo periódico down-counter.
     * @param sysclkHz  Frecuencia de reloj del sistema (Hz), p.ej. 120000000.
     * @param timerBase TIMERx_BASE (32-bit) o WTIMERx_BASE (64-bit).
     *                  Ej: TIMER1_BASE o WTIMER0_BASE.
     * @return true si se pudo inicializar, false en error.
     */
    bool begin(uint32_t sysclkHz, uint32_t timerBase);

    /** Detiene el timer (no afecta configuración). */
    void stop();

    /** Reinicia el "origen" de tiempo (offset lógico). */
    void reset();

    /** Tiempo transcurrido en milisegundos desde el begin()/reset(). */
    uint64_t millis() const;

    /** Tiempo transcurrido en microsegundos desde el begin()/reset(). */
    uint64_t micros() const;

    /** Devuelve el base del timer configurado. */
    uint32_t base() const { return m_base; }

    /** Indica si está usando Wide Timer (64-bit). */
    bool isWide() const { return m_isWide; }

private:
    // Helpers
    static bool enablePeripheralForBase(uint32_t base);
    static bool isWideBase(uint32_t base);

    // Estado
    uint32_t m_base;           // TIMERx_BASE o WTIMERx_BASE
    bool     m_isWide;         // true si m_base es WTIMERx_BASE
    uint32_t m_sysclkHz;       // Hz del sistema
    uint32_t m_ticksPerUs;     // ticks de timer por microsegundo (redondeado)
    // "Origen" (valor capturado al comenzar / reset). Siempre lector del contador descendente.
    uint32_t m_start32;        // si 32-bit
    uint64_t m_start64;        // si 64-bit
};

#endif // TM4C_TIMER_H
