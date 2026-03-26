# elapsedTime (para TM4C + DriverLib)

Librería header-only que replica la API de `elapsedMillis/elapsedMicros` de PJRC pero utilizando tu clase `Timer` (basada en DriverLib) como backend. Permite medir microsegundos, milisegundos y segundos transcurridos de forma sencilla y sin interrupciones.

## Requisitos
- `Timer.h` y `Timer.cpp` incluidos en tu proyecto
- DriverLib TM4C (TivaWare) correctamente configurado

## Uso

1) Inicializa un `Timer` de 32 bits en modo periódico libre:

```cpp
#include "Timer.h"
#include "elapsedTime.h"

void setupTimer(Timer& t) {
    const uint32_t sysclkHz = 120000000; // ajusta a tu reloj
    t.begin(sysclkHz, TIMER0_BASE);      // o TIMERx_BASE que prefieras
}
```

2) Crea medidores de tiempo transcurrido y úsalos como enteros:

```cpp
Timer t;
setupTimer(t);

elapsedMillis em(t);   // ms transcurridos
elapsedMicros eu(t);   // us transcurridos
elapsedSeconds es(t);  // s  transcurridos

// ...
if (em >= 1000) {      // ~1 segundo
    em = 0;            // reinicia (como en PJRC)
}

uint64_t dt_us = eu;   // conversión implícita
```

### Constructores y operadores compatibles
- `elapsedMicros(Timer&)`, `elapsedMicros(Timer&, uint64_t val)`
- `elapsedMillis(Timer&)`, `elapsedMillis(Timer&, uint64_t val)`
- `elapsedSeconds(Timer&)`, `elapsedSeconds(Timer&, uint64_t seconds)`
- Conversión implícita a `uint64_t` para leer el tiempo transcurrido.
- Operadores `=`, `+=`, `-=`, `+`, `-` con semántica equivalente a la librería de PJRC.

## Notas
- La precisión depende del `sysclkHz` y la división en `Timer.cpp`. `micros()` usa división entera; a frecuencias altas la resolución es 1 us.
- El `Timer` es de 32 bits; el cálculo de delta maneja el down-counter y el wrap de 32 bits de forma segura.

## Licencia
Se inspira en la API de PJRC (MIT-like). Este header se publica bajo MIT.
