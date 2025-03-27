#include "command_utils.h"
#include "timer_utils.h"
#include "debug_log.h"

#define SEQUENCE_TIMEOUT_MS 1500  // Time window for command sequence
#define NEXT_BUTTON 7            // Button 8 (0-based index)
#define PREV_BUTTON 0            // Button 1 (0-based index)
#define MULTI_CLICK_BUTTONS (1 << NEXT_BUTTON | 1 << PREV_BUTTON)  // Buttons that can be used for multi-click
#define NEXT_PREV_COUNT 3        // Number of presses needed for NEXT/PREV

void Command_Init(Command_t* cmd) {
    cmd->button_pressed = 0;
    cmd->button_clicked = 0;
    cmd->click_count = 0;
    cmd->sequence_complete = false;
    cmd->last_press_time = 0;
    cmd->current_cmd = CMD_NONE;
    DBG_DEBUG(DBG_CAT_COMMAND, "Command initialized");
}

uint8_t New_Action_Check(Command_t* cmd, uint8_t debaunced_buttons)
{ 
    //if more than one button is pressed
    if(debaunced_buttons & (debaunced_buttons - 1))
    {
        return 0;
    }
    //Button 0 - left end position
    //Button 7 - right end position
    //if new press is more right than the last pressed button (on higher bit position), return 0
    if(cmd->button_clicked > 0 && debaunced_buttons > 0)
    {
        uint8_t lb_btn_clk = cmd->button_clicked & (-cmd->button_clicked); //lowest bit set in button_clicked
        if(debaunced_buttons > lb_btn_clk) //new press is more right than the last clicked button
        {
            return 0;
        }
    }
    //If pressed button is one of the already clicked buttons and not multi click button and there is just one button clicked
    if((cmd->button_clicked & debaunced_buttons) && (!(debaunced_buttons & MULTI_CLICK_BUTTONS)
        || (cmd->button_clicked & (cmd->button_clicked-1))))
    {
        return 0;
    }
    //if new press is not the same as the last pressed button
    if(cmd->button_pressed != 0 && debaunced_buttons != 0 && cmd->button_pressed != debaunced_buttons)
    {
        return 0;
    }

    return 1;
}

void New_Press_Process(Command_t* cmd, uint8_t debaunced_buttons, uint32_t current_time)
{
    //update new press parameters
    //check for multi click
    cmd->last_press_time = current_time;
    cmd->button_pressed = debaunced_buttons;
    if(cmd->button_pressed & MULTI_CLICK_BUTTONS)
    {
        cmd->click_count++;
        DBG_DEBUG(DBG_CAT_COMMAND, "Multi click detected: count=%d", cmd->click_count);
        if(cmd->click_count == NEXT_PREV_COUNT)
        {
            cmd->current_cmd = (cmd->button_pressed == (1 << NEXT_BUTTON)) ? CMD_NEXT : CMD_PREV;
            DBG_INFO(DBG_CAT_COMMAND, "%s command detected", 
                cmd->current_cmd == CMD_NEXT ? "NEXT" : "PREV");
        }
    }
    else
    {
        cmd->click_count = 1;
        DBG_DEBUG(DBG_CAT_COMMAND, "New press detected");
    }
}

void Press_Process(Command_t* cmd, uint8_t debaunced_buttons, bool long_press_detected, uint32_t current_time)
{
    //process press btn
    if(cmd->button_pressed == 0) //new press
    {
        New_Press_Process(cmd, debaunced_buttons, current_time);
    }
    else //old press
    {
        //check for long press
        if(long_press_detected)
        {
            cmd->current_cmd = CMD_SET;
            DBG_INFO(DBG_CAT_COMMAND, "SET command detected");
        }
    }
}

void Command_Process(Command_t* cmd, uint8_t debaunced_buttons, bool long_press_detected)
{
    uint32_t current_time = TimerUtils_GetTick();

    //If no press has been detected, set none command
    if(cmd->last_press_time == 0)
    {
        if(debaunced_buttons > 0)
        {
            cmd->last_press_time = current_time;
        }
    }
    //check if new button pressed is valid 
    else if (!New_Action_Check(cmd, debaunced_buttons))
    {
        cmd->current_cmd = CMD_INVALID;
        DBG_INFO(DBG_CAT_COMMAND, "Bad action - Invalid command detected");
    }
    else if(debaunced_buttons != 0)
    {
        Press_Process(cmd, debaunced_buttons, long_press_detected, current_time);
    }
    else if(!cmd->sequence_complete)
    {
        cmd->button_clicked |= cmd->button_pressed;
        cmd->button_pressed = debaunced_buttons;

        if(cmd->current_cmd != CMD_NONE)
        {
            cmd->sequence_complete = true;
        }

        if(!cmd->sequence_complete && ((current_time - cmd->last_press_time) > SEQUENCE_TIMEOUT_MS))
        {
            cmd->current_cmd = CMD_VIEW;
            DBG_INFO(DBG_CAT_COMMAND, "VIEW command detected");
            cmd->sequence_complete = true;
        }   
    }
}

CommandType_t Command_GetType(Command_t* cmd) {
    return cmd->current_cmd;
}

bool Command_IsSequenceComplete(Command_t* cmd) {
    return cmd->sequence_complete;
}   
