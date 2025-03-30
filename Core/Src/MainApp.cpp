/*
 * MainApp.cpp
 *
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

// Array of buttons for debouncing
#define NUM_BUTTONS 8

/**
 * @brief Application states
 */
typedef enum {
    STATE_READING = 0,
    STATE_SET_DISPLAY_VIEW,
    STATE_SET_DISPLAY_SET,
    STATE_DISPLAY,
    STATE_ERROR,
    STATE_CNT
} AppState_t;

// Display text for each state
const char* stateText[] = {
    "READING ",   // Reading
    "VIEW   ",    // Set_Display_View
    "SET    ",    // Set_Display_Set
    "DISPLAY",    // Display
    "ERROR  "     // Error
};


// Command display strings
#define CMD_NEXT_STR "NEXT"
#define CMD_PREV_STR "PREV"
#define CMD_VIEW_STR "VIEW"
#define CMD_SET_STR "SET"
#define CMD_ERROR_STR "ERROR"

// State display strings
#define STATE_READING_STR "Reading"
#define STATE_SET_DISPLAY_VIEW_STR "Set Display"
#define STATE_SET_DISPLAY_SET_STR "Set Value"
#define STATE_DISPLAY_STR "Display"

#define CMD_TIME_NOWAIT 0
#define CMD_TIME_ERROR 2000
#define CMD_TIME_
AppState_t appStateTransitionTable[STATE_CNT][CMD_CNT];



bool input_enabled = true;
uint32_t cmd_timer_start = 0;
uint32_t cmd_timer_duration = 0;


// Function to get display text for current state
const char* getStateDisplayText(AppState_t state) {
    return stateText[state];
}

// Function to get button state from raw button byte
static uint8_t getButtonState(uint8_t buttonByte, uint8_t buttonIndex) {
    return (buttonByte & (1 << buttonIndex)) ? 1 : 0;
}
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

static void app_init_interaction(Button_t* buttons, Command_t* command) {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        Button_Init(&buttons[i]);
    }
    Command_Init(command);
}

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
static void app_init_state_transition_table() {
    
    appStateTransitionTable[STATE_READING][CMD_NEXT] = STATE_SET_DISPLAY_VIEW;
    appStateTransitionTable[STATE_READING][CMD_PREV] = STATE_DISPLAY;
    appStateTransitionTable[STATE_READING][CMD_VIEW] = STATE_READING;
    appStateTransitionTable[STATE_READING][CMD_SET] = STATE_ERROR;
    appStateTransitionTable[STATE_READING][CMD_NONE] = STATE_READING;
    appStateTransitionTable[STATE_READING][CMD_INVALID] = STATE_ERROR;

    appStateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_NEXT] = STATE_DISPLAY;
    appStateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_PREV] = STATE_READING;
    appStateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_VIEW] = STATE_SET_DISPLAY_VIEW;
    appStateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_SET] = STATE_SET_DISPLAY_SET;
    appStateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_NONE] = STATE_SET_DISPLAY_VIEW;
    appStateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_INVALID] = STATE_ERROR;

    appStateTransitionTable[STATE_SET_DISPLAY_SET][CMD_NEXT] = STATE_ERROR;
    appStateTransitionTable[STATE_SET_DISPLAY_SET][CMD_PREV] = STATE_ERROR;
    appStateTransitionTable[STATE_SET_DISPLAY_SET][CMD_VIEW] = STATE_SET_DISPLAY_VIEW;
    appStateTransitionTable[STATE_SET_DISPLAY_SET][CMD_SET] = STATE_ERROR;
    appStateTransitionTable[STATE_SET_DISPLAY_SET][CMD_NONE] = STATE_SET_DISPLAY_SET;
    appStateTransitionTable[STATE_SET_DISPLAY_SET][CMD_INVALID] = STATE_ERROR;

    appStateTransitionTable[STATE_DISPLAY][CMD_NEXT] = STATE_READING;
    appStateTransitionTable[STATE_DISPLAY][CMD_PREV] = STATE_SET_DISPLAY_VIEW;
    appStateTransitionTable[STATE_DISPLAY][CMD_VIEW] = STATE_ERROR;
    appStateTransitionTable[STATE_DISPLAY][CMD_SET] = STATE_ERROR;
    appStateTransitionTable[STATE_DISPLAY][CMD_NONE] = STATE_DISPLAY;
    appStateTransitionTable[STATE_DISPLAY][CMD_INVALID] = STATE_ERROR;
    
    
}
#define TIMEOUT_ERROR 2000u
#define TIMEOUT_NONE 0
uint8_t command_timeout = TIMEOUT_NONE;
uint32_t command_start_time = 0;
static void app_update_state(AppState_t* currentState, AppState_t* defaultState, Command_t* command) {
	*defaultState = (*currentState != STATE_SET_DISPLAY_SET) ? (*currentState) : (STATE_SET_DISPLAY_VIEW);
	*currentState = appStateTransitionTable[*currentState][command->current_cmd];
}

