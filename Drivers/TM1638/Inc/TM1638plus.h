/*!
	@file     TM1638plus.h
	@author   Gavin Lyons
	@brief    Arduino library Tm1638plus, Header file for TM1638 module(LED & KEY). Model 1 & Model 3.
*/
/*
	Modified by: Stefan Djordjevic
	Description: Added new sliding text function and clear display function. Display functions are updated.
*/
#ifndef TM1638PLUS_H
#define TM1638PLUS_H

#include "TM1638plus_common.h"
#include "timer_utils.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
/*!
	@brief Class for Model 1 and Model 3
*/
class TM1638plus  : public TM1638plus_common {

public:
	/*!
		@brief Default constructor
	*/
	TM1638plus();

	/*!
		@brief Constructor with GPIO pin configuration
		@param strobe Strobe pin configuration
		@param clock Clock pin configuration
		@param data Data pin configuration
	*/
	TM1638plus(TM_GPIO_PinConfig_t strobe, TM_GPIO_PinConfig_t clock, TM_GPIO_PinConfig_t data);

	// Methods
	/*!
		@brief Read the button state from the module
		@return Byte with each bit representing a button (1 = pressed)
	*/
	uint8_t readButtons(void);
	
	/*!
		@brief Set all LEDs on the module at once
		@param greenred 16-bit value where bits 8-15 control the 8 LEDs
	*/
	void setLEDs(uint16_t greenred);
	
	/*!
		@brief Set a single LED
		@param position Position of LED (0-7)
		@param value LED state (0 = off, 1 = on)
	*/
	void setLED(uint8_t position, uint8_t value);

	/*!
		@brief Display text on the module
		@param text Text to display (up to 8 characters)
		@param textAlign Alignment type (left, right, center)
	*/
	void displayText(const char *text, AlignTextType_e textAlign);
	
	/*!
		@brief Display ASCII character at a specific position
		@param position Display position (0-7)
		@param ascii ASCII character code to display
	*/
	void displayASCII(uint8_t position, uint8_t ascii);
	
	/*!
		@brief Display ASCII character with decimal point
		@param position Display position (0-7)
		@param ascii ASCII character code to display
	*/
	void displayASCIIwDot(uint8_t position, uint8_t ascii);
	
	/*!
		@brief Display hexadecimal digit at a specific position
		@param position Display position (0-7)
		@param hex Hexadecimal value to display (0-F)
	*/
	void displayHex(uint8_t position, uint8_t hex);
	
	/*!
		@brief Display 7-segment value at a specific position
		@param position Display position (0-7)
		@param value 7-segment pattern to display
	*/
	void display7Seg(uint8_t position, uint8_t value);
	
	/*!
		@brief Clear the entire display
	*/
	void displayClear();
	
	/*!
		@brief Display integer number on the module
		@param number Number to display
		@param leadingZeros Whether to display leading zeros
		@param textAlign Alignment type (left, right, center)
	*/
	void displayIntNum(unsigned long number, bool leadingZeros = true, AlignTextType_e = TMAlignTextLeft);
	
	/*!
		@brief Display two separate numbers on upper and lower nibbles
		@param numberUpper Number for upper display nibble
		@param numberLower Number for lower display nibble
		@param leadingZeros Whether to display leading zeros
		@param textAlign Alignment type (left, right, center)
	*/
	void DisplayDecNumNibble(uint16_t numberUpper, uint16_t numberLower, bool leadingZeros = true, AlignTextType_e = TMAlignTextLeft);
	
	/*!
		@brief Initialize sliding text on the display
		@param text Text to slide across the display
	*/
	void displaySlidingText(const char *text);
	
	/*!
		@brief Update sliding text animation
		@return 1 if text is still sliding, 0 when complete
	*/
	uint8_t updateSlidingText(void);

private:
	SoftTimer_t _slideTimer;       /*!< Timer for sliding text animation */
	const char* _currentText;      /*!< Pointer to current text being displayed */
	uint8_t _textSize;             /*!< Size of the sliding text */
	uint8_t _currentPosition;      /*!< Current position in the sliding text */
	uint8_t _isSliding;            /*!< Flag indicating if text is in sliding mode */
};

#endif
#endif// TM1638PLUS_H
