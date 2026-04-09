#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

extern "C" {
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"
#include "sysctl_pll.h"
#include "FreeRTOS.h"
#include "task.h"
}

#include "buzzer.h"
#include "stopwatch.h"
#include "joystick_nav.h"
#include "screen_mic.h"

enum ScreenID : uint8_t {
    SCREEN_STOPWATCH = 0,
    SCREEN_MIC,
    SCREEN_COUNT_VAL
};

volatile uint8_t gCurrentScreen = SCREEN_STOPWATCH;

static tContext gContext;
static uint32_t gSystemClock;

static void initializeDisplay(void)
{
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    GrContextInit(&gContext, &g_sCrystalfontz128x128);
    GrContextFontSet(&gContext, &g_sFontFixed6x8);

    tRectangle full = {0, 0, 127, 127};
    GrContextForegroundSet(&gContext, ClrBlack);
    GrRectFill(&gContext, &full);
}

static void DrawHeader(const char *title)
{
    tRectangle header = {0, 0, 127, 20};
    GrContextForegroundSet(&gContext, ClrDarkBlue);
    GrRectFill(&gContext, &header);

    GrContextForegroundSet(&gContext, ClrYellow);
    GrStringDraw(&gContext, "<", -1, 4, 6, false);
    GrStringDraw(&gContext, ">", -1, 118, 6, false);

    GrContextForegroundSet(&gContext, ClrWhite);
    GrStringDrawCentered(&gContext, title, -1, 64, 10, false);
}

void DisplayTask(void *pvParams)
{
    for (;;) {
        tRectangle bg = {0, 0, 127, 127};
        GrContextForegroundSet(&gContext, ClrBlack);
        GrRectFill(&gContext, &bg);

        switch (gCurrentScreen) {
        case SCREEN_STOPWATCH:
            DrawHeader("STOPWATCH");
            Stopwatch_Draw(&gContext);
            break;
        case SCREEN_MIC:
            DrawHeader("MICROPHONE");
            ScreenMic_Draw(&gContext);
            break;
        }

        GrFlush(&gContext);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void)
{
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();

    gSystemClock = SysCtlClockFreqSet(
        SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,
        120000000);

    initializeDisplay();
    Buzzer_Init(gSystemClock);
    Stopwatch_Init();
    ScreenMic_Init();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    IntMasterEnable();

    xTaskCreate(TimeTask,    "Time",    512, NULL, 3, NULL);
    xTaskCreate(ButtonTask,  "Btn",     256, NULL, 2, NULL);
    xTaskCreate(DisplayTask, "Disp",    512, NULL, 2, NULL);
    xTaskCreate(BuzzerTask,  "Buzz",    256, NULL, 1, NULL);
    xTaskCreate(JoystickTask,"Joy",     256, NULL, 2, NULL);
    xTaskCreate(MicTask,     "Mic",     512, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {}
}
