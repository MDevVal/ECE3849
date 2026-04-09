#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "pins.h"        // Energia pin mapping for TM4C1294XL

enum class ButtonPull {
    PullUp,
    PullDown,
    None,
};

class Button {
public:
    // Nuevas variables de estado f�sico y eventos
    bool _wasPressedFlag;
    bool _wasReleasedFlag;
    bool _currentPhysicalLevel;

    // Tipos de callback compatibles con OneButton
    typedef void (*callbackFunction)(void);
    typedef void (*parameterizedCallbackFunction)(void*);

    // Nota: el constructor habilita automáticamente el reloj del puerto GPIO
    // asociado al pin, espera a que el periférico esté listo y configura el
    // pin como entrada con pull-up interno (activo-bajo), usando el mapeo
    // de Energia provisto en pins.h.
    // Los parámetros en "ticks" se convierten a ms usando _tickIntervalMs (por defecto 20 ms)
    Button(uint8_t pin, uint32_t debounceTicks = 3,
           uint32_t longPressTicks = 50, uint32_t doubleClickTicks = 25,
           ButtonPull pull = ButtonPull::PullUp);

    // Bucle de sondeo
    void tick();                 // llamado periódicamente cada ~_tickIntervalMs
    void tick(bool activeLevel); // tick con nivel lógico (true=presionado)
    void begin();                // habilita reloj GPIO y configura el pin (llamar tras configurar el clock del MCU)

    // API de estado
    void setPull(ButtonPull pull);     // reconfigurar pull en tiempo de ejecución

    // Configuración de tiempos (ms) — estilo OneButton
    void setDebounceMs(int ms);
    void setClickMs(unsigned int ms);
    void setPressMs(unsigned int ms);
    void setIdleMs(unsigned int ms);
    void setLongPressIntervalMs(unsigned int ms);
    void setTickIntervalMs(unsigned int ms);

    // Registro de callbacks — estilo OneButton
    void attachPress(callbackFunction fn);
    void attachPress(parameterizedCallbackFunction fn, void* param);

    void attachClick(callbackFunction fn);
    void attachClick(parameterizedCallbackFunction fn, void* param);

    void attachDoubleClick(callbackFunction fn);
    void attachDoubleClick(parameterizedCallbackFunction fn, void* param);

    void attachMultiClick(callbackFunction fn);
    void attachMultiClick(parameterizedCallbackFunction fn, void* param);

    void attachLongPressStart(callbackFunction fn);
    void attachLongPressStart(parameterizedCallbackFunction fn, void* param);

    void attachLongPressStop(callbackFunction fn);
    void attachLongPressStop(parameterizedCallbackFunction fn, void* param);

    void attachDuringLongPress(callbackFunction fn);
    void attachDuringLongPress(parameterizedCallbackFunction fn, void* param);

    void attachIdle(callbackFunction fn);

    // Utilidades
    void reset();
    int  getNumberClicks() const;
    bool isIdle() const;
    bool isLongPressed() const;

    // Nuevos métodos para estado y eventos del botón
    bool wasPressed();      // Devuelve true si el botón fue presionado desde la última consulta y resetea la bandera
    bool isPressed() const; // Devuelve true si el botón está presionado actualmente
    bool wasReleased();     // Devuelve true si el botón fue soltado desde la última consulta y resetea la bandera

    // Nuevas funciones avanzadas
    bool isDoubleClicked(); // Devuelve true si hubo doble clic desde la última consulta y resetea la bandera
    void attachRelease(callbackFunction fn); // Registra un callback para el evento de soltado
    unsigned int getHoldTime() const; // Devuelve el tiempo en ms que el botón estuvo presionado en la última pulsación

private:
    // Máquina de estados tipo OneButton
    enum class StateMachine : int {
        OCS_INIT = 0,
        OCS_DOWN = 1,
        OCS_UP = 2,
        OCS_COUNT = 3,
        OCS_PRESS = 6,
        OCS_PRESSEND = 7,
    };

    // Hardware mapping
    uint8_t  _pin;
    uint32_t _portBase;
    uint8_t  _bitMask;
    ButtonPull _pullMode;
    uint32_t _periph;
    bool _initialized;

    // Temporización (ms)
    int _debounceMs;                 // debounce
    unsigned int _clickMs;           // ventana click/doble click
    unsigned int _pressMs;           // umbral long press
    unsigned int _idleMs;            // tiempo para idle
    unsigned int _longPressIntervalMs; // periodo DuringLongPress
    unsigned int _tickIntervalMs;    // incremento de tiempo en tick()

    // Estado interno
    StateMachine _state;
    bool _idleState;
    bool _debouncedLevel;
    bool _lastDebounceLevel;
    unsigned long _lastDebounceTime;
    unsigned long _now;
    unsigned long _startTime;
    unsigned long _lastDuringLongPressTime;
    int _nClicks;
    int _maxClicks;

    // Lectura/config de hardware
    bool readPhysical();
    void applyPadConfig();

    // Lógica FSM + debounce
    void fsm(bool activeLevel);
    void newState(StateMachine next);
    bool debounce(bool value);

    // Callbacks registrados
    callbackFunction _pressFunc = nullptr;
    parameterizedCallbackFunction _paramPressFunc = nullptr; void* _pressFuncParam = nullptr;

    callbackFunction _clickFunc = nullptr;
    parameterizedCallbackFunction _paramClickFunc = nullptr; void* _clickFuncParam = nullptr;

    callbackFunction _doubleClickFunc = nullptr;
    parameterizedCallbackFunction _paramDoubleClickFunc = nullptr; void* _doubleClickFuncParam = nullptr;

    callbackFunction _multiClickFunc = nullptr;
    parameterizedCallbackFunction _paramMultiClickFunc = nullptr; void* _multiClickFuncParam = nullptr;

    callbackFunction _longPressStartFunc = nullptr;
    parameterizedCallbackFunction _paramLongPressStartFunc = nullptr; void* _longPressStartFuncParam = nullptr;

    callbackFunction _longPressStopFunc = nullptr;
    parameterizedCallbackFunction _paramLongPressStopFunc = nullptr; void* _longPressStopFuncParam = nullptr;

    callbackFunction _duringLongPressFunc = nullptr;
    parameterizedCallbackFunction _paramDuringLongPressFunc = nullptr; void* _duringLongPressFuncParam = nullptr;

    callbackFunction _idleFunc = nullptr;
    // Variables para nuevas funciones
    bool _doubleClickedFlag;
    unsigned int _holdTimeMs;
    callbackFunction _releaseFunc;
};

#endif
