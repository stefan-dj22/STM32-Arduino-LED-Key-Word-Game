#ifndef DISPLAY_CONTROL_UNIT_H
#define DISPLAY_CONTROL_UNIT_H

#include "TM1638plus.h"
#include "command_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations of C structs and types if needed
// (CommandType_t is already declared in command_utils.h)

#ifdef __cplusplus
}
#endif

/**
 * @brief Display Control Unit for managing display output
 * 
 * This class manages:
 * - Text display based on application state
 * - Command text display
 * - State transition display animations
 */
class DisplayControlUnit {
public:
    /**
     * @brief Construct a new Display Control Unit
     * 
     * @param tm Pointer to TM1638plus display object
     */
    DisplayControlUnit(TM1638plus* tm);

    /**
     * @brief Initialize the display
     */
    void init();

    /**
     * @brief Display text for a specific state
     * 
     * @param stateText State text to display
     */
    void displayStateText(const char* stateText);

    /**
     * @brief Display text for a specific command
     * 
     * @param command Pointer to command structure
     */
    void displayCommandText(Command_t* command);

    /**
     * @brief Clear the display
     */
    void clear();

    /**
     * @brief Reset the display
     */
    void reset();

    /**
     * @brief Get command display text
     * 
     * @param command_type Type of command
     * @return const char* Text for command
     */
    const char* getCommandText(CommandType_t command_type) const;

private:
    // Display hardware
    TM1638plus* m_tm;

    // Command display text
    static const char* CMD_TEXT[];
};

#endif // DISPLAY_CONTROL_UNIT_H 