#include "button_utils.h"
#include "timer_utils.h"
#include "debug_log.h"

#define DEBOUNCE_TIME_MS 50
#define LONG_PRESS_TIME_MS 1000

void Button_Init(Button_t* button) {
    button->state = BUTTON_STATE_RELEASED;
    button->debounce_start = 0;
    button->press_start = 0;
    button->is_long_press = false;
    button->is_clicked = false;
    DBG_DEBUG(DBG_CAT_BUTTON, "Button initialized: state=RELEASED");
}

void Button_Process(Button_t* button, uint8_t raw_state) {
    uint32_t current_time = TimerUtils_GetTick();

    switch (button->state) {
        case BUTTON_STATE_RELEASED:
            if (raw_state) {
                button->state = BUTTON_STATE_DEBOUNCING;
                button->debounce_start = current_time;
                DBG_DEBUG(DBG_CAT_BUTTON, "Button press detected, entering debounce");
            }
            button->is_clicked = false;
            break;

        case BUTTON_STATE_DEBOUNCING:
            if (!raw_state) {
                button->state = BUTTON_STATE_RELEASED;
                DBG_DEBUG(DBG_CAT_BUTTON, "Button release during debounce");
            } else if (current_time - button->debounce_start >= DEBOUNCE_TIME_MS) {
                button->state = BUTTON_STATE_PRESSED;
                button->press_start = current_time;
                button->is_long_press = false;
                DBG_DEBUG(DBG_CAT_BUTTON, "Button debounced, now pressed");
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (!raw_state) {
                button->state = BUTTON_STATE_RELEASED;
                button->is_clicked = true;
                DBG_DEBUG(DBG_CAT_BUTTON, "Button released");
            } else if (!button->is_long_press && 
                      current_time - button->press_start >= LONG_PRESS_TIME_MS) {
                button->is_long_press = true;
                DBG_INFO(DBG_CAT_BUTTON, "Long press detected");
            }
            break;
    }
}

bool Button_IsPressed(Button_t* button) {
    return (button->state == BUTTON_STATE_PRESSED);
}

bool Button_IsLongPress(Button_t* button) {
    return button->is_long_press;
}

bool Button_IsClicked(Button_t* button) {
    return button->is_clicked;
}
