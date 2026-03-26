#include "timerLib.h"

extern "C" {
  #include "driverlib/sysctl.h"
  #include "driverlib/timer.h"
  #include "inc/hw_memmap.h"
  #include "inc/hw_types.h"
}

Timer::Timer()
: m_base(0),
  m_isWide(false),
  m_sysclkHz(0),
  m_ticksPerUs(0),
  m_start32(0),
  m_start64(0)
{}

bool Timer::isWideBase(uint32_t base)
{
    switch (base) {
        case WTIMER0_BASE: case WTIMER1_BASE: case WTIMER2_BASE:
        case WTIMER3_BASE: case WTIMER4_BASE: case WTIMER5_BASE:
            return true;
        default:
            return false;
    }
}

static bool periphReady(uint32_t periph)
{
    // Espera breve a que el perif�rico quede listo
    for (volatile uint32_t guard = 0; guard < 120000; ++guard) {
        if (SysCtlPeripheralReady(periph)) return true;
    }
    return SysCtlPeripheralReady(periph);
}

bool Timer::enablePeripheralForBase(uint32_t base)
{
    switch (base) {
        // 32-bit GPTM
        case TIMER0_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); return periphReady(SYSCTL_PERIPH_TIMER0);
        case TIMER1_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); return periphReady(SYSCTL_PERIPH_TIMER1);
        case TIMER2_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2); return periphReady(SYSCTL_PERIPH_TIMER2);
        case TIMER3_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3); return periphReady(SYSCTL_PERIPH_TIMER3);
        case TIMER4_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4); return periphReady(SYSCTL_PERIPH_TIMER4);
        case TIMER5_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5); return periphReady(SYSCTL_PERIPH_TIMER5);
        // 64-bit Wide GPTM
        case WTIMER0_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0); return periphReady(SYSCTL_PERIPH_WTIMER0);
        case WTIMER1_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER1); return periphReady(SYSCTL_PERIPH_WTIMER1);
        case WTIMER2_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2); return periphReady(SYSCTL_PERIPH_WTIMER2);
        case WTIMER3_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3); return periphReady(SYSCTL_PERIPH_WTIMER3);
        case WTIMER4_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER4); return periphReady(SYSCTL_PERIPH_WTIMER4);
        case WTIMER5_BASE: SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5); return periphReady(SYSCTL_PERIPH_WTIMER5);
        default: return false;
    }
}

bool Timer::begin(uint32_t sysclkHz, uint32_t timerBase)
{
    m_base    = timerBase;
    m_isWide  = isWideBase(timerBase);
    m_sysclkHz = sysclkHz;

    if (!enablePeripheralForBase(timerBase)) return false;

    // ticks por microsegundo con redondeo
    // (evita sesgos cuando sysclkHz no es m�ltiplo exacto de 1 MHz)
    m_ticksPerUs = (m_sysclkHz + 500000u) / 1000000u;
    if (m_ticksPerUs == 0) m_ticksPerUs = 1; // evita divisi�n por cero

    // Configurar como peri�dico, down-counter, reloj del sistema
    TimerDisable(timerBase, TIMER_BOTH);
    TimerClockSourceSet(timerBase, TIMER_CLOCK_SYSTEM);

    if (m_isWide) {
        // 64-bit peri�dico
        TimerConfigure(timerBase, TIMER_CFG_PERIODIC);
        TimerLoadSet64(timerBase, 0xFFFFFFFFFFFFFFFFULL);  // down desde max
        TimerEnable(timerBase, TIMER_BOTH);                // habilita A y B como 64-bit
        m_start64 = TimerValueGet64(timerBase);
    } else {
        // 32-bit peri�dico
        TimerConfigure(timerBase, TIMER_CFG_PERIODIC);
        TimerLoadSet(timerBase, TIMER_A, 0xFFFFFFFFu);     // down desde max
        TimerEnable(timerBase, TIMER_A);
        m_start32 = TimerValueGet(timerBase, TIMER_A);
    }

    return true;
}

void Timer::stop()
{
    if (m_base) TimerDisable(m_base, m_isWide ? TIMER_BOTH : TIMER_A);
}

void Timer::reset()
{
    if (!m_base) return;
    if (m_isWide) {
        m_start64 = TimerValueGet64(m_base);
    } else {
        m_start32 = TimerValueGet(m_base, TIMER_A);
    }
}

uint64_t Timer::micros() const
{
    if (!m_base) return 0;

    if (m_isWide) {
        // Down-counter 64-bit
        uint64_t now    = TimerValueGet64(m_base);
        uint64_t dticks = (m_start64 - now);           // aritm�tica unsigned maneja wrap
        // ticks -> �s (div. por ticks/�s)
        return (m_ticksPerUs ? (dticks / m_ticksPerUs) : 0ULL);
    } else {
        // Down-counter 32-bit
        uint32_t now    = TimerValueGet(m_base, TIMER_A);
        uint32_t dticks = (m_start32 - now);           // unsigned maneja wrap (m�dulo 2^32)
        return (m_ticksPerUs ? (uint64_t)dticks / m_ticksPerUs : 0ULL);
    }
}

uint64_t Timer::millis() const
{
    // �s -> ms
    return micros() / 1000ULL;
}
