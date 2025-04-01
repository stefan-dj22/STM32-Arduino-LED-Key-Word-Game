#include "FlowControlUnit.h"

// State text definitions
const char* FlowControlUnit::STATE_TEXT[] = {
    "READING ",   // Reading
    "VIEW   ",    // Set_Display_View
    "SET    ",    // Set_Display_Set
    "DISPLAY",    // Display
    "ERROR  "     // Error
};

// Timeout definitions
#define TIMEOUT_ERROR 2000u
#define TIMEOUT_NONE 0

FlowControlUnit::FlowControlUnit(DisplayControlUnit& displayUnit, LEDControlUnit& ledUnit)
    : m_displayUnit(displayUnit)
    , m_ledUnit(ledUnit)
    , m_currentState(STATE_DISPLAY)
    , m_defaultState(STATE_DISPLAY)
    , m_commandStartTime(0)
    , m_commandTimeout(TIMEOUT_NONE)
    , m_inputEnabled(true)
{
    // Constructor implementation
}

void FlowControlUnit::init() {
    // Initialize state transition table
    initStateTransitionTable();
    
    // Setup initial state
    m_currentState = STATE_DISPLAY;
    m_defaultState = STATE_DISPLAY;
    
    // Display initial state
    m_displayUnit.displayStateText(getStateText());
}

void FlowControlUnit::initStateTransitionTable() {
    // Initialize all transitions to ERROR state (default)
    for(int state = 0; state < STATE_CNT; state++) {
        for(int cmd = 0; cmd < CMD_CNT; cmd++) {
            m_stateTransitionTable[state][cmd] = STATE_ERROR;
        }
    }
    
    // STATE_READING transitions
    m_stateTransitionTable[STATE_READING][CMD_NEXT] = STATE_SET_DISPLAY_VIEW;
    m_stateTransitionTable[STATE_READING][CMD_PREV] = STATE_DISPLAY;
    m_stateTransitionTable[STATE_READING][CMD_VIEW] = STATE_READING;
    m_stateTransitionTable[STATE_READING][CMD_SET] = STATE_ERROR;
    m_stateTransitionTable[STATE_READING][CMD_NONE] = STATE_READING;
    m_stateTransitionTable[STATE_READING][CMD_INVALID] = STATE_ERROR;

    // STATE_SET_DISPLAY_VIEW transitions
    m_stateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_NEXT] = STATE_DISPLAY;
    m_stateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_PREV] = STATE_READING;
    m_stateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_VIEW] = STATE_SET_DISPLAY_VIEW;
    m_stateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_SET] = STATE_SET_DISPLAY_SET;
    m_stateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_NONE] = STATE_SET_DISPLAY_VIEW;
    m_stateTransitionTable[STATE_SET_DISPLAY_VIEW][CMD_INVALID] = STATE_ERROR;

    // STATE_SET_DISPLAY_SET transitions
    m_stateTransitionTable[STATE_SET_DISPLAY_SET][CMD_NEXT] = STATE_ERROR;
    m_stateTransitionTable[STATE_SET_DISPLAY_SET][CMD_PREV] = STATE_ERROR;
    m_stateTransitionTable[STATE_SET_DISPLAY_SET][CMD_VIEW] = STATE_SET_DISPLAY_VIEW;
    m_stateTransitionTable[STATE_SET_DISPLAY_SET][CMD_SET] = STATE_ERROR;
    m_stateTransitionTable[STATE_SET_DISPLAY_SET][CMD_NONE] = STATE_SET_DISPLAY_SET;
    m_stateTransitionTable[STATE_SET_DISPLAY_SET][CMD_INVALID] = STATE_ERROR;

    // STATE_DISPLAY transitions
    m_stateTransitionTable[STATE_DISPLAY][CMD_NEXT] = STATE_READING;
    m_stateTransitionTable[STATE_DISPLAY][CMD_PREV] = STATE_SET_DISPLAY_VIEW;
    m_stateTransitionTable[STATE_DISPLAY][CMD_VIEW] = STATE_ERROR;
    m_stateTransitionTable[STATE_DISPLAY][CMD_SET] = STATE_ERROR;
    m_stateTransitionTable[STATE_DISPLAY][CMD_NONE] = STATE_DISPLAY;
    m_stateTransitionTable[STATE_DISPLAY][CMD_INVALID] = STATE_ERROR;
}

void FlowControlUnit::processCommand(Command_t* command) {
    // Handle different states differently
    if (m_currentState == STATE_ERROR) {
        handleErrorState(command);
    } else {
        // Normal state processing
        if (Command_IsSequenceComplete(command) && 
            (TimerUtils_GetTick() - m_commandStartTime >= m_commandTimeout)) {
            
            m_commandTimeout = TIMEOUT_NONE;
            Command_Init(command);
            m_commandStartTime = TimerUtils_GetTick();
        }
    }
}

void FlowControlUnit::handleErrorState(Command_t* command) {
    if(m_inputEnabled) {
        // Just entered error state
        m_inputEnabled = false;
        m_commandTimeout = TIMEOUT_ERROR;
        m_commandStartTime = TimerUtils_GetTick();
    } else if(TimerUtils_GetTick() - m_commandStartTime > m_commandTimeout) {
        // Error timeout elapsed, return to default state
        m_inputEnabled = true;
        m_commandTimeout = TIMEOUT_NONE;
        Command_Init(command);
        m_commandStartTime = TimerUtils_GetTick();
        m_currentState = m_defaultState;
        
        // Update display and LEDs
        m_displayUnit.displayStateText(getStateText());
        //m_ledUnit.updateStateLEDs(m_currentState);
    }
}

void FlowControlUnit::updateState(Command_t* command) {
    if(!Command_IsSequenceComplete(command)) {
        return;
    }
    
    // Store default state (for returning from error)
    m_defaultState = (m_currentState != STATE_SET_DISPLAY_SET) ? 
                      m_currentState : STATE_SET_DISPLAY_VIEW;
    
    // Get command type
    CommandType_t cmdType = Command_GetType(command);
    
    // Display command text
    m_displayUnit.displayCommandText(command);
    
    // Update state based on transition table
    m_currentState = m_stateTransitionTable[m_currentState][cmdType];
    
    // Start command processing timer
    m_commandStartTime = TimerUtils_GetTick();
    
    // Update display with new state
    m_displayUnit.reset();
    m_displayUnit.displayStateText(getStateText());
    
    // Update LEDs
   // m_ledUnit.updateStateLEDs(m_currentState);
}

AppState_t FlowControlUnit::getCurrentState() const {
    return m_currentState;
}

const char* FlowControlUnit::getStateText() const {
    return STATE_TEXT[m_currentState];
} 
