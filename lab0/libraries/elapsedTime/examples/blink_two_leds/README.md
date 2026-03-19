# Ejemplo: Blink de dos LEDs con elapsedTime

Este ejemplo para EK-TM4C1294XL parpadea PN1 (D1) cada 500 ms y PN0 (D2) cada 200 ms usando `elapsedMillis` de la librería `elapsedTime` y el backend `Timer`.

## Archivos
- `main.cpp`: código fuente del ejemplo.

## Notas de integración
- Asegúrate de incluir en el proyecto:
  - `Timer.h` y `Timer.cpp`
  - `elapsedTime/elapsedTime.h`
  - DriverLib (TivaWare).
- La función `SysCtlClockFreqSet` configura el sistema a 120 MHz; ajusta si tu configuración es distinta.
- Se usa `TIMER0_BASE` para el backend; puedes cambiar a otro TIMERx si está ocupado.

## Pines
- PN1 (GPIO_PORTN_BASE, GPIO_PIN_1) -> LED D1
- PN0 (GPIO_PORTN_BASE, GPIO_PIN_0) -> LED D2
