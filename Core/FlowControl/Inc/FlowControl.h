#ifndef FLOW_CONTROL_H
#define FLOW_CONTROL_H

#include "FlowControlUnit.h"
#include "DisplayControlUnit.h"
#include "LEDControlUnit.h"
#include "button_utils.h"
#include "command_utils.h"
#include "timer_utils.h"
#include "TM1638plus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Helper function to extract individual button states from button byte
 * 
 * @param buttonByte Byte containing all button states
 * @param buttonIndex Index of the button to extract (0-7)
 * @return 1 if button is pressed, 0 otherwise
 */
static inline uint8_t getButtonState(uint8_t buttonByte, uint8_t buttonIndex) {
    return (buttonByte & (1 << buttonIndex)) ? 1 : 0;
}

#ifdef __cplusplus
}

/**
 * @brief Example of how to use the FlowControl modules
 * 
 * This example shows how to:
 * - Initialize the control units
 * - Set up button processing
 * - Run the main application loop
 * 
 * @code
 * 
 * // Create TM1638 object (hardware abstraction)
 * TM_GPIO_PinConfig_t strobe_pin = {STB_GPIO_Port, STB_Pin};
 * TM_GPIO_PinConfig_t clock_pin = {CLK_GPIO_Port, CLK_Pin};
 * TM_GPIO_PinConfig_t data_pin = {DIO_GPIO_Port, DIO_Pin};
 * TM1638plus tm(strobe_pin, clock_pin, data_pin);
 * 
 * // Create control units
 * DisplayControlUnit displayUnit(&tm);
 * LEDControlUnit ledUnit(&tm);
 * FlowControlUnit flowUnit(displayUnit, ledUnit);
 * 
 * // Create button and command structures
 * Button_t buttons[NUM_BUTTONS];
 * Command_t command;
 * 
 * // Initialize everything
 * displayUnit.init();
 * ledUnit.init();
 * flowUnit.init();
 * 
 * // Initialize button and command utils
 * for(int i = 0; i < NUM_BUTTONS; i++) {
 *     Button_Init(&buttons[i]);
 * }
 * Command_Init(&command);
 * TimerUtils_Init();
 * 
 * // Main loop
 * while(1) {
 *     // Read hardware buttons
 *     uint8_t raw_buttons = tm.readButtons();
 *     uint8_t pressed_buttons = 0;
 *     bool long_press_detected = false;
 *     
 *     // Process buttons
 *     for(int i = 0; i < NUM_BUTTONS; i++) {
 *         Button_Process(&buttons[i], getButtonState(raw_buttons, i));
 *         
 *         // Update pressed buttons bitmap
 *         if(Button_IsPressed(&buttons[i])) {
 *             pressed_buttons |= (1 << i);
 *         }
 *         
 *         // Check for long press
 *         if(Button_IsLongPress(&buttons[i])) {
 *             long_press_detected = true;
 *         }
 *     }
 *     
 *     // Update LEDs based on button states
 *     ledUnit.updateButtonLEDs(pressed_buttons);
 *     
 *     // Process command based on button states
 *     Command_Process(&command, pressed_buttons, long_press_detected);
 *     
 *     // Update state if command sequence is complete
 *     if(Command_IsSequenceComplete(&command)) {
 *         flowUnit.updateState(&command);
 *     }
 *     
 *     // Process command state (handles timeouts and error states)
 *     flowUnit.processCommand(&command);
 *     
 *     // Small delay to prevent too frequent updates
 *     HAL_Delay(10);
 * }
 * 
 * @endcode
 */
#endif

#endif // FLOW_CONTROL_H 