/**
 * @file FlowControlUnit.cpp
 * @brief Implementation of the Flow Control Unit for managing application state and transitions
 * @details Handles state transitions, command processing, and coordination between
 *          display and LED control units.
 */

#include "FlowControlUnit.h"
#include "string.h"
// State text definitions
const char* FlowControlUnit::STATE_TEXT[] = {
    "READING ",   // Reading
    "VIEW   ",    // Set_Display_View
    "SET    ",    // Set_Display_Set
    "DISPLAY",    // Display
    "ERROR  "     // Error
};

// Timeout definitions
#define FCU_TIMEOUT_STATE_ERROR 2000u
#define FCU_TIMEOUT_CMD_VIEW 1000u
#define FCU_TIMEOUT_NONE 0u

/**
 * @brief Constructor for FlowControlUnit
 * 
 * Initializes the flow control unit with default values:
 * - No transition time configured
 * - Command indexes reset to 0
 */
FlowControlUnit::FlowControlUnit()
    : m_transitionTime(0)
    , m_transitionTimeStart(FCU_TIMEOUT_NONE)
	, m_cmd_view_index(0)
	, m_cmd_set_index(0)
	, m_cmd_last_set_index(0)
{
    // Constructor implementation
}

/**
 * @brief Initialize the flow control unit
 * 
 * Sets up:
 * - State transition table
 * - Default state table
 * - Initial application state
 * - Word arrays and display buffers
 * 
 * @param lcu Pointer to LED control unit
 * @param dcu Pointer to display control unit
 */
void FlowControlUnit::init(LEDControlUnit* lcu, DisplayControlUnit* dcu) {
    // Initialize state transition table
    initStateTransitionTable();
    // Initialize default state table
    initDefaultStateTable();
    // Setup initial state
    m_currentState = STATE_READING;
    m_defaultState = m_defaultStateTable[m_currentState];
    
    //init arrays to empty
    int i = 0;
    while(i < FCU_WORD_CUR_CNT || i < FCU_DISPLAY_WORD_CNT)
    {
        if(i < FCU_DISPLAY_WORD_CNT)
        {
            m_display_array[i][0] = '\0';
        }
        if(i < FCU_WORD_CUR_CNT)
        {
            m_word_array[i][0] = '\0';
        }
        i++;
    }
    loadPredefinedWords();
    // Display initial state
    dcu->displayPrepare(getStateText(), false);
}

/**
 * @brief Load predefined words into word array
 * 
 * Initializes the word array with preset values for testing and demonstration
 */
void FlowControlUnit::loadPredefinedWords() {
    static const char* words[] = {
        "SMILE", "HAPPY", "DANCE", "LAUGH", "DREAM",
        "SHINE", "SPARK", "GLOW", "JUMP", "PLAY"
    };
    for (uint8_t i = 0; i < FCU_DISPLAY_WORD_CNT && i < sizeof(words)/sizeof(words[0]); i++) {
        strcpy(m_word_array[i], words[i]);
    }
}

/**
 * @brief Initialize the state transition table
 * 
 * Sets up transitions between states for different commands:
 * - Default all transitions to ERROR state
 * - Define valid transitions for each state/command combination
 */
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

/**
 * @brief Initialize the default state table
 * 
 * Sets up default states to return to after error recovery or timeouts
 */
void FlowControlUnit::initDefaultStateTable() {
    m_defaultStateTable[STATE_READING] = STATE_READING;
    m_defaultStateTable[STATE_SET_DISPLAY_VIEW] = STATE_SET_DISPLAY_VIEW;
    m_defaultStateTable[STATE_SET_DISPLAY_SET] = STATE_SET_DISPLAY_VIEW;
    m_defaultStateTable[STATE_DISPLAY] = STATE_DISPLAY;
    m_defaultStateTable[STATE_ERROR] = STATE_ERROR;
}

/**
 * @brief Process new button input 
 * 
 * Handles button presses and updates LED indicators:
 * - Processes button input through Command_Process
 * - Updates LED state to reflect button presses
 * - Initiates state transitions for completed commands
 * 
 * @param pressed_buttons Bitmap of currently pressed buttons
 * @param long_press_detected Flag indicating if a long press was detected
 * @param lcu Pointer to LED control unit for visual feedback
 */
void FlowControlUnit::processNewInput(uint8_t pressed_buttons, bool long_press_detected, LEDControlUnit* lcu)
{
    Command_Process(&m_active_command, pressed_buttons, long_press_detected);
    //Update output for new input
    if(pressed_buttons && m_active_command.button_clicked==0)
    {//need to refactor - input process is shufled with output update
    	lcu->clear();
    	lcu->setLedsOn(true);
    }
    lcu->addActiveLeds(pressed_buttons);

    //new command preparation
    if (Command_IsSequenceComplete(&m_active_command)) //new command started
    {
        setTransitionTime();
        if(m_transitionTime != 0)
        {
        	m_shouldUpdateOutput = true;
        }
    }
}

/**
 * @brief Update the current application state
 * 
 * Applies state transitions based on current command:
 * - Uses transition table to determine next state
 * - Updates default state for error recovery
 * - Handles returning from error state
 */
void FlowControlUnit::updateState()
{
    m_currentState = m_stateTransitionTable[m_currentState][m_active_command.current_cmd];
    if (m_currentState != STATE_ERROR)
    {
    	m_defaultState = m_defaultStateTable[m_currentState];
    }
    else if(m_currentState == STATE_ERROR && m_active_command.current_cmd == CMD_NONE)
    {
        m_currentState = m_defaultState;
    }
}

/**
 * @brief Set transition time based on current state and command
 * 
 * Configures timeout duration:
 * - Error state uses error timeout
 * - VIEW command has its own timeout
 * - Other states/commands have no timeout
 */
void FlowControlUnit::setTransitionTime()
{
    if(m_currentState == STATE_ERROR)
    {
        m_transitionTime = FCU_TIMEOUT_STATE_ERROR;
    }
    else if(m_active_command.current_cmd == CMD_VIEW)
    {
        m_transitionTime = FCU_TIMEOUT_CMD_VIEW;
    }
    else
    {
        m_transitionTime = FCU_TIMEOUT_NONE;
    }
    m_transitionTimeStart = TimerUtils_GetTick();
}

/**
 * @brief Convert button press bitmap to array index
 * 
 * Maps button presses to word array indexes by:
 * - Reversing the bit order of the button bitmap
 * - Adjusting to zero-based index
 * 
 * @return Array index derived from button bitmap
 */
