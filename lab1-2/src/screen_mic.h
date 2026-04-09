#ifndef SCREEN_MIC_H
#define SCREEN_MIC_H

#include <stdint.h>

extern "C" {
#include "FreeRTOS.h"
#include "grlib/grlib.h"
}

extern volatile float gMicLevel;
extern volatile float gMicDb;

void ScreenMic_Init(void);
void MicTask(void *pvParams);
void ScreenMic_Draw(tContext *ctx);

#endif
