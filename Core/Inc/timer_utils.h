#ifndef TIMER_UTILS_H
#define TIMER_UTILS_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef struct {
    uint32_t start_time;
    uint32_t delay_ms;
    uint8_t is_running;
} SoftTimer_t;

#ifdef __cplusplus
extern "C" {
#endif

void TimerUtils_Init(void);
void TimerUtils_IncTick(void);
uint32_t TimerUtils_GetTick(void);
void SoftTimer_Start(SoftTimer_t* timer, uint32_t delay_ms);
uint8_t SoftTimer_IsExpired(SoftTimer_t* timer);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_UTILS_H */ 