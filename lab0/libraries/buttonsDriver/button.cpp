#include "button.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

// Las tablas port_to_base, digital_pin_to_port y digital_pin_to_bit_mask
// son provistas por pins.h (incluido por button.h)

Button::Button(uint8_t pin, uint32_t debounceTicks,
                             uint32_t longPressTicks, uint32_t doubleClickTicks,
                             ButtonPull pull)
    : _pin(pin),
      _pullMode(pull),
    _periph(0),
    _initialized(false),
      _debounceMs((int)(debounceTicks * 20)),
      _clickMs((unsigned int)(doubleClickTicks * 20)),
      _pressMs((unsigned int)(longPressTicks * 20)),
      _idleMs(1000),
      _longPressIntervalMs(0),
      _tickIntervalMs(20),
      _state(StateMachine::OCS_INIT),
      _idleState(false),
      _debouncedLevel(false),
      _lastDebounceLevel(false),
      _lastDebounceTime(0),
      _now(0),
      _startTime(0),
      _lastDuringLongPressTime(0),
      _nClicks(0),
    _maxClicks(2)
        , _wasPressedFlag(false)
        , _wasReleasedFlag(false)
        , _currentPhysicalLevel(false)
        , _doubleClickedFlag(false)
        , _holdTimeMs(0)
        , _releaseFunc(nullptr)
{
    uint8_t portIndex = digital_pin_to_port[_pin];
    _portBase = port_to_base[portIndex];
    _bitMask  = digital_pin_to_bit_mask[_pin];

    // Guardar periférico; la habilitación se hace en begin()
    _periph = sysctl_periph_for_port(portIndex);
}

bool Button::readPhysical() {
    uint32_t v = GPIOPinRead(_portBase, _bitMask);
    bool levelHigh = (v & _bitMask) != 0;
    // Si hay pull-up, consideramos el botón activo-bajo (presionado = 0)
    // Si hay pull-down, activo-alto (presionado = 1)
    if (_pullMode == ButtonPull::PullUp) {
        return !levelHigh;
    } else if (_pullMode == ButtonPull::PullDown) {
        return levelHigh;
    } else {
        // Sin pull: se asume activo-bajo por defecto
        return !levelHigh;
    }
}

