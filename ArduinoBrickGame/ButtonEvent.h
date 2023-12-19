#pragma once
/*
	Button debouncing class I wrote long time ago.
	To check the button state, need to continuously call the state() function.
	It returns 0 is button not pressed, 1 if short pressed and let go,
	2 if button is held down.

	There is a struct specific to this project defined after the class.
	All buttons can be check if you include this file.

	For button pins / names check that struct.

	Usage example:
		if (button.ok.state() == 1) Serial.println("Button OK pressed");

	There are also other function here for ease to include them in all other files.
*/

/*
	Color codes from our display are different that one defined
	in the library, so here are now fixed colors:
*/
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF // 0b 11111 111111 11111
#define COLOR_BLUE 0xF800  // 0b 11111 000000 00000
#define COLOR_GREEN 0x07E0 // 0b 00000 111111 00000
#define COLOR_RED 0x001F   // 0b 00000 000000 11111

#define buttonDebounceDelay 20 // minimum delay between change of signals
#define buttonHoldTimer 400	   // how long to hold the button before it's considered being hold

/*
	Returns a color code for the display.
	Red and blue are represented in 5 bits (0-31)
	Green is represented in 6 bits (0-63).

	This function accepts values 0-31 for all of the colors for simplicity.
*/
unsigned int getColor(int red, int green, int blue)
{
	if (red < 0)
		red = 0;
	else if (red > 31)
		red = 31;
	green = (float)green * 2.05;
	if (green < 0)
		green = 0;
	else if (green > 63)
		green = 63;
	if (blue < 0)
		blue = 0;
	else if (blue > 31)
		blue = 31;

	unsigned int rgb = blue << 11;
	rgb += green << 5;
	rgb += red;

	return rgb;
}

class BUTTON
{
private:
	bool buttonLastState;		  // what was last button state
	bool buttonCurrentState;	  // what is current state
	bool buttonPressUsed;		  // if button was determined to be pressed, use this to return true only once
	byte buttonID;				  // holds button id, like A1
	unsigned long buttonDebounce; // holds time when button changed last time

public:
	// constructor that sets all variables to default
	BUTTON(byte buttonID_c)
	{
		pinMode(buttonID_c, INPUT_PULLUP);

		// update all variables
		buttonID = buttonID_c;
		buttonDebounce = 0;
		buttonLastState = HIGH;
		buttonCurrentState = HIGH;
		buttonPressUsed = true;
	}

	/*
	Returns:
		- 0 if button not pressed
		- 1 if button pressed and released
		- 2 - if button is being held down
	*/
	int state()
	{
		// read actual button state
		buttonCurrentState = digitalRead(buttonID);

		// check if it changed from last time, if it did take note of the time
		if (buttonCurrentState != buttonLastState)
			buttonDebounce = millis();

		// update the last state variable
		buttonLastState = buttonCurrentState;

		// check if button is being held down
		if (millis() - buttonDebounce > buttonHoldTimer && buttonCurrentState == LOW)
		{
			// prevent from setting the "isPressed" and "isHeld" signal simultaneously
			buttonPressUsed = true;

			return 2;
		}

		// now check if button is in its current state for long enough, but trigger only if button is let go
		else if ((millis() - buttonDebounce) > buttonDebounceDelay && millis() - buttonDebounce < buttonHoldTimer)
		{
			// remember to return true only once per button press
			// to prevent value increase when button is being hold
			if (!buttonPressUsed && buttonCurrentState == HIGH)
			{
				buttonPressUsed = true;
				return 1;
			}

			// if not pressed, reset the 'use' variable
			if (buttonCurrentState == LOW)
				buttonPressUsed = false;
		}

		return 0;
	}

	/*
		Returns unfiltered state of the button.
		True if button is pressed, false otherwise.
	*/
	bool raw()
	{
		return digitalRead(buttonID) == LOW;
	}
};

struct s_button
{
	BUTTON ok = (A1);
	BUTTON esc = (A2);
	BUTTON menu = (A3);
	BUTTON up = (A4);
	BUTTON right = (A5);
	BUTTON down = (4);
	BUTTON left = (2);
} button;