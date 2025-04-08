/**
 * @file DisplayControlUnit.cpp
 * @brief Implementation of the Display Control Unit for TM1638 display management
 * @details Implements display control functionality including text display,
 *          sliding text animations, and display state management.
 */

#include "DisplayControlUnit.h"
#include "string.h"

/**
 * @brief Clear all segments on the display
 * 
 * Resets the display to blank state while maintaining display enabled state.
 * If no display module is connected, this operation is safely ignored.
 */
void DisplayControlUnit::clear() {
    if (m_tm) {
        m_tm->displayClear();
    }
}

/**
 * @brief Constructor for DisplayControlUnit
 * 
 * Initializes the display unit and all member variables
 * 
 * @param tm Pointer to TM1638plus display object
 */
DisplayControlUnit::DisplayControlUnit(TM1638plus* tm)
    : m_tm(tm)
    , m_sliding_enabled(false)
    , m_display_enabled(false)  // Start with display enabled
    , m_init_display(false)
    , m_display_text_length(0)
{
    if (m_tm) {
        m_tm->displayBegin();
        m_tm->displayClear();
    }
    memset(m_display_buffer, 0, DCU_DISPLAY_BUFFER_SIZE);
}

/**
 * @brief Update the display content
 * 
 * This method should be called periodically in the main loop to:
 * - Initialize new display content if needed
 * - Update sliding text animation if enabled
 * - Maintain display state
 * 
 * @note If display is disabled or no display module is connected,
 *       this operation is safely ignored
 */
void DisplayControlUnit::displayUpdate() {
    if (!m_tm || !m_display_enabled) {
        return;
    }

    if(m_init_display) 
    {
    	m_tm->displayClear();
        if(m_sliding_enabled) {
            m_tm->displaySlidingText(m_display_buffer);
        } else {
            m_tm->displayText(m_display_buffer, TMAlignTextLeft);   
        }
        m_init_display=false;
    }
    else if(m_sliding_enabled) {
        m_tm->updateSlidingText();
    }
}

/**
 * @brief Prepare text for display
 * 
 * Safely copies text into display buffer and configures display parameters:
 * - Truncates text if longer than buffer size
 * - Ensures null-termination
 * - Sets sliding animation flag
 * - Marks display for initialization
 * 
 * @param text Text to be displayed
 * @param slide_enabled Enable sliding animation for text
 * @note Operation is safely ignored if no display module is connected
 */
void DisplayControlUnit::displayPrepare(const char* text, bool slide_enabled) {
    if (m_tm && text) {
        // Calculate the actual length of the input text
        m_display_text_length = strlen(text);
        
        // Ensure we don't overflow the buffer
        if (m_display_text_length >= sizeof(m_display_buffer)) {
            m_display_text_length = sizeof(m_display_buffer) - 1;
        }

        memset(m_display_buffer, 0, DCU_DISPLAY_BUFFER_SIZE);
        // Copy the text into the buffer
        memcpy(m_display_buffer, text, m_display_text_length);
        
        // Null-terminate the string
        m_display_buffer[m_display_text_length] = '\0';

        m_sliding_enabled = slide_enabled;
        m_display_enabled = true;
        m_init_display = true;
    }
}

/**
 * @brief Turn off the display
 * 
 * Disables the display and clears all segments.
 * Sets the display_enabled flag to false to prevent updates.
 * 
 * @note Operation is safely ignored if no display module is connected
 */
void DisplayControlUnit::turnOffDisplay() {
    if (m_tm) {
        m_display_enabled = false;
        m_tm->displayClear();
    }
}