void Button::applyPadConfig() {
    if (_pullMode == ButtonPull::PullUp) {
        GPIOPadConfigSet(_portBase, _bitMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    } else if (_pullMode == ButtonPull::PullDown) {
        GPIOPadConfigSet(_portBase, _bitMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    } else {
        GPIOPadConfigSet(_portBase, _bitMask, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    }
}

void Button::setPull(ButtonPull pull) {
    _pullMode = pull;
    if (_initialized) applyPadConfig();
}

// Debounce similar a OneButton
bool Button::debounce(bool value) {
    // Avanza el reloj de software
    _now += _tickIntervalMs;

    // No aplicar debounce al flanco a activo si _debounceMs < 0 (no usado aquí)
    if (value && _debounceMs < 0) {
        _debouncedLevel = value;
    }

    if (_lastDebounceLevel == value) {
        if ((_now - _lastDebounceTime) >= (unsigned long) ( (_debounceMs<0)?- _debounceMs : _debounceMs))
            _debouncedLevel = value;
    } else {
        _lastDebounceTime = _now;
        _lastDebounceLevel = value;
    }
    return _debouncedLevel;
}

void Button::newState(StateMachine next) {
    _state = next;
}

void Button::fsm(bool activeLevel) {
    unsigned long waitTime = (_now - _startTime);

    switch (_state) {
    case StateMachine::OCS_INIT:
        // Idle callback
        if (!_idleState && (waitTime > _idleMs)) {
            if (_idleFunc) _idleFunc();
            _idleState = true;
        }
        if (activeLevel) {
            newState(StateMachine::OCS_DOWN);
            _startTime = _now;
            _nClicks = 0;
            if (_pressFunc) _pressFunc();
            if (_paramPressFunc) _paramPressFunc(_pressFuncParam);
            // Guardar tiempo de inicio de pulsación
            _holdTimeMs = 0;
        }
        break;

    case StateMachine::OCS_DOWN:
        if (!activeLevel) {
            newState(StateMachine::OCS_UP);
            _startTime = _now;
            // Calcular tiempo de pulsación
            _holdTimeMs = _now - (_startTime - waitTime);
            // Callback de release
            if (_releaseFunc) _releaseFunc();
        } else if (waitTime > _pressMs) {
            if (_longPressStartFunc) _longPressStartFunc();
            if (_paramLongPressStartFunc) _paramLongPressStartFunc(_longPressStartFuncParam);
            newState(StateMachine::OCS_PRESS);
        }
        break;

    case StateMachine::OCS_UP:
        // Contar como click corto
        _nClicks++;
        if (_nClicks == 2) {
            _doubleClickedFlag = true;
        }
        newState(StateMachine::OCS_COUNT);
        break;

    case StateMachine::OCS_COUNT:
        if (activeLevel) {
            newState(StateMachine::OCS_DOWN);
            _startTime = _now;
        } else if ((waitTime >= _clickMs) || (_nClicks == _maxClicks)) {
            if (_nClicks == 1) {
                if (_clickFunc) _clickFunc();
                if (_paramClickFunc) _paramClickFunc(_clickFuncParam);
            } else if (_nClicks == 2) {
                if (_doubleClickFunc) _doubleClickFunc();
                if (_paramDoubleClickFunc) _paramDoubleClickFunc(_doubleClickFuncParam);
            } else {
                if (_multiClickFunc) _multiClickFunc();
                if (_paramMultiClickFunc) _paramMultiClickFunc(_multiClickFuncParam);
            }
            reset();
        }
        break;

    case StateMachine::OCS_PRESS:
        if (!activeLevel) {
            newState(StateMachine::OCS_PRESSEND);
        } else {
            if ((_now - _lastDuringLongPressTime) >= _longPressIntervalMs) {
                if (_duringLongPressFunc) _duringLongPressFunc();
                if (_paramDuringLongPressFunc) _paramDuringLongPressFunc(_duringLongPressFuncParam);
                _lastDuringLongPressTime = _now;
            }
        }
        break;

    case StateMachine::OCS_PRESSEND:
        if (_longPressStopFunc) _longPressStopFunc();
        if (_paramLongPressStopFunc) _paramLongPressStopFunc(_longPressStopFuncParam);
        reset();
        break;
    }
}

void Button::tick() {
    if (!_initialized) { begin(); }
    bool physical = readPhysical();
    // Guardar el estado físico anterior
    bool prevPhysical = _currentPhysicalLevel;
    _currentPhysicalLevel = physical;
    // Detectar flanco de bajada (presionado)
    if (!prevPhysical && physical) {
        _wasPressedFlag = true;
    }
    // Detectar flanco de subida (soltado)
    if (prevPhysical && !physical) {
        _wasReleasedFlag = true;
    }
    // Debounce e índice de tiempo
    bool active = debounce(physical);
    fsm(active);
}

void Button::tick(bool activeLevel) {
    if (!_initialized) { begin(); }
    bool prevPhysical = _currentPhysicalLevel;
    _currentPhysicalLevel = activeLevel;
    if (!prevPhysical && activeLevel) {
        _wasPressedFlag = true;
    }
    if (prevPhysical && !activeLevel) {
        _wasReleasedFlag = true;
    }
    bool active = debounce(activeLevel);
    fsm(active);
}

// Setters de tiempo
void Button::setDebounceMs(int ms) { _debounceMs = ms; }
void Button::setClickMs(unsigned int ms) { _clickMs = ms; }
void Button::setPressMs(unsigned int ms) { _pressMs = ms; }
void Button::setIdleMs(unsigned int ms) { _idleMs = ms; }
void Button::setLongPressIntervalMs(unsigned int ms) { _longPressIntervalMs = ms; }
void Button::setTickIntervalMs(unsigned int ms) { _tickIntervalMs = ms; }

// Callbacks attach
void Button::attachPress(callbackFunction fn) { _pressFunc = fn; }
void Button::attachPress(parameterizedCallbackFunction fn, void* param) { _paramPressFunc = fn; _pressFuncParam = param; }

void Button::attachClick(callbackFunction fn) { _clickFunc = fn; }
void Button::attachClick(parameterizedCallbackFunction fn, void* param) { _paramClickFunc = fn; _clickFuncParam = param; }

void Button::attachDoubleClick(callbackFunction fn) { _doubleClickFunc = fn; if (_maxClicks < 2) _maxClicks = 2; }
void Button::attachDoubleClick(parameterizedCallbackFunction fn, void* param) { _paramDoubleClickFunc = fn; _doubleClickFuncParam = param; if (_maxClicks < 2) _maxClicks = 2; }

void Button::attachMultiClick(callbackFunction fn) { _multiClickFunc = fn; if (_maxClicks < 100) _maxClicks = 100; }
void Button::attachMultiClick(parameterizedCallbackFunction fn, void* param) { _paramMultiClickFunc = fn; _multiClickFuncParam = param; if (_maxClicks < 100) _maxClicks = 100; }

void Button::attachLongPressStart(callbackFunction fn) { _longPressStartFunc = fn; }
void Button::attachLongPressStart(parameterizedCallbackFunction fn, void* param) { _paramLongPressStartFunc = fn; _longPressStartFuncParam = param; }

void Button::attachLongPressStop(callbackFunction fn) { _longPressStopFunc = fn; }
void Button::attachLongPressStop(parameterizedCallbackFunction fn, void* param) { _paramLongPressStopFunc = fn; _longPressStopFuncParam = param; }

void Button::attachDuringLongPress(callbackFunction fn) { _duringLongPressFunc = fn; }
void Button::attachDuringLongPress(parameterizedCallbackFunction fn, void* param) { _paramDuringLongPressFunc = fn; _duringLongPressFuncParam = param; }

void Button::attachIdle(callbackFunction fn) { _idleFunc = fn; }

// Utilidades
void Button::reset() {
    _state = StateMachine::OCS_INIT;
    _nClicks = 0;
    _startTime = _now;
    _idleState = false;
    _wasPressedFlag = false;
    _wasReleasedFlag = false;
    _doubleClickedFlag = false;
    _holdTimeMs = 0;
}

int Button::getNumberClicks() const { return _nClicks; }
bool Button::isIdle() const { return _state == StateMachine::OCS_INIT; }
bool Button::isLongPressed() const { return _state == StateMachine::OCS_PRESS; }

// Nuevo método: Devuelve true si el botón fue presionado desde la última consulta y resetea la bandera
bool Button::wasPressed() {
    bool ret = _wasPressedFlag;
    _wasPressedFlag = false;
    return ret;
}

// Nuevo método: Devuelve true si el botón está presionado actualmente
bool Button::isPressed() const {
    return _currentPhysicalLevel;
}

// Nuevo método: Devuelve true si el botón fue soltado desde la última consulta y resetea la bandera
bool Button::wasReleased() {
    bool ret = _wasReleasedFlag;
    _wasReleasedFlag = false;
    return ret;
}

// Nuevo método: Devuelve true si hubo doble clic desde la última consulta y resetea la bandera
bool Button::isDoubleClicked() {
    bool ret = _doubleClickedFlag;
    _doubleClickedFlag = false;
    return ret;
}

// Nuevo método: Permite registrar un callback para el evento de soltado
void Button::attachRelease(callbackFunction fn) {
    _releaseFunc = fn;
}

// Nuevo método: Devuelve el tiempo en ms que el botón estuvo presionado en la última pulsación
unsigned int Button::getHoldTime() const {
    return _holdTimeMs;
}

void Button::begin() {
    if (_initialized) return;
    if (_periph != 0) {
        SysCtlPeripheralEnable(_periph);
        while (!SysCtlPeripheralReady(_periph)) { }
        GPIOPinTypeGPIOInput(_portBase, _bitMask);
        applyPadConfig();
    }
    _initialized = true;
}
