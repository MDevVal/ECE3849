# Ejemplo: Click simple y doble click (sin RTOS)

Este ejemplo muestra cómo usar la librería `Button` para:
- Encender LED1 (PN0) mientras el botón está presionado
- Encender LED2 (PN1) con doble click
- Apagar LED2 con un click simple

Placa objetivo: TM4C1294XL LaunchPad (TivaC). Ajusta pines si tu placa es diferente.

## Pines
- Botón: `USR_SW1` (definido en `pins.h`, normalmente PJ0 con pull-up)
- LED1: PN0
- LED2: PN1

## Archivos
- `example/callbacks_example.cpp`: ejemplo usando callbacks estilo OneButton
- `example/interrupts_example.cpp`: ejemplo con IRQ en GPIO y tick en ISR de puerto
- `example/timer_example.cpp`: ejemplo con TIMER0A generando tick periódico en ISR

## Cómo compilar/ejecutar
Este proyecto usa TivaWare/DriverLib. Integra estos archivos a tu proyecto CCS/Energia/Makefile y asegúrate de:
- Incluir headers: `inc/`, `driverlib/`
- Linkear contra `driverlib.lib`
- Incluir `button.cpp` y `button.h` en tu proyecto
- Incluir tu `pins.h` con el mapeo correcto (define `USR_SW1`)

### Notas
- El constructor de `Button` configura el pin con pull-up por defecto (activo-bajo). Puedes cambiarlo con `ButtonPull::PullDown` si tu hardware lo requiere.
- `tick()` debe llamarse periódicamente (~10–20 ms). Puedes hacerlo desde:
	- el bucle principal (callbacks_example),
	- la ISR del puerto GPIO (interrupts_example), o
	- la ISR de un timer periódico (timer_example).
- Puedes configurar los tiempos en milisegundos (debounce, click/doble click, long press) con: `setDebounceMs`, `setClickMs`, `setPressMs`, `setIdleMs`, `setLongPressIntervalMs`.
- Callbacks disponibles: `attachPress`, `attachClick`, `attachDoubleClick`, `attachMultiClick`, `attachLongPressStart`, `attachDuringLongPress`, `attachLongPressStop`, `attachIdle`.
