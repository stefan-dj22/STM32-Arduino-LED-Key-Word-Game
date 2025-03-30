#ifndef __COMMAND_UTILS_H
#define __COMMAND_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Command types that can be detected
 */
typedef enum {
    CMD_NONE = 0,      // No command detected
    CMD_NEXT,          // 3 x SP(button8) - Next item
    CMD_PREV,          // 3 x SP(button1) - Previous item
    CMD_VIEW,          // n x SP(different button) - View current item
    CMD_SET,           // n x SP(different button) + LP(new button) - Set value
    CMD_INVALID,        // Invalid command sequence
    CMD_CNT
} CommandType_t;

/**
 * @brief Command structure for tracking command sequences
 */
typedef struct {
    uint8_t button_pressed;      // Currently pressed button
    uint8_t button_clicked;      // Bit mask of clicked buttons in sequence
    bool sequence_complete;      // Flag indicating sequence is complete
    uint8_t click_count;         // Count of short presses for current button
    uint32_t last_press_time;    // Time of last button press
    CommandType_t current_cmd;   // Current command being built
} Command_t;

/**
 * @brief Initialize a command structure
 * @param cmd Pointer to the command structure to initialize
 */
void Command_Init(Command_t* cmd);

/**
 * @brief Process button states to detect command sequences
 * @param cmd Pointer to the command structure to process
 * @param debaunced_buttons Bit mask of currently pressed buttons
 * @param long_press_detected Flag indicating a long press was detected
 */
void Command_Process(Command_t* cmd, uint8_t debaunced_buttons, bool long_press_detected);

/**
 * @brief Get the current command type
 * @param cmd Pointer to the command structure to check
 * @return Current command type
 */
CommandType_t Command_GetType(Command_t* cmd);

/**
 * @brief Check if command sequence is complete
 * @param cmd Pointer to the command structure to check
 * @return true if sequence is complete, false otherwise
 */
bool Command_IsSequenceComplete(Command_t* cmd);

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_UTILS_H */ 
