#include "DisplayControlUnit.h"

// Command text definitions
const char* DisplayControlUnit::CMD_TEXT[] = {
    "       ",  // CMD_NONE
    "NEXT   ",  // CMD_NEXT
    "PREV   ",  // CMD_PREV
    "VIEW   ",  // CMD_VIEW
    "SET    ",  // CMD_SET
    "ERROR  "   // CMD_INVALID
};

DisplayControlUnit::DisplayControlUnit(TM1638plus* tm)
    : m_tm(tm)
{
    // Constructor implementation
}

void DisplayControlUnit::init() {
    // Initialize the display
    if (m_tm) {
        m_tm->displayBegin();
        clear();
    }
}

void DisplayControlUnit::displayStateText(const char* stateText) {
    if (m_tm && stateText) {
        m_tm->displayText(stateText, TMAlignTextLeft);
    }
}

void DisplayControlUnit::displayCommandText(Command_t* command) {
    if (!m_tm || !command) {
        return;
    }
    
    CommandType_t cmdType = Command_GetType(command);
    const char* cmdText = getCommandText(cmdType);
    
    m_tm->displayText(cmdText, TMAlignTextLeft);
}

void DisplayControlUnit::clear() {
    if (m_tm) {
        m_tm->displayText("        ", TMAlignTextLeft);
    }
}

void DisplayControlUnit::reset() {
    if (m_tm) {
        m_tm->reset();
    }
}

const char* DisplayControlUnit::getCommandText(CommandType_t command_type) const {
    if (command_type >= 0 && command_type < CMD_CNT) {
        return CMD_TEXT[command_type];
    }
    return CMD_TEXT[CMD_INVALID];
} 