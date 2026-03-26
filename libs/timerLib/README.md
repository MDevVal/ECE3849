# Librería Timer (TM4C1294XL)

Fuente: Wide Timer 64-bit (WTIMERx) — independiente de SysTick y sin ISR.

API C++:
- `tm4c::Timer t; t.begin(sysclkHz, WTIMER0_BASE); t.millis(); t.micros();`
- Pasa `sysclkHz = SysCtlFrequencyGet(25_000_000)` tras configurar el PLL.

Notas:
- Se configura el Wide Timer en modo 64-bit count-up con periodo máximo y se toma
  como referencia el valor en el instante de `begin()` para reportar tiempo relativo.
- No toca SysTick, por lo que no interfiere con RTOS u otros usos que lo requieran.

Ejemplo:
```c++
tm4c::Timer t; // por defecto WideTimer64
t.begin(gSystemClock, WTIMER0_BASE); // o WTIMER1_BASE .. WTIMER5_BASE
auto ms = t.millis();
auto us = t.micros();
```