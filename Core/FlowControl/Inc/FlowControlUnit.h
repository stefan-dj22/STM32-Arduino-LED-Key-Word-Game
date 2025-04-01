#ifndef FLOW_CONTROL_UNIT_H
#define FLOW_CONTROL_UNIT_H

#include <stdint.h>
#include "command_utils.h"
#include "timer_utils.h"
#include "TM1638plus.h"
#include "DisplayControlUnit.h"
#include "LEDControlUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

// Application states
typedef enum {
    STATE_READING = 0,
    STATE_SET_DISPLAY_VIEW,
    STATE_SET_DISPLAY_SET,
    STATE_DISPLAY,
    STATE_ERROR,
    STATE_CNT
} AppState_t;

#ifdef __cplusplus
}
#endif

// State transition table type
typedef AppState_t StateTransitionTable[STATE_CNT][CMD_CNT];

/**
 * @brief Flow Control Unit for managing application state and command flow
 * 
 * This class manages:
 * - Application state transitions
 * - Command processing and timing
 * - Error state handling and recovery
 * - Coordination between display and LED units
 */
class FlowControlUnit {
public:
    /**
     * @brief Construct a new Flow Control Unit
     * 
     * @param displayUnit Reference to the display control unit
     * @param ledUnit Reference to the LED control unit
     */
    FlowControlUnit(DisplayControlUnit& displayUnit, LEDControlUnit& ledUnit);

    /**
     * @brief Initialize the flow control unit
     * 
     * Sets up the state transition table and initial state
     */
    void init();

    /**
     * @brief Process current command state and handle transitions
     * 
     * @param command Current command structure
     */
    void processCommand(Command_t* command);

    /**
     * @brief Update the application state based on command
     * 
     * @param command Current command structure
     */
    void updateState(Command_t* command);

    /**
     * @brief Get the current application state
     * 
     * @return Current AppState_t value
     */
    AppState_t getCurrentState() const;

    /**
     * @brief Get the name of the current state
     * 
     * @return const char* Name of current state
     */
    const char* getStateText() const;

private:
    // State transition table
    StateTransitionTable m_stateTransitionTable;
    
    // Current and default states
    AppState_t m_currentState;
    AppState_t m_defaultState;
    
    // Timing control
    uint32_t m_commandStartTime;
    uint32_t m_commandTimeout;
    
    // Control flags
    bool m_inputEnabled;
    
    // References to other control units
    DisplayControlUnit& m_displayUnit;
    LEDControlUnit& m_ledUnit;

    // Display text for each state
    static const char* STATE_TEXT[];

    /**
     * @brief Initialize the state transition table
     */
    void initStateTransitionTable();
    
    /**
     * @brief Handle error state and timeouts
     * 
     * @param command Current command structure
     */
    void handleErrorState(Command_t* command);
};

#endif // FLOW_CONTROL_UNIT_H 