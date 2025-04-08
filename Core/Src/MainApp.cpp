/**
 * @file MainApp.cpp
 * @brief Main application implementation for the LED Key Word Game
 * @details Implements the main application logic, hardware initialization,
 *          and input/output processing for the STM32-Arduino LED Key Word Game.
 *  Created on: Mar 5, 2025
 *      Author: joe
 */

#include "MainApp.h"
#include "TM1638plus.h"
#include "TM1638plus_common.h"
#include "pin-info.h"
#include "stdio.h"
#include "timer_utils.h"
#include "button_utils.h"
#include "command_utils.h"
#include "debug_log.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "main.h"
#include "LEDControlUnit.h"
#include "DisplayControlUnit.h"
#include "FlowControlUnit.h"

// Array of buttons for debouncing
#define NUM_BUTTONS 8

/**
 * @brief Extract state of a specific button from button bitmap
 * 
 * @param buttonByte Bitmap of all button states
 * @param buttonIndex Index of button to check (0-7)
 * @return uint8_t 1 if button is pressed, 0 if not
 */
static uint8_t getButtonState(uint8_t buttonByte, uint8_t buttonIndex) {
    return (buttonByte & (1 << buttonIndex)) ? 1 : 0;
}

/**
 * @brief Initialize the TM1638 display module
 * 
 * Sets up GPIO pins and initializes display hardware:
 * - Configures strobe, clock, and data pins
 * - Creates TM1638plus object
 * - Initializes display
 * 
 * @param tm Reference to TM1638plus pointer to store created object
 * @param tm_common Reference to TM1638plus_common pointer
 * @return true if initialization succeeded, false otherwise
 */
static bool app_init_tm1638(TM1638plus*& tm, TM1638plus_common*& tm_common) {
    TM_GPIO_PinConfig_t strobe_pin = {STB_GPIO_Port, STB_Pin};  // STB
    TM_GPIO_PinConfig_t clock_pin = {CLK_GPIO_Port, CLK_Pin};   // CLK
    TM_GPIO_PinConfig_t data_pin = {DIO_GPIO_Port, DIO_Pin};    // DIO
    
    tm = new TM1638plus(strobe_pin, clock_pin, data_pin);
    if (tm == nullptr) {
        return false;
    }
    
    tm->displayBegin();
    return true;
}

/**
 * @brief Initialize button and command handling
 * 
 * Sets up:
 * - Button debounce structures for all buttons
 * - Command state tracking
 * 
 * @param buttons Array of Button_t structures for each button
 * @param command Pointer to Command_t structure to initialize
 */
static void app_init_interaction(Button_t* buttons, Command_t* command) {
	for(int i = 0; i < NUM_BUTTONS; i++) {
	    Button_Init(&buttons[i]);
	}
    Command_Init(command);
}

/**
 * @brief Process button inputs from TM1638 module
 * 
 * Handles:
 * - Reading raw button states from hardware
 * - Debouncing button inputs
 * - Detecting long presses
 * - Creating a debounced button bitmap
 * 
 * @param out_pressed_buttons Pointer to store the bitmap of pressed buttons
 * @param out_long_press_detected Pointer to store long press detection flag
 * @param tm Pointer to TM1638plus display module
 * @param buttons Array of Button_t structures for each button
 */
static void app_process_buttons(uint8_t* out_pressed_buttons, bool* out_long_press_detected, TM1638plus* tm, Button_t* buttons)
{
    uint8_t raw_buttons = tm->readButtons();
    *out_long_press_detected = false;

		// Update each button's debounced state and check for commands
		for(int i = 0; i < NUM_BUTTONS; i++) {
		    Button_Process(&buttons[i], getButtonState(raw_buttons, i));
		    
        // Update debounced button states
        if(Button_IsPressed(&buttons[i])) {
            *out_pressed_buttons |= (1 << i);
        } else {
            *out_pressed_buttons &= ~(1 << i);
        }

        // Check for long press
        if(Button_IsLongPress(&buttons[i])) {
            *out_long_press_detected = true;
        }
    }
}


void MainApp() {
    TM1638plus* tm = nullptr;
    TM1638plus_common* tm_common = nullptr;
    Button_t buttons[NUM_BUTTONS];
    Command_t command;
    AppState_t currentState = STATE_READING;
    AppState_t defaultState = currentState;

    bool input_enabled = true;
    uint8_t pressed_buttons = 0;
    bool long_press_detected = false;

    if (!app_init_tm1638(tm, tm_common)) {
        // Handle initialization failure
        return;
    }
    LEDControlUnit lcu(tm);
    DisplayControlUnit dcu(tm);
    FlowControlUnit fcu;
    app_init_interaction(buttons, &command);

    TimerUtils_Init();
    fcu.init(&lcu,&dcu);
    fcu.updateOutput(&lcu, &dcu);
    while(1) {
        if(input_enabled)
        {
        	app_process_buttons(&pressed_buttons, &long_press_detected, tm, buttons);
        	fcu.processNewInput(pressed_buttons, long_press_detected,&lcu);
        }
        fcu.Update(&lcu,&dcu,&input_enabled);
        lcu.updateLEDs();
        dcu.displayUpdate();
        HAL_Delay(10);
    }


}