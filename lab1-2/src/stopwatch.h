#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stdint.h>
#include <stdbool.h>

extern "C" {
#include "FreeRTOS.h"
#include "grlib/grlib.h"
}

extern volatile bool     gRunning;
extern volatile uint8_t  gHours, gMinutes, gSeconds;
extern volatile uint16_t gMillis;

void Stopwatch_Init(void);
void TimeTask(void *pvParams);
void ButtonTask(void *pvParams);
void Stopwatch_Draw(tContext *ctx);

#endif
