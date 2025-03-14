/*
 * MainApp.cpp
 *
 *  Created on: Mar 5, 2025
 *      Author: joe
 */

#include "MainApp.h"
#include "TM1638plus.h"
#include "pin-info.h"
#include "stdio.h"

void Test0(TM1638plus tm)
{
  tm.setLED(0, 1);
  HAL_Delay(500);
  tm.reset();
}

void Test13(TM1638plus tm)
{
  //Test 13 LED display
  uint8_t LEDposition = 0;

  // Test 13A Turn on redleds one by one, left to right, with setLED where 0 is L1 and 7 is L8 (L8 RHS of display)
  for (LEDposition = 0; LEDposition < 8; LEDposition++) {
    tm.setLED(LEDposition, 1);
    HAL_Delay(500);
    tm.setLED(LEDposition, 0);
  }

  // TEST 13b test setLEDs function (0xLEDXX) ( L8-L1 , XX )
  // NOTE passed L8-L1 and on display L8 is on right hand side. i.e. 0x01 turns on L1. LXXX XXXX
  // For model 1 just use upper byte , lower byte is is used by model3 for bi-color leds leave at 0x00 for model 1.
  tm.setLEDs(0xFF00); //  all LEDs on
  HAL_Delay(500);
  tm.setLEDs(0x0100); // Displays as LXXX XXXX (L1-L8) , NOTE on display L8 is on right hand side.
  HAL_Delay(500);
  tm.setLEDs(0xF000); //  Displays as XXXX LLLL (L1-L8) , NOTE on display L8 is on right hand side.
  HAL_Delay(500);
  tm.setLEDs(0x0000); // all off
  HAL_Delay(500);

}

void MainApp()
{
	TM_GPIO_PinConfig_t STB_gpio = {.port= STB_GPIO_Port, .pin= STB_Pin};
	TM_GPIO_PinConfig_t CLK_gpio = {.port= CLK_GPIO_Port, .pin= CLK_Pin};
	TM_GPIO_PinConfig_t DIO_gpio = {.port= DIO_GPIO_Port, .pin= DIO_Pin};
	TM1638plus_common tm(STB_gpio, CLK_gpio, DIO_gpio);
	TM1638plus tm_plus(STB_gpio, CLK_gpio, DIO_gpio);

	tm.displayBegin();
	//HAL_Delay(500);

	tm_plus.displayText("");
	uint8_t buttons;
	int16_t buttons_led;
	while(1)
	{
		buttons = tm_plus.readButtons();
		buttons_led = buttons << 8;
		tm_plus.setLEDs(buttons_led);
	}

}
