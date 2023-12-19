/*
	List of references/libraries:
	- ST775 display driver - https://github.com/adafruit/Adafruit-ST7735-Library
	- Two way communication - https://www.hackster.io/lightthedreams/nrf24l01-for-communication-1-way-and-2-way-80e65c
*/

#include <SPI.h>
#include <Adafruit_ST7735.h> // https://github.com/adafruit/Adafruit-ST7735-Library library for ST7735
#include <RF24.h>			 // https://github.com/nRF24/RF24 - RF24 by TMRh20
#include <avr/eeprom.h>
#include "ButtonEvent.h"
#include "MainMenu.h"
#include "Pong.h"
#include "Settings.h"

/*
	Display has 128x160 resolution.

	Pin-out notes for NANO:
		- Both display and NRF24L0+ use SPI interface which means
			they should share those pins: MOSI - D11, MISO - D12, SCK - D13

			Display -> Arduino:
				- GND -> GND
				- VCC -> 5V
				- SCL -> D13 (SCK pin, Serial Clock)
				- SDA -> D11 (MOSI - Master out slave in)
				- RES -> D9
				- DC -> D8
				- CS -> D10
			NRF24L0+ -> Arduino:
			-----------------
			1 2             /   1 - GND                 2 - VCC (3.3V !!!)
			3 4             \   3 - CE (Chip enable)    4 - CSN - Chip Select Not
			5 6             /   5 - SCK (Serial Clock)  6 - MOSI (Master Out Slave In)
			7 8             \   7 - MISO (master in)    8 - IRQ - Interrupt pin
			-----------------
				- GND -> GND
				- VCC -> 3.3V !!!
				- CE -> D6
				- CSN -> D7
				- SCK -> D13
				- MOSI -> D11
				- MISO -> D12

	If uploading first time / using new arduino, go to settings and set the console ID.
*/
#define DISPLAY_CS 10
#define DISPLAY_RST 9
#define DISPLAY_DC 8
#define nRF24_CSN 7
#define nRF24_CE 6

#define BUZZER 3
#define VIBR 5
const byte BATTERY = A0; // connected to battery positive to measure battery voltage

const uint8_t radioAddress[][6] = {"1Node", "2Node"};

Adafruit_ST7735 display = Adafruit_ST7735(DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);

// radio setup from https://github.com/nRF24/RF24/blob/master/examples/GettingStarted/GettingStarted.ino
RF24 radio(nRF24_CE, nRF24_CSN);

/*
	This function should be called continuously in the loop without any parameters.
	If vibration is required, pass a parameter of how long it should vibrate.

	So there must be a call to any loop that might take a long time, or
	vibrations might be stuck ON.

	I'm assuming there will be no instance when we want vibrations to be longer than 2 seconds.
	If passed with vibr > 2000 it means "shut it off"
*/
void vibrate(unsigned long newDuration = __LONG_MAX__, byte newPwm = 255)
{
	static unsigned long startedAt = 0;
	static unsigned long duration = 0;

	// if passed with parameter and vibrations are not disabled
	if (newDuration != __LONG_MAX__ && newDuration <= 2000 && SETTINGS.vibrations)
	{
		duration = newDuration;
		analogWrite(VIBR, newPwm);
		startedAt = millis();
		return;
	}
	else if (newDuration != __LONG_MAX__ && newDuration > 2000)
	{
		duration = 0;
		startedAt = 0;
	}

	// if without, check if vibration should be turned off
	if (millis() - startedAt > duration)
	{
		digitalWrite(VIBR, 0);
	}
}

void toneHelper(unsigned int freq, unsigned int duration)
{
	if (SETTINGS.sound)
		tone(BUZZER, freq, duration);
	else
		noTone(BUZZER);
}

void setup(void)
{
	/*
		Load settings from EEPROM
	*/
	loadSettings();

	// Serial.begin(9600);
	pinMode(BUZZER, OUTPUT);
	pinMode(VIBR, OUTPUT);

	display.initR(INITR_GREENTAB);
	display.fillScreen(0);
	display.setCursor(0, 0);

	// initialize the transceiver on the SPI bus
	if (!radio.begin())
	{
		print(F("Radio fail"), 0, 0, COLOR_RED);

		while (1)
		{
		}
	}

	// radio.setPALevel( RF24_PA_LOW ); // RF24_PA_MAX is default.
	radio.setPALevel(RF24_PA_HIGH);

	// set the TX address of the RX node into the TX pipe
	radio.openWritingPipe(radioAddress[SETTINGS.id]); // always uses pipe 0

	// set the RX address of the TX node into a RX pipe
	radio.openReadingPipe(1, radioAddress[!SETTINGS.id]); // using pipe 1

	radio.powerDown();
}

void loop()
{
	radio.powerDown();
	vibrate(10000);
	digitalWrite(VIBR, 0);
	noTone(BUZZER);
	delay(100);

	int returnedFromMenu = mainMenu();

	if (returnedFromMenu == 0)
	{
		display.fillScreen(COLOR_BLACK);
		Pong pong;
		if (pong.pong_menu())
			pong.pong_play();
	}
}
