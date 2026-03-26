
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "Crystalfontz128x128_ST7735.h"
#include "grlib/grlib.h"
#include "sysctl_pll.h"
#include "FreeRTOS.h"
#include "task.h"
}

#include "button.h"
#include "timerLib.h"
#include "elapsedTime.h"
#include "pins.h"

// ===== Global configuration =====
#define LED_PORT_BASE GPIO_PORTN_BASE
#define LED1_PIN      GPIO_PIN_0
#define LED2_PIN      GPIO_PIN_1

typedef struct {
    uint32_t port;
    uint32_t pins;
    uint64_t period_ms;
} LedTaskPeriodicParams;

void led_task_periodic(void *pvParameters) {
    // === One-time setup (runs once when task starts) ===
    LedTaskPeriodicParams *params = (LedTaskPeriodicParams *)pvParameters;

    bool pin_state = false;
    // === Periodic loop ===
    for (;;) {
        // Do work here
        GPIOPinWrite(params->port, params->pins, pin_state ? params->pins : 0);
        pin_state = !pin_state;

        vTaskDelay(pdMS_TO_TICKS(params->period_ms));
    }
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================
int main(void)
{
    IntMasterDisable();
    FPUEnable();
    FPULazyStackingEnable();

    SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480,120000000);

    // Initialize LED GPIO
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));
    GPIOPinTypeGPIOOutput(LED_PORT_BASE, LED1_PIN | LED2_PIN);


    IntMasterEnable();


    static LedTaskPeriodicParams task_params_200ms = {
        .port = LED_PORT_BASE,
        .pins = LED2_PIN,
        .period_ms = 200,
    };
    TaskHandle_t led_task_200ms;
    xTaskCreate(&led_task_periodic, "200ms Blink", 512, &task_params_200ms, 0, &led_task_200ms);

    static LedTaskPeriodicParams task_params_2000ms = {
        .port = LED_PORT_BASE,
        .pins = LED1_PIN,
        .period_ms = 2000,
    };
    TaskHandle_t led_task_2000ms;
    xTaskCreate(&led_task_periodic, "2000ms Blink", 512, &task_params_2000ms, 0, &led_task_2000ms);
    vTaskStartScheduler();

    while (true) {
    }

}