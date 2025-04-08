/**
 * @file LEDControlUnit.cpp
 * @brief Implementation of the LED Control Unit for TM1638 LED control
 * @details Implements LED control functionality including pattern display,
 *          blinking behavior, and LED state management.
 */

#include "LEDControlUnit.h"

/**
 * @brief Constructor for LEDControlUnit
 * 
 * Initializes the LED control unit with default values:
 * - LEDs turned off
 * - No active LEDs
 * - No blinking configured
 * 
 * @param tm Pointer to TM1638plus display object for LED control
 */
LEDControlUnit::LEDControlUnit(TM1638plus* tm)
    : m_tm(tm)
    , m_ledsOn(false)
    , m_activeLeds(LCU_LED_NONE_MASK)
    , m_blinkingOn(false)
    , m_blinkStartTime(0)
    , m_blinkInterval(LCU_NO_BLINK_INTERVAL)
{
    // Constructor implementation
}

/**
 * @brief Reset all LED settings to defaults
 * 
 * Clears all LED settings:
 * - Turns off all LEDs
 * - Clears active LED pattern
 * - Disables blinking
 * - Resets all timing values
 * - Immediately updates physical LEDs
 */
void LEDControlUnit::clear()
{
	m_ledsOn = false;
	m_activeLeds = LCU_LED_NONE_MASK;
	m_blinkingOn = false;
	m_blinkStartTime = 0;
	m_blinkInterval = 0;
	updateLEDs();
}

/**
 * @brief Set which LEDs should be active
 * 
 * @param activeLeds Bitmap of LEDs to activate (bits 0-7 correspond to LEDs 1-8)
 * @note This overwrites any previous LED pattern
 */
void LEDControlUnit::setActiveLeds(uint8_t activeLeds)
{
	m_activeLeds = activeLeds;
}

/**
 * @brief Add LEDs to current active pattern
 * 
 * @param activeLeds Bitmap of additional LEDs to activate
 * @note This preserves existing active LEDs and adds new ones
 */
void LEDControlUnit::addActiveLeds(uint8_t activeLeds)
{
	m_activeLeds |= activeLeds;
}

/**
 * @brief Configure LED blinking behavior
 * 
 * @param blinkingOn Enable/disable blinking
 * @param blinkStartTime Initial timestamp for blink timing
 * @param blinkInterval Time between blink states in milliseconds
 */
void LEDControlUnit::setBlinking(bool blinkingOn, uint32_t blinkStartTime, uint32_t blinkInterval)
{
	m_blinkingOn = blinkingOn;
	m_blinkStartTime = blinkStartTime;
	m_blinkInterval = blinkInterval;
}

/**
 * @brief Disable LED blinking
 * 
 * Convenience method to turn off blinking and reset blink timing
 */
void LEDControlUnit::setBlinkingOff()
{
	m_blinkingOn = false;
	m_blinkStartTime = 0;
	m_blinkInterval = 0;
}

/**
 * @brief Set the overall LED state
 * 
 * @param ledsOn True to enable configured LED pattern, false to turn all LEDs off
 */
void LEDControlUnit::setLedsOn(bool ledsOn)
{
	m_ledsOn = ledsOn;
}

/**
 * @brief Update LED states based on current configuration
 * 
 * This method should be called periodically to:
 * - Apply LED pattern changes
 * - Handle blinking timing
 * - Update physical LED states
 * 
 * If blinking is enabled, this alternates between showing the pattern and 
 * turning off all LEDs based on the configured interval.
 */
void LEDControlUnit::updateLEDs()
{
	if (m_tm == nullptr) {
		return;
	}

	if (m_blinkingOn)
	{
		uint32_t currentTime = TimerUtils_GetTick();
		if (currentTime - m_blinkStartTime >= m_blinkInterval) {
			if(m_ledsOn)
			{
				m_tm->setLEDs(uint16_t(m_activeLeds<<8));
			}
			else
			{
				m_tm->setLEDs(LCU_ALL_LEDS_OFF_CMD);
			}
			m_blinkStartTime = currentTime;
			m_ledsOn = !m_ledsOn;
		}
	}
	else if (m_ledsOn)
	{
		m_tm->setLEDs(uint16_t(m_activeLeds<<8));
	}
	else
	{
		m_tm->setLEDs(LCU_ALL_LEDS_OFF_CMD);
	}
}
