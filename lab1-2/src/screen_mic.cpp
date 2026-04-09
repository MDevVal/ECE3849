#include "screen_mic.h"

#include <math.h>
#include <stdio.h>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "grlib/grlib.h"
}

#define MIC_ADC_BASE    ADC0_BASE
#define MIC_ADC_SEQ     3
#define MIC_ADC_CHANNEL ADC_CTL_CH8
#define WINDOW_SIZE     128
#define A_REF           0.25f

volatile float gMicLevel = 0.0f;
volatile float gMicDb    = -60.0f;

static TimerHandle_t     xMicTimer    = NULL;
static SemaphoreHandle_t xMicReadySem = NULL;

static uint16_t gMicSamples[WINDOW_SIZE];
static volatile uint16_t gMicIndex = 0;

static void Mic_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    ADCSequenceConfigure(MIC_ADC_BASE, MIC_ADC_SEQ, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(MIC_ADC_BASE, MIC_ADC_SEQ, 0,
                             MIC_ADC_CHANNEL | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(MIC_ADC_BASE, MIC_ADC_SEQ);
    ADCIntClear(MIC_ADC_BASE, MIC_ADC_SEQ);
}

static uint16_t Mic_Read(void)
{
    uint32_t value;
    ADCProcessorTrigger(MIC_ADC_BASE, MIC_ADC_SEQ);
    while (!ADCIntStatus(MIC_ADC_BASE, MIC_ADC_SEQ, false)) {}
    ADCIntClear(MIC_ADC_BASE, MIC_ADC_SEQ);
    ADCSequenceDataGet(MIC_ADC_BASE, MIC_ADC_SEQ, &value);
    return (uint16_t)value;
}

static void MicSampleCb(TimerHandle_t xTimer)
{
    (void)xTimer;
    gMicSamples[gMicIndex++] = Mic_Read();
    if (gMicIndex >= WINDOW_SIZE) {
        gMicIndex = 0;
        xSemaphoreGive(xMicReadySem);
    }
}

void ScreenMic_Init(void)
{
    Mic_Init();
    xMicReadySem = xSemaphoreCreateBinary();
    xMicTimer = xTimerCreate("mic", pdMS_TO_TICKS(1), pdTRUE, NULL, MicSampleCb);
    xTimerStart(xMicTimer, 0);
}

void MicTask(void *pvParams)
{
    uint16_t localBuf[WINDOW_SIZE];

    for (;;) {
        xSemaphoreTake(xMicReadySem, portMAX_DELAY);

        for (int i = 0; i < WINDOW_SIZE; i++)
            localBuf[i] = gMicSamples[i];

        float sumSq = 0.0f;
        for (int i = 0; i < WINDOW_SIZE; i++) {
            float s = localBuf[i] / 4095.0f;
            float a = s - 0.5f;
            sumSq += a * a;
        }

        float rms = sqrtf(sumSq / WINDOW_SIZE);
        float db = 20.0f * log10f(rms / A_REF);

        if (db < -60.0f) db = -60.0f;
        if (db > 0.0f)   db = 0.0f;

        float level = (db + 60.0f) / 60.0f;
        if (level < 0.0f) level = 0.0f;
        if (level > 1.0f) level = 1.0f;

        gMicLevel = level;
        gMicDb = db;
    }
}

void ScreenMic_Draw(tContext *ctx)
{
    float level = gMicLevel;
    float db = gMicDb;

    GrContextForegroundSet(ctx, ClrCyan);
    GrStringDrawCentered(ctx, "MICROPHONE", -1, 64, 30, false);

    int16_t barLeft   = 24;
    int16_t barRight  = 104;
    int16_t barTop    = 45;
    int16_t barBottom = 75;

    tRectangle barBg = {barLeft, barTop, barRight, barBottom};
    GrContextForegroundSet(ctx, ClrDarkGray);
    GrRectFill(ctx, &barBg);

    int16_t fillWidth = (int16_t)((barRight - barLeft) * level);
    if (fillWidth > 0) {
        uint32_t barColor;
        if (level < 0.40f)
            barColor = ClrGreen;
        else if (level < 0.75f)
            barColor = ClrYellow;
        else
            barColor = ClrRed;

        tRectangle barFill = {barLeft, barTop,
                              (int16_t)(barLeft + fillWidth), barBottom};
        GrContextForegroundSet(ctx, barColor);
        GrRectFill(ctx, &barFill);
    }

    GrContextForegroundSet(ctx, ClrWhite);
    GrRectDraw(ctx, &barBg);

    char str[32];
    snprintf(str, sizeof(str), "%.1f dB", (double)db);
    GrContextForegroundSet(ctx, ClrWhite);
    GrStringDrawCentered(ctx, str, -1, 64, 90, false);

    snprintf(str, sizeof(str), "Level: %.0f%%", (double)(level * 100.0f));
    GrContextForegroundSet(ctx, ClrGray);
    GrStringDrawCentered(ctx, str, -1, 64, 105, false);
}
