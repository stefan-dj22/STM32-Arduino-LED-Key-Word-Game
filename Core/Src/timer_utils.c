#include "timer_utils.h"
#include "main.h"  // For Error_Handler and other standard definitions

// Use SysTick instead of dedicated Timer
static volatile uint32_t tick_count = 0;

void TimerUtils_Init(void)
{
    // SysTick is already initialized by HAL_Init() in main.c
    // We don't need to do anything here
}

// Function to be called from SysTick_Handler
void TimerUtils_IncTick(void)
{
    tick_count++;
}

uint32_t TimerUtils_GetTick(void)
{
    return tick_count;
}

void SoftTimer_Start(SoftTimer_t* timer, uint32_t delay_ms)
{
    timer->start_time = TimerUtils_GetTick();
    timer->delay_ms = delay_ms;
    timer->is_running = 1;
}

uint8_t SoftTimer_IsExpired(SoftTimer_t* timer)
{
    if (!timer->is_running) {
        return 1;
    }
    
    uint32_t current_time = TimerUtils_GetTick();
    uint32_t elapsed = current_time - timer->start_time;
    
    if (elapsed >= timer->delay_ms) {
        timer->is_running = 0;
        return 1;
    }
    
    return 0;
} 