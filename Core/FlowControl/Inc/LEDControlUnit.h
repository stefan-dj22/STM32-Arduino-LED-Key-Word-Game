/**
 * @file LEDControlUnit.h
 * @brief LED Control Unit for managing TM1638 LED outputs
 * @details This module provides control over LED states, patterns, and blinking functionality
 *          for the TM1638 display module.
 */

#ifndef LED_CONTROL_UNIT_H
#define LED_CONTROL_UNIT_H

#include <stdint.h>
#include "TM1638plus.h"
#include "timer_utils.h"
#include "command_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Timing constants for LED control
 * @{
 */
#define LCU_LED_NONE_MASK 0x00u
#define LCU_LED_ALL_MASK 0xffu
#define LCU_ERROR_BLINK_INTERVAL 200u     /**< Blink interval for error state in milliseconds */
#define LCU_DISPLAY_SET_BLINK_INTERVAL 300u /**< Blink interval for display set state in milliseconds */
#define LCU_NO_BLINK_INTERVAL 0u          /**< Constant for no blinking */
#define LCU_ALL_LEDS_ON_CMD 0xFF00u       /**< Command to turn on all LEDs */
#define LCU_ALL_LEDS_OFF_CMD 0x0000u      /**< Command to turn off all LEDs */
/** @} */

#ifdef __cplusplus
}
#endif

/**
 * @brief LED Control Unit for managing LED outputs
 * 
 * This class manages:
 * - LED patterns based on application state
 * - Button feedback LEDs
 * - Blinking patterns for various states
 * - Error indication through LED patterns
 * 
 * The LEDs are controlled through the TM1638 display module, which provides
 * 8 LED outputs that can be individually controlled.
 */
class LEDControlUnit {
public:
    /**
     * @brief Construct a new LED Control Unit
     * 
     * @param tm Pointer to TM1638plus display object for LED control
     * @note The TM1638plus object must be initialized before being passed to this constructor
     */
    LEDControlUnit(TM1638plus* tm);
    
    /**
     * @brief Set which LEDs should be active
     * 
     * @param activeLeds Bitmap of LEDs to activate (bits 0-7 correspond to LEDs 1-8)
     * @note This doesn't immediately turn on the LEDs, use updateLEDs() to apply changes
     */
    void setActiveLeds(uint8_t activeLeds);

    
    void addActiveLeds(uint8_t activeLeds);
    /**
     * @brief Configure LED blinking behavior
     * 
     * @param blinkingOn Enable/disable blinking
     * @param blinkStartTime Initial timestamp for blink timing
     * @param blinkInterval Time between blink states in milliseconds
     */
    void setBlinking(bool blinkingOn, uint32_t blinkStartTime, uint32_t blinkInterval);

    void clear();
    /**
     * @brief Set the overall LED state
     * 
     * @param ledsOn True to enable configured LED pattern, false to turn all LEDs off
     */
    void setLedsOn(bool ledsOn);

    /**
     * @brief Update LED states based on current configuration
     * 
     * This method should be called periodically to:
     * - Apply LED pattern changes
     * - Handle blinking timing
     * - Update physical LED states
     */
    void updateLEDs();

    /**
     * @brief Disable LED blinking
     * 
     * Convenience method to turn off blinking and reset blink timing
     */
    void setBlinkingOff();
    
private:
    TM1638plus* m_tm;                 /**< Pointer to TM1638 display module */
    
    bool m_ledsOn;                    /**< Master LED enable state */
    uint8_t m_activeLeds;            /**< Bitmap of which LEDs should be on */
    
    bool m_blinkingOn;                /**< Whether blinking is enabled */
    uint32_t m_blinkStartTime;        /**< Timestamp of last blink state change */
    uint32_t m_blinkInterval;         /**< Time between blink states in ms */

};

#endif // LED_CONTROL_UNIT_H
