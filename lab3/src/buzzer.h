#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
}

struct BuzzerCmd {
    uint32_t freq_hz;
    uint32_t duration_ms;
};

void Buzzer_Init(uint32_t systemClock);
void BuzzerTask(void *pvParams);

#endif
