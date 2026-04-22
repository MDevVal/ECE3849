// Minimal Snake (FreeRTOS + GRLIB + Mutex)
// Hardware: TM4C1294XL + Crystalfontz 128x128 LCD

#include <stdint.h>
#include <stdbool.h>
#include <cstdlib>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "sysctl_pll.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
}

// Board drivers (provided in project includes)
#include "button.h"
#include "joystick.h"
#include "buzzer.h"

// App modules per lab structure
#include "app_objects.h"
#include "game.h"
#include "display.h"

// Shared objects
tContext gContext;
uint32_t gSysClk;

// Buttons used for pause/reset
static Button btnPause(S1);
static Button btnReset(S2);
// Joystick (axes + stick push). Pins from HAL pins.h (BOOSTERPACK1)
static Joystick gJoystick(JSX, JSY, JS1);

// Config
#define INPUT_TICK_MS   10U

static TaskHandle_t render_handle;

EventGroupHandle_t xGameEvents;

// Prototypes
static void configureSystemClock(void);
static void vInputTask(void *pvParameters);
static void vSnakeTask(void *pvParameters);
static void vRenderTask(void *pvParameters);

int main(void)
{
    xGameEvents = xEventGroupCreate();
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();

    configureSystemClock();

    // Init buttons and joystick
    btnPause.begin();
    btnReset.begin();
    gJoystick.begin();
    btnPause.setTickIntervalMs(INPUT_TICK_MS);
    btnReset.setTickIntervalMs(INPUT_TICK_MS);
    gJoystick.setTickIntervalMs(INPUT_TICK_MS);
    btnPause.setDebounceMs(30);
    btnReset.setDebounceMs(30);
    // Optional joystick tuning
    gJoystick.setDeadzone(0.15f);

    Buzzer_Init(gSysClk);

    IntMasterEnable();

    // Create tasks (priorities per lab suggestion)
    xTaskCreate(vInputTask,  "Input",  512, NULL, 2, NULL);
    xTaskCreate(vSnakeTask,  "Snake",  512, NULL, 2, NULL);
    xTaskCreate(vRenderTask, "Render", 768, NULL, 1, &render_handle);
    xTaskCreate(BuzzerTask,  "Buzzer", 512, NULL, 2, NULL);

    vTaskStartScheduler();
    while (1);
}

static void configureSystemClock(void)
{
    gSysClk = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
        SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        120000000);
}

// Reads joystick/buttons and updates gameState
static void vInputTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;) {
        // Hardware button + joystick polling
        btnPause.tick();
        btnReset.tick();
        gJoystick.tick();

        // Toggle pause on S1
        if (btnPause.wasPressed()) {
            if (gameState.state == PAUSED) {
                gameState.state = RUNNING;
            } else if (gameState.state == RUNNING) {
                gameState.state = PAUSED;
            }
        }
        // Request reset on S2
        if (btnReset.wasPressed()) {
            gameState.needsReset = true;
        }

        // Joystick 8-way direction mapping to game directions
        switch (gJoystick.direction8()) {
            case JoystickDir::N:
            case JoystickDir::NE:
            case JoystickDir::NW:
                if (gameState.currentDirection != DOWN) {
                    gameState.currentDirection = UP;
                }
                break;
            case JoystickDir::S:
            case JoystickDir::SE:
            case JoystickDir::SW:
                if (gameState.currentDirection != UP) {
                    gameState.currentDirection = DOWN;
                }
                break;
            case JoystickDir::E:
                if (gameState.currentDirection != LEFT) {
                    gameState.currentDirection = RIGHT;
                }
                break;
            case JoystickDir::W:
                if (gameState.currentDirection != RIGHT) {
                    gameState.currentDirection = LEFT;
                }
                break;
            case JoystickDir::Center:
            default:
                // keep last direction
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(INPUT_TICK_MS));
    }
}

// Advances the snake periodically
static void vSnakeTask(void *pvParameters)
{
    (void)pvParameters;
    ResetGame();
    for(;;){
        if (gameState.needsReset) {
            ResetGame();
        }
        if (gameState.state) {
            moveSnake();
        }
        xTaskNotifyGive(render_handle);
        vTaskDelay(pdMS_TO_TICKS(150 - 2 * gameState.score));
    }
}

// Renders current frame to LCD (guarded by mutex)
static void vRenderTask(void *pvParameters)
{
    (void)pvParameters;
    LCD_Init();
    TickType_t last = xTaskGetTickCount();
    for(;;)
    {
        DrawGame(&gameState);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

