#ifndef __BUTTON_UTILS_H
#define __BUTTON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Button states in the state machine
 */
typedef enum {
    BUTTON_STATE_RELEASED = 0,  // Button is not pressed
    BUTTON_STATE_PRESSED,       // Button is confirmed pressed
    BUTTON_STATE_DEBOUNCING     // Button is in debounce period
} ButtonState_t;

/**
 * @brief Button structure containing state and timing information
 */
typedef struct {
    ButtonState_t state;        // Current state in the state machine
    uint32_t debounce_start;    // Time when debounce period started
    uint32_t press_start;       // Time when button press started
    bool is_long_press;         // Flag indicating long press
    bool is_clicked;            // Flag indicating button was clicked
} Button_t;

/**
 * @brief Initialize a button structure
 * @param button Pointer to the button structure to initialize
 */
void Button_Init(Button_t* button);

/**
 * @brief Process button state changes and implement debouncing
 * @param button Pointer to the button structure to process
 * @param raw_state Current raw state of the button (0 or 1)
 */
void Button_Process(Button_t* button, uint8_t raw_state);

/**
 * @brief Check if button is currently pressed
 * @param button Pointer to the button structure to check
 * @return true if button is pressed, false otherwise
 */
bool Button_IsPressed(Button_t* button);

/**
 * @brief Check if button is in long press state
 * @param button Pointer to the button structure to check
 * @return true if button is in long press state, false otherwise
 */
bool Button_IsLongPress(Button_t* button);

/**
 * @brief Check if button was clicked (released after valid press)
 * @param button Pointer to the button structure to check
 * @return true if button was clicked, false otherwise
 */
bool Button_IsClicked(Button_t* button);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_UTILS_H */ 