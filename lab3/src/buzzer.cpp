#include "buzzer.h"
#include "game.h"
#include "app_objects.h"

extern "C" {
    #include "driverlib/gpio.h"
    #include "driverlib/sysctl.h"
    #include "driverlib/pwm.h"
    #include "driverlib/pin_map.h"
    #include "inc/hw_memmap.h"
    #include "FreeRTOS.h"
    #include "task.h"
    #include "queue.h"
}

#define BUZZER_PWM_BASE  PWM0_BASE
#define BUZZER_GEN       PWM_GEN_0
#define BUZZER_OUTNUM    PWM_OUT_1
#define BUZZER_OUTBIT    PWM_OUT_1_BIT

static uint32_t sBuzzerClock = 0;

void Buzzer_Init(uint32_t systemClock)
{
    sBuzzerClock = systemClock;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64);
}

static void Buzzer_Beep(uint32_t freq_hz, uint32_t duration_ms)
{
    if (freq_hz == 0) return;

    uint32_t pwmClock = sBuzzerClock / 64;
    uint32_t period = pwmClock / freq_hz;

    PWMGenConfigure(BUZZER_PWM_BASE, BUZZER_GEN, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(BUZZER_PWM_BASE, BUZZER_GEN, period);
    PWMPulseWidthSet(BUZZER_PWM_BASE, BUZZER_OUTNUM, period / 2);
    PWMOutputState(BUZZER_PWM_BASE, BUZZER_OUTBIT, true);
    PWMGenEnable(BUZZER_PWM_BASE, BUZZER_GEN);

    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    PWMOutputState(BUZZER_PWM_BASE, BUZZER_OUTBIT, false);
}

void BuzzerTask(void *pvParams)
{
    for (;;) {
        EventBits_t bits = xEventGroupWaitBits(
            xGameEvents,
            EVT_FOOD_EATEN | EVT_GAME_OVER,
            pdTRUE,       // clear the bits after waking
            pdFALSE,      // wake on any bit (not all)
            portMAX_DELAY
        );

        if (bits & EVT_FOOD_EATEN) {
            // short, high-pitched beep
            Buzzer_Beep(1000, 150);
        }
        if (bits & EVT_GAME_OVER) {
            // long, low-pitched tone
            Buzzer_Beep(400, 750);
        }
    }
}