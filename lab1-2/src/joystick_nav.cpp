#include "joystick_nav.h"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

#include "joystick.h"
#include "pins.h"

extern volatile uint8_t gCurrentScreen;

#define SCREEN_COUNT 2

static Joystick js(JSX, JSY, JS1);

void JoystickTask(void *pvParams)
{
    js.begin();
    js.calibrateCenter(32);

    bool readyForNextMove = true;

    for (;;) {
        js.tick();

        JoystickDir dir = js.direction8();

        if (dir == JoystickDir::Center) {
            readyForNextMove = true;
        } else if (readyForNextMove && dir == JoystickDir::E) {
            gCurrentScreen = (gCurrentScreen + 1u < SCREEN_COUNT)
                ? (gCurrentScreen + 1u) : 0u;
            readyForNextMove = false;
        } else if (readyForNextMove && dir == JoystickDir::W) {
            gCurrentScreen = (gCurrentScreen > 0u)
                ? (gCurrentScreen - 1u) : (SCREEN_COUNT - 1u);
            readyForNextMove = false;
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
