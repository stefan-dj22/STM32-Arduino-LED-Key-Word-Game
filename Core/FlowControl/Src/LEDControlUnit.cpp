#include "LEDControlUnit.h"

LEDControlUnit::LEDControlUnit(TM1638plus* tm)
    : m_tm(tm)
    , m_ledsOn(false)
    , m_activeLeds(0x00)
    , m_blinkingOn(false)
    , m_blinkStartTime(0)
    , m_blinkInterval(LCU_ERROR_BLINK_INTERVAL)
{
    // Constructor implementation
}


void LEDControlUnit::setActiveLeds(uint8_t activeLeds)
{
	m_activeLeds = activeLeds;
}

void LEDControlUnit::addActiveLeds(uint8_t activeLeds)
{
	m_activeLeds |= activeLeds;
}
void LEDControlUnit::setBlinking(bool blinkingOn, uint32_t blinkStartTime, uint32_t blinkInterval)
{
	m_blinkingOn = blinkingOn;
	m_blinkStartTime = blinkStartTime;
	m_blinkInterval = blinkInterval;
}

void LEDControlUnit::setBlinkingOff()
{
	m_blinkingOn = false;
	m_blinkStartTime = 0;
	m_blinkInterval = 0;
}

void LEDControlUnit::setLedsOn(bool ledsOn)
{
	m_ledsOn = ledsOn;
}

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
