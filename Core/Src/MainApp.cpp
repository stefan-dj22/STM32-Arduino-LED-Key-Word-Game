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
#include "timer_utils.h"
#include "button_utils.h"
#include "command_utils.h"
#include "debug_log.h"
// Array of buttons for debouncing
#define NUM_BUTTONS 8
static Button_t buttons[NUM_BUTTONS];
static Command_t command;

// Command display strings
#define CMD_NEXT_STR "NEXT"
#define CMD_PREV_STR "PREV"
#define CMD_VIEW_STR "VIEW"
#define CMD_SET_STR "SET"
#define CMD_ERROR_STR "ERROR"

// Function to extract individual button states from button byte
static uint8_t getButtonState(uint8_t buttonByte, uint8_t buttonIndex) {
    return (buttonByte & (1 << buttonIndex)) ? 1 : 0;
}

void Test0(TM1638plus tm)
{
  tm.setLED(0, 1);
  HAL_Delay(500);
  tm.reset();
}
#define show_str "Hello world"
#define short_press_str "SP"
#define long_press_str "LP"
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
	// Initialize timer utilities
	TimerUtils_Init();
	
	// Initialize all buttons and command
	for(int i = 0; i < NUM_BUTTONS; i++) {
	    Button_Init(&buttons[i]);
	}
	Command_Init(&command);

	TM_GPIO_PinConfig_t STB_gpio = {.port= STB_GPIO_Port, .pin= STB_Pin};
	TM_GPIO_PinConfig_t CLK_gpio = {.port= CLK_GPIO_Port, .pin= CLK_Pin};
	TM_GPIO_PinConfig_t DIO_gpio = {.port= DIO_GPIO_Port, .pin= DIO_Pin};
	TM1638plus_common tm(STB_gpio, CLK_gpio, DIO_gpio);
	TM1638plus tm_plus(STB_gpio, CLK_gpio, DIO_gpio);

	tm.displayBegin();
	tm_plus.displayText("Ready", TMAlignTextLeft);
	
	uint8_t raw_buttons;
	uint8_t debounced_buttons = 0;
	int16_t buttons_led;
	bool long_press_detected = false;
	while(1)
	{
		// Read raw button states
		raw_buttons = tm_plus.readButtons();
		long_press_detected = false;
		// Update each button's debounced state and check for commands
		for(int i = 0; i < NUM_BUTTONS; i++) {

	//		DBG_DEBUG(DBG_CAT_BUTTON, "Button[%d] being proccesed", i);
		    Button_Process(&buttons[i], getButtonState(raw_buttons, i));
		    
		    // Update the debounced button byte

		    if(Button_IsPressed(&buttons[i])){
		        debounced_buttons |= (1 << i);
		        long_press_detected |= Button_IsLongPress(&buttons[i]);
		        // Process command for this button
		    } else {
		        debounced_buttons &= ~(1 << i);
		    }
		}
		
		Command_Process(&command, debounced_buttons, long_press_detected);
		// Handle commands
		if(Command_IsSequenceComplete(&command)) {
		    switch(Command_GetType(&command)) {
		        case CMD_NEXT:
					tm.reset();
		            tm_plus.displayText(CMD_NEXT_STR, TMAlignTextLeft);
		            break;
		        case CMD_PREV:
				tm.reset();
		            tm_plus.displayText(CMD_PREV_STR, TMAlignTextLeft);
		            break;
		        case CMD_VIEW:
		        	tm.reset();
				    tm_plus.displayText(CMD_VIEW_STR, TMAlignTextLeft);
		            break;
		        case CMD_SET:
		        	tm.reset();
		            tm_plus.displayText(CMD_SET_STR, TMAlignTextLeft);
		            break;
		        default:
		        	tm.reset();
		            tm_plus.displayText(CMD_ERROR_STR, TMAlignTextLeft);
		            break;
		    }
		    Command_Init(&command); // Reset command after handling
		}
		
		// Use debounced button states for LED control
		buttons_led = (uint16_t)(debounced_buttons << 8);
		tm_plus.setLEDs(buttons_led);
		
		// Small delay to prevent too frequent updates
		HAL_Delay(1);
	}
}
