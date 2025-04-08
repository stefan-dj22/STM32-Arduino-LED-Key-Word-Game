/**
 * @file DisplayControlUnit.h
 * @brief Header file for the Display Control Unit that manages TM1638 display output
 * @details This module provides a high-level interface for controlling the TM1638 display,
 *          including text display, command feedback, state transition animations, and
 *          consistent display formatting.
 */

#ifndef DISPLAY_CONTROL_UNIT_H
#define DISPLAY_CONTROL_UNIT_H

#include "TM1638plus.h"
#include "command_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum size of the display text buffer */
#define DCU_DISPLAY_BUFFER_SIZE 256

// Forward declarations of C structs and types if needed
// (CommandType_t is already declared in command_utils.h)

#ifdef __cplusplus
}
#endif

/**
 * @brief Display Control Unit for managing display output
 * 
 * This class provides a high-level interface for managing the TM1638 display module.
 * It handles:
 * - Text display with optional sliding animation
 * - Display buffer management
 * - Display state control (on/off)
 * - Display clearing and reset
 */
class DisplayControlUnit {
public:
    /**
     * @brief Construct a new Display Control Unit
     * 
     * @param tm Pointer to initialized TM1638plus display object
     * @note The TM1638plus object must be initialized before being passed to constructor
     */
    DisplayControlUnit(TM1638plus* tm);

    /**
     * @brief Update the display state
     * 
     * This method should be called periodically to:
     * - Update sliding text animation if enabled
     * - Initialize new display content
     * - Maintain display state
     */
    void displayUpdate();

    /**
     * @brief Prepare text for display
     * 
     * @param text The text to be displayed
     * @param slide_enabled Enable sliding animation for text longer than display width
     * @note Text will be truncated if longer than DCU_DISPLAY_BUFFER_SIZE
     */
    void displayPrepare(const char* text, bool slide_enabled);

    /**
     * @brief Turn off the display
     * 
     * Disables the display and clears all segments
     */
    void turnOffDisplay();

    /**
     * @brief Clear the display
     * 
     * Clears all segments but keeps the display enabled
     */
    void clear();

private:
    TM1638plus* m_tm;              /**< Pointer to TM1638 display module */

    bool m_sliding_enabled;         /**< Flag indicating if text sliding is enabled */
    bool m_display_enabled;         /**< Flag indicating if display is enabled */
    bool m_init_display;           /**< Flag indicating if display needs initialization */
    uint8_t m_display_text_length; /**< Length of current display text */
    char m_display_buffer[DCU_DISPLAY_BUFFER_SIZE]; /**< Buffer for display text */
};

#endif // DISPLAY_CONTROL_UNIT_H