#ifndef JOYSTICK_NAV_H
#define JOYSTICK_NAV_H

#include <stdint.h>

extern "C" {
#include "FreeRTOS.h"
}

void JoystickTask(void *pvParams);

#endif
