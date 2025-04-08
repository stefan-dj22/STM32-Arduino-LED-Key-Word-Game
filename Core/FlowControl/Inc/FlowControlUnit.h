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

typedef AppState_t DefaultStateTabe[STATE_CNT];
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
     */
    FlowControlUnit();

    /**
     * @brief Initialize the flow control unit
     * 
     * Sets up the state transition table and initial state
     */
    void init(LEDControlUnit* lcu, DisplayControlUnit* dcu);

    /**
     * @brief Process new input from the buttons
     * 
     * @param pressed_buttons Bitmask of pressed buttons
     * @param long_press_detected True if a long press was detected
     */
    void processNewInput(uint8_t pressed_buttons, bool long_press_detected, LEDControlUnit* lcu);
    /**
     * @brief Process current command state and handle transitions
     * 
     * @param command Current command structure
     */
    void processCommand(Command_t* command);

    /**
     * @brief Update the output
     * 
     * @param lcu LEDControlUnit pointer
     * @param ldu DisplayControlUnit pointer
     */
    void updateOutput(LEDControlUnit* lcu, DisplayControlUnit* ldu);

    /**
     * @brief Set the transition time
     */
        void setTransitionTime();

    /**
     * @brief Update the state
     */
    void updateState();
    

    void Update(LEDControlUnit* lcu, DisplayControlUnit* dcu, bool* out_input_enabled);
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
    DefaultStateTabe m_defaultStateTable;   
    // Current and default states
    AppState_t m_currentState;
    AppState_t m_defaultState;

    // Active command
    Command_t m_active_command;
    
    // Timing control
    uint32_t m_transitionTime;
    uint32_t m_transitionTimeStart;
    bool m_shouldUpdateOutput;

#define FCU_WORD_MAX_CNT 0xFFu
#define FCU_WORD_CUR_CNT 32
#define FCU_DISPLAY_WORD_CNT 16
#define FCU_DISPLAY_WORD_SIZE 16
    //Word arrays
    char m_word_array[FCU_WORD_CUR_CNT][FCU_DISPLAY_WORD_SIZE]={"jedan","dva","tri","cetri"};
    char m_display_array[FCU_DISPLAY_WORD_CNT][FCU_DISPLAY_WORD_SIZE];

    uint8_t m_cmd_view_index;
    uint8_t m_cmd_set_index;
    uint8_t m_cmd_last_set_index;
    // Display text for each state
    static const char* STATE_TEXT[];

    /**
     * @brief Initialize the state transition table
     */
    void initStateTransitionTable();
    
    /**
     * @brief Initialize the default state table
     */
    void initDefaultStateTable();

    void concatenateDisplayWords(char* output);
    /**
     * @brief Handle error state and timeouts
     * 
     * @param command Current command structure
     */
    void handleErrorState(Command_t* command);

    void loadPredefinedWords();
    
    uint8_t getArrIndexFromCmd();
};

#endif // FLOW_CONTROL_UNIT_H 
