#ifndef STM32_TM1638_H
#define STM32_TM1638_H

#include "stm32f1xx_hal.h"

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} TM_GPIO_PinConfig_t;

typedef enum
{
	INPUT=GPIO_MODE_INPUT,
	OUTPUT=GPIO_MODE_OUTPUT_PP
} TM_GPIO_PinMode_t;

typedef enum
{
	LOW = GPIO_PIN_RESET,
	HIGH = GPIO_PIN_SET
}TM_PinState;

typedef enum
{
	LSBFIRST,
	MSBFIRST
} TM_Bit_Order;

void stm_pinMode(TM_GPIO_PinConfig_t gpio, TM_GPIO_PinMode_t mode);
void stm_digitalWrite(TM_GPIO_PinConfig_t gpio, TM_PinState PinState);
void stm_shiftOut(TM_GPIO_PinConfig_t dataPin, TM_GPIO_PinConfig_t clockPin, TM_Bit_Order bitOrder, uint8_t val);
uint8_t stm_shiftIn(TM_GPIO_PinConfig_t dataPin, TM_GPIO_PinConfig_t clockPin, TM_Bit_Order bitOrder);

#define pinMode(gpio, mode) stm_pinMode(gpio, mode)
#define digitalWrite(gpio, state) stm_digitalWrite(gpio, state)
#define shiftOut(dataPin, clockPin, bitOrder, val) stm_shiftOut(dataPin, clockPin, bitOrder, val)
#define shiftIn(dataPin, clockPin, bitOrder) stm_shiftIn(dataPin, clockPin, bitOrder)
#endif // STM32_TM1638_H