uint8_t FlowControlUnit::getArrIndexFromCmd()
{
    uint8_t x = m_active_command.button_clicked;
    // Reverse the bits in a byte using bit manipulation
    x = ((x * 0x0802LU & 0x22110LU) | (x * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
    return x-1;
}

/**
 * @brief Main update function for flow control
 * 
 * Coordinates state transitions, timeouts, and display updates:
 * - Handles transition timing and timeout processing
 * - Updates state when commands are completed
 * - Processes SET and VIEW commands
 * - Updates output devices when needed
 * 
 * @param lcu Pointer to LED control unit
 * @param dcu Pointer to display control unit
 * @param out_input_enabled Pointer to flag that controls input processing
 */
void FlowControlUnit::Update(LEDControlUnit* lcu, DisplayControlUnit* dcu, bool* out_input_enabled) {
    
    if(m_transitionTime != FCU_TIMEOUT_NONE)
    {
        if(*out_input_enabled == true)
        {
            *out_input_enabled = false;
        }
        if(TimerUtils_GetTick() - m_transitionTimeStart >= m_transitionTime)
        {//command is over
            updateState();
            setTransitionTime();
            m_shouldUpdateOutput = true;
            Command_Init(&m_active_command);
            *out_input_enabled = true;
        }
    }
    else if(Command_IsSequenceComplete(&m_active_command))
    {//command is over
    	if(m_active_command.current_cmd == CMD_SET && m_currentState == STATE_SET_DISPLAY_VIEW)//todo: state update happens before command is over
    	{																						//and that cousing incosistance
    		m_cmd_set_index = getArrIndexFromCmd();
    		m_cmd_last_set_index = (m_cmd_last_set_index < m_cmd_set_index) ? m_cmd_set_index : m_cmd_last_set_index;
    	}
        updateState();
        setTransitionTime();
        m_shouldUpdateOutput = true;
    	Command_Init(&m_active_command);
    }

    //
    //should update output when: 
        //1. new command started
        //2. transition time ended
    if(m_shouldUpdateOutput)
    {
    	if(m_active_command.current_cmd == CMD_VIEW && m_currentState != STATE_DISPLAY)
    	{
    		m_cmd_view_index = getArrIndexFromCmd();
    		if(m_currentState == STATE_SET_DISPLAY_SET)
    		{
    			strcpy(m_display_array[m_cmd_set_index],m_word_array[m_cmd_view_index]);
    		}
    	}

        updateOutput(lcu, dcu);
        m_shouldUpdateOutput = false;
    }
}

/**
 * @brief Concatenate words from display array into a single string
 * 
 * Creates a space-separated string of all words in the display array:
 * - Includes all words up to the last set index
 * - Adds spaces between words
 * 
 * @param output Buffer to store the concatenated string
 */
void FlowControlUnit::concatenateDisplayWords(char* output) {
    if (m_cmd_last_set_index >= FCU_DISPLAY_WORD_CNT) return;

    // Start with empty string
    output[0] = '\0';

    for (uint8_t i = 0; i <= m_cmd_last_set_index; i++)
    {
    	// Append current word
    	strcat(output, m_display_array[i]);

    	strcat(output, " ");
    }
}
#define DISPLAY_STRING_SIZE (FCU_DISPLAY_WORD_CNT*FCU_DISPLAY_WORD_SIZE) + FCU_DISPLAY_WORD_CNT

/**
 * @brief Update output devices based on current state
 * 
 * Updates display and LED indicators based on current state:
 * - Updates display content according to state
 * - Configures LED patterns and blinking behavior
 * - Handles special cases for different states
 * 
 * @param lcu Pointer to LED control unit
 * @param ldu Pointer to display control unit
 */
void FlowControlUnit::updateOutput(LEDControlUnit* lcu, DisplayControlUnit* dcu)
{
    switch(m_currentState)
    {
        case STATE_READING: 
            switch(m_active_command.current_cmd)
            {
                case CMD_VIEW:
                    //set display
                    dcu->displayPrepare(m_word_array[m_cmd_view_index], false);
                    break;
                default:
                    dcu->displayPrepare(getStateText(), false);
                    lcu->clear();
                    lcu->setLedsOn(true);
                break;
            }
            break;
        case STATE_SET_DISPLAY_VIEW:
            switch(m_active_command.current_cmd)
            {
                case CMD_VIEW:
                    //set display
                	dcu->displayPrepare(m_display_array[m_cmd_view_index], false);
                    break;
                default:
                    dcu->displayPrepare(getStateText(), false);
                    lcu->clear();
                    lcu->setLedsOn(true);
                break;
            }
            break;
        case STATE_SET_DISPLAY_SET:
            switch(m_active_command.current_cmd)
            {
                case CMD_VIEW:
                    //set display
                	dcu->displayPrepare(m_word_array[m_cmd_view_index], false);
                    break;
                default:
                    //display empty
                    lcu->setBlinking(true, TimerUtils_GetTick(), LCU_DISPLAY_SET_BLINK_INTERVAL);
                    dcu->displayPrepare(getStateText(), false);
                    break;
            }
            break;
        case STATE_DISPLAY:
            lcu->clear();
            lcu->setLedsOn(true);
            char displayString[DISPLAY_STRING_SIZE];
            concatenateDisplayWords(displayString);
            dcu->displayPrepare(displayString, true);
            //get all words from array and put it in string
            //put string as parameter to ldu->displayPrepare(getStateText(), true);
            break;  
        default:
            lcu->clear();
            lcu->setActiveLeds(LCU_LED_ALL_MASK);
            lcu->setLedsOn(true);
            lcu->setBlinking(true, TimerUtils_GetTick(), LCU_ERROR_BLINK_INTERVAL);
            dcu->displayPrepare(getStateText(), false);
            break;
    }   
}
AppState_t FlowControlUnit::getCurrentState() const {
    return m_currentState;
}

const char* FlowControlUnit::getStateText() const {
    return STATE_TEXT[m_currentState];
} 