static void app_update_display(Command_t* command, AppState_t* currentState, TM1638plus* tm) {
    tm->displayText(getStateDisplayText(*currentState), TMAlignTextLeft);
}
uint32_t leds_interval_start_time = 0;
#define LEDS_INTERVAL_TIME 30
uint32_t leds_interval_time = LEDS_INTERVAL_TIME;   

void turnLeds(TM1638plus* tm, bool on, uint16_t led_mask){
    if(on)
    {
        tm->setLEDs(led_mask);
    }
    else
    {
        tm->setLEDs(0x00);
    }
}

bool leds_on = false;
#define ALL_LEDS_MASK 0xff00
static void app_update_leds(Command_t* command, AppState_t* currentState, TM1638plus* tm) {
    if(*currentState == STATE_ERROR)
    {
        if(leds_interval_start_time - TimerUtils_GetTick() > leds_interval_time || leds_interval_start_time == 0)
        {
            leds_interval_start_time = TimerUtils_GetTick();
            turnLeds(tm, leds_on, ALL_LEDS_MASK);
            leds_on = !leds_on;
        }
    }//ekse if(currnetstate == STATE_SET_DISPLAY_SET) { set leds similar to error state }
    else if (Command_IsSequenceComplete(command) && command->current_cmd == CMD_NONE)
    {
        turnLeds(tm, false, ALL_LEDS_MASK);
        leds_on = false;
        leds_interval_start_time = 0;
    }


}   
uint16_t leds_to_turn_on = 0;
void app_proccess_command_state(Command_t* command, AppState_t* currentState, AppState_t* defaultState)
{
    switch(*currentState)
    {
        case STATE_ERROR:
        {
            if(input_enabled)
            {
            input_enabled = false;
            command_timeout = TIMEOUT_ERROR;
            command_start_time = TimerUtils_GetTick();
            }
            else if(TimerUtils_GetTick() - command_start_time > command_timeout)
            {
                input_enabled = true;
                command_timeout = TIMEOUT_NONE;
                Command_Init(command);
                command_start_time = TimerUtils_GetTick();
                leds_to_turn_on=0;
                *currentState = *defaultState;
            }
            break;
        }
        default:
        {
            if(Command_IsSequenceComplete(command) && ((TimerUtils_GetTick() - command_start_time) >= command_timeout))
            {
                command_timeout = TIMEOUT_NONE;
                Command_Init(command);
                command_start_time = TimerUtils_GetTick();
                leds_to_turn_on=0;
            }
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

    uint8_t pressed_buttons = 0;
    bool long_press_detected = false;

    if (!app_init_tm1638(tm, tm_common)) {
        // Handle initialization failure
        return;
    }
    
    app_init_interaction(buttons, &command);
    app_init_state_transition_table();
    TimerUtils_Init();
    
    tm->displayText(getStateDisplayText(currentState), TMAlignTextLeft);
    while(1) {

        if(input_enabled)
        {
            app_process_buttons(&pressed_buttons, &long_press_detected, tm, buttons);
            leds_to_turn_on |= pressed_buttons<<8;
            tm->setLEDs(leds_to_turn_on);
            
            Command_Process(&command, pressed_buttons, long_press_detected);
            if(Command_IsSequenceComplete(&command))
            {
            	command_start_time = TimerUtils_GetTick();
                app_update_state(&currentState, &defaultState, &command);
            }
          
        }
        app_proccess_command_state(&command, &currentState, &defaultState);
        app_update_leds(&command, &currentState, tm);
        app_update_display(&command, &currentState, tm);

        HAL_Delay(10);
    }


}

















    // // Create TM1638 object with GPIO pin configurations
    // static TM_GPIO_PinConfig_t strobe_pin = {STB_GPIO_Port, STB_Pin};  // STB
    // static TM_GPIO_PinConfig_t clock_pin = {CLK_GPIO_Port, CLK_Pin};   // CLK
    // static TM_GPIO_PinConfig_t data_pin = {DIO_GPIO_Port, DIO_Pin};    // DIO
    // static TM1638plus tm(strobe_pin, clock_pin, data_pin);

    // // Initialize TM1638
    // tm.displayBegin();

    // // Initialize all buttons and command
    // for(int i = 0; i < NUM_BUTTONS; i++) {
    //     Button_Init(&buttons[i]);
    // }
    // Command_Init(&command);

    // // Initialize timer utilities
    // TimerUtils_Init();

    // // Initialize state
    // currentState = STATE_DISPLAY;
    // previousState = STATE_DISPLAY;
    // tm.displayText(getStateDisplayText(currentState), TMAlignTextLeft);

    // uint8_t raw_buttons;
    // uint8_t debounced_buttons = 0;
    // uint16_t buttons_led = 0;
    // bool long_press_detected = false;
    // uint32_t errorDisplayTime = 0;
    // bool inErrorState = false;

    // // Main loop
    // while(1) {
    //     // Read raw button states
    //     raw_buttons = tm.readButtons();
    //     long_press_detected = false;

    //     // Update each button's debounced state and check for commands
    //     for(int i = 0; i < NUM_BUTTONS; i++) {
    //         Button_Process(&buttons[i], getButtonState(raw_buttons, i));

    //         // Update debounced button states
    //         if(Button_IsPressed(&buttons[i])) {
    //             debounced_buttons |= (1 << i);
    //         } else {
    //             debounced_buttons &= ~(1 << i);
    //         }

    //         // Check for long press
    //         if(Button_IsLongPress(&buttons[i])) {
    //             long_press_detected = true;
    //         }
    //     }

    //     // Process command based on button states
    //     Command_Process(&command, debounced_buttons, long_press_detected);

    //     // Check if command sequence is complete
    //     if(Command_IsSequenceComplete(&command)) {
    //         CommandType_t cmd_type = Command_GetType(&command);

    //         // // Display command text
    //         // switch(cmd_type) {
    //         //     case CMD_NEXT:
    //         //         tm.displayText("NEXT", TMAlignTextLeft);
    //         //         break;
    //         //     case CMD_PREV:
    //         //         tm.displayText("PREV", TMAlignTextLeft);
    //         //         break;
    //         //     case CMD_VIEW:
    //         //         tm.displayText("VIEW", TMAlignTextLeft);
    //         //         break;
    //         //     case CMD_SET:
    //         //         tm.displayText("SET", TMAlignTextLeft);
    //         //         break;
    //         //     default:
    //         //         tm.displayText("ERROR", TMAlignTextLeft);
    //         //         break;
    //         // }

    //         // Update state based on command
    //         switch(currentState)
    //         {
    //             case STATE_READING:
	// 			switch(cmd_type)
	// 			{
	// 				case CMD_NEXT:
	// 					currentState = STATE_SET_DISPLAY_VIEW;
	// 					break;
	// 				case CMD_PREV:
	// 					currentState = STATE_DISPLAY;
	// 					break;
	// 				default:
	// 					currentState = STATE_ERROR;
	// 					break;
    //             }
    //             case STATE_SET_DISPLAY_VIEW:
	// 			switch(cmd_type)
	// 			{
	// 				case CMD_NEXT:
	// 					currentState = STATE_DISPLAY;
	// 					break;
	// 				case CMD_PREV:
	// 					currentState = STATE_READING;
	// 					break;
	// 				case CMD_SET:
	// 					currentState = STATE_SET_DISPLAY_SET;
	// 					break;
	// 				default:
	// 					currentState = STATE_ERROR;
	// 					break;
	// 			}
    //                 break;
    //             case STATE_SET_DISPLAY_SET:
    //                 break;
	// 			case STATE_DISPLAY:
	// 				break;
	// 			case STATE_ERROR:
	// 				break;
    //         }
    //         switch(cmd_type) {
    //             case CMD_NEXT:
    //                 if(currentState < STATE_ERROR) {
    //                     currentState = (AppState_t)(currentState + 1);
    //                 }
    //                 break;
    //             case CMD_PREV:
    //                 if(currentState > STATE_READING) {
    //                     currentState = (AppState_t)(currentState - 1);
    //                 }
    //                 break;
    //             case CMD_VIEW:
    //                 // Only change state if we're in SET_DISPLAY_VIEW state
    //                 if(currentState == STATE_SET_DISPLAY_VIEW) {
    //                     currentState = STATE_SET_DISPLAY_SET;
    //                 }
    //                 break;
    //             case CMD_SET:
    //                 // Only change state if we're in SET_DISPLAY_SET state
    //                 if(currentState == STATE_SET_DISPLAY_SET) {
    //                     currentState = STATE_SET_DISPLAY_VIEW;
    //                 }
    //                 break;
	// 			case CMD_NONE:
	// 				break;
    //             default:
    //                 // Store previous state and go to error state
    //                 previousState = currentState;
    //                 currentState = STATE_ERROR;
    //                 inErrorState = true;
    //                 errorDisplayTime = HAL_GetTick();
    //                 break;
    //         }

    //         // Display new state text
    //         tm.reset();
    //         tm.displayText(getStateDisplayText(currentState), TMAlignTextLeft);

    //         Command_Init(&command); // Reset command after handling
	// 		buttons_led = 0;
    //         tm.setLEDs(buttons_led); // Turn off all LEDs when command sequence is complete
            
    //     }

    //     // Check if we need to exit error state
    //     if(inErrorState && (HAL_GetTick() - errorDisplayTime >= 2000)) { // 2 second error display
    //         currentState = previousState;
    //         inErrorState = false;
    //         tm.reset();
    //         tm.displayText(getStateDisplayText(currentState), TMAlignTextLeft);
    //     }

    //     // Use debounced button states for LED control
    //     buttons_led |= (uint16_t)(debounced_buttons << 8);
    //     tm.setLEDs(buttons_led);

    //     // Small delay to prevent too frequent updates
    //     HAL_Delay(10);
    // }
// }
