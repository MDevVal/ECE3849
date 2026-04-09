#include "stopwatch.h"
#include "buzzer.h"

#include <stdio.h>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "grlib/grlib.h"
}

#include "button.h"
#include "pins.h"

#define BTN_PORT_BASE GPIO_PORTL_BASE
#define BTN_PIN_MASK  (GPIO_PIN_1 | GPIO_PIN_2)
#define BTN_INT_NUM   INT_GPIOL

volatile bool     gRunning  = false;
volatile uint8_t  gHours    = 0;
volatile uint8_t  gMinutes  = 0;
volatile uint8_t  gSeconds  = 0;
volatile uint16_t gMillis   = 0;

static SemaphoreHandle_t xBtnSem = NULL;
static Button btnS1(S1);
static Button btnS2(S2);

extern "C" void ButtonISR(void)
{
    GPIOIntClear(BTN_PORT_BASE, BTN_PIN_MASK);
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(xBtnSem, &woken);
    portYIELD_FROM_ISR(woken);
}

void Stopwatch_Init(void)
{
    btnS1.begin();
    btnS1.setTickIntervalMs(20);
    btnS1.setDebounceMs(30);

    btnS2.begin();
    btnS2.setTickIntervalMs(20);
    btnS2.setDebounceMs(30);

    xBtnSem = xSemaphoreCreateBinary();

    GPIOIntRegister(BTN_PORT_BASE, ButtonISR);
    GPIOIntTypeSet(BTN_PORT_BASE, BTN_PIN_MASK, GPIO_BOTH_EDGES);
    GPIOIntEnable(BTN_PORT_BASE, BTN_PIN_MASK);
    IntEnable(BTN_INT_NUM);
}

void TimeTask(void *pvParams)
{
    TickType_t lastTick;
    const uint32_t tickMs = 10;

    for (;;) {
        lastTick = xTaskGetTickCount();
        if (gRunning) {
            gMillis += tickMs;
            if (gMillis >= 1000) {
                gMillis = 0;
                if (++gSeconds >= 60) {
                    gSeconds = 0;
                    if (++gMinutes >= 60) {
                        gMinutes = 0;
                        ++gHours;
                    }
                }
            }
        }
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(tickMs));
    }
}

void ButtonTask(void *pvParams)
{
    for (;;) {
        xSemaphoreTake(xBtnSem, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(15));

        btnS1.tick();
        btnS2.tick();

        if (btnS1.wasPressed()) {
            gRunning = !gRunning;
            BuzzerCmd cmd = {1000, 200};
            xQueueSend(gBuzzerQ, &cmd, 0);
        }
        if (btnS2.wasPressed()) {
            gRunning = false;
            gHours = 0;
            gMinutes = 0;
            gSeconds = 0;
            gMillis = 0;
            BuzzerCmd cmd = {1500, 200};
            xQueueSend(gBuzzerQ, &cmd, 0);
        }
    }
}

void Stopwatch_Draw(tContext *ctx)
{
    char str[32];

    GrContextForegroundSet(ctx, ClrCyan);
    GrStringDrawCentered(ctx, "STOPWATCH", -1, 64, 30, false);

    snprintf(str, sizeof(str), "%02d:%02d:%02d.%03d",
             gHours, gMinutes, gSeconds, gMillis);

    GrContextForegroundSet(ctx, gRunning ? ClrYellow : ClrOlive);
    GrStringDrawCentered(ctx, str, -1, 64, 55, false);

    GrContextForegroundSet(ctx, ClrWhite);
    if (gRunning)
        GrStringDrawCentered(ctx, "Running", -1, 64, 75, false);
    else
        GrStringDrawCentered(ctx, "Stopped", -1, 64, 75, false);

    GrContextForegroundSet(ctx, ClrGray);
    GrStringDrawCentered(ctx, "S1:Play/Pause S2:Reset", -1, 64, 115, false);
}
