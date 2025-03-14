#include "stm32_tm1638.h"
#define BYTE_SIZE 8u

/*!
    @brief Configure the mode of a GPIO pin
    @param gpio GPIO pin configuration structure
    @param mode GPIO pin mode (INPUT or OUTPUT)
*/
void stm_pinMode(TM_GPIO_PinConfig_t gpio, TM_GPIO_PinMode_t mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = gpio.pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(gpio.port, &GPIO_InitStruct);
}

/*!
    @brief Write a digital value to a GPIO pin
    @param gpio GPIO pin configuration structure
    @param PinState GPIO pin state (LOW or HIGH)
*/
void stm_digitalWrite(TM_GPIO_PinConfig_t gpio, TM_PinState PinState)
{
    HAL_GPIO_WritePin(gpio.port, gpio.pin, (GPIO_PinState) PinState);
}

/*!
    @brief Shift out a byte of data one bit at a time
    @param dataPin GPIO pin configuration structure for data pin
    @param clockPin GPIO pin configuration structure for clock pin
    @param bitOrder Bit order (LSBFIRST or MSBFIRST)
    @param val Byte value to shift out
*/
void stm_shiftOut(TM_GPIO_PinConfig_t dataPin, TM_GPIO_PinConfig_t clockPin, TM_Bit_Order bitOrder, uint8_t val)
{
    int mask;
    for (int i=0; i < BYTE_SIZE; i++)
    {
        HAL_GPIO_WritePin(clockPin.port, clockPin.pin, GPIO_PIN_RESET);
        mask = (bitOrder == LSBFIRST) ? (1<<i) : (1 << ((BYTE_SIZE-1)-i));
        HAL_GPIO_WritePin(dataPin.port, dataPin.pin, (GPIO_PinState) (val & mask));
        HAL_GPIO_WritePin(clockPin.port, clockPin.pin, GPIO_PIN_SET);
    }
}

/*!
    @brief Shift in a byte of data one bit at a time
    @param dataPin GPIO pin configuration structure for data pin
    @param clockPin GPIO pin configuration structure for clock pin
    @param bitOrder Bit order (LSBFIRST or MSBFIRST)
    @return Byte value shifted in
*/
uint8_t stm_shiftIn(TM_GPIO_PinConfig_t dataPin, TM_GPIO_PinConfig_t clockPin, TM_Bit_Order bitOrder)
{
    uint8_t data = 0;
    uint8_t pinState = 0;
    for(int i = 0; i < BYTE_SIZE; i++)
    {
        HAL_GPIO_WritePin(clockPin.port, clockPin.pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(clockPin.port, clockPin.pin, GPIO_PIN_SET);
        pinState = HAL_GPIO_ReadPin(dataPin.port, dataPin.pin);
        pinState = (bitOrder == LSBFIRST) ? (pinState << i) : (pinState << ((BYTE_SIZE-1)-i));
        data |= pinState;
    }
    return data;
}