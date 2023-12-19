#pragma once
#include <SPI.h>
#include <Adafruit_ST7735.h> // https://github.com/adafruit/Adafruit-ST7735-Library library for ST7735
#include <RF24.h>			 // https://github.com/nRF24/RF24 - RF24 by TMRh20
#include "ButtonEvent.h"
#include "Settings.h"

/*
	If this is set to 1, test menu is completely disabled.
	(all code for test menu is removed before compiling)

	Last time I checked, disabling it saves almost 6.5 Kb of memory
*/
#define DISABLE_TEST_MENU 0
#define BUZZER 3
#define VIBR 5

// all objects defined in main .ino file that will also be used here
extern Adafruit_ST7735 display;
extern RF24 radio;
extern const byte BATTERY;

/*
	All string constants for menu options.
	Holding them in this way uses PROGMEM rather than RAM
	from https://playground.arduino.cc/Main/PROGMEM/

	First position is "Title" that will be on the top of the screen,
	rest are the options that user can select.
*/

// If making changes to main menu, search for "CHANGE ME IF MAKING CHANGES TO MAIN MENU" and change code there
const char _menu_main_0[] PROGMEM = "Arduino Brick Game";
const char _menu_main_1[] PROGMEM = "Play";
const char _menu_main_2[] PROGMEM = "Options";
const char _menu_main_3[] PROGMEM = "Info";
#if DISABLE_TEST_MENU == 0
const char _menu_main_4[] PROGMEM = "Test";
const char *const menuMain[] PROGMEM = {_menu_main_0, _menu_main_1, _menu_main_2, _menu_main_3, _menu_main_4};
#else
const char *const menuMain[] PROGMEM = {_menu_main_0, _menu_main_1, _menu_main_2, _menu_main_3};
#endif

const char _menu_play_0[] PROGMEM = "Games";
const char _menu_play_1[] PROGMEM = "Pong";
const char *const menuPlay[] PROGMEM = {_menu_play_0, _menu_play_1};

#if DISABLE_TEST_MENU == 0
const char _menu_test_0[] PROGMEM = "Test menu";
const char _menu_test_1[] PROGMEM = "Buzzer";
const char _menu_test_2[] PROGMEM = "Vibrator";
const char _menu_test_3[] PROGMEM = "Wireless";
const char _menu_test_4[] PROGMEM = "Buttons";
const char _menu_test_5[] PROGMEM = "Colors";
const char _menu_test_6[] PROGMEM = "Colors 2";
const char *const menuTest[] PROGMEM = {_menu_test_0, _menu_test_1, _menu_test_2, _menu_test_3, _menu_test_4, _menu_test_5, _menu_test_6};
#endif

const char _menu_options_0[] PROGMEM = "Options";
const char _menu_options_1[] PROGMEM = "Vibrations: ";
const char _menu_options_2[] PROGMEM = "Sound: ";
const char _menu_options_21[] PROGMEM = "Save to EEPROM";
const char _menu_options_3[] PROGMEM = "Set console ID 0";
const char _menu_options_4[] PROGMEM = "Set console ID 1";
const char _menu_options_5[] PROGMEM = "Reset to default";
const char *const menuOptions[] PROGMEM = {_menu_options_0, _menu_options_1, _menu_options_2, _menu_options_21, _menu_options_3, _menu_options_4, _menu_options_5};

const int menuOptionX = 20, menuOptionY = 40, menuOptionHeight = 9;

// bitmaps generated on http://javl.github.io/image2cpp/
//  'battery', 18x10px
const unsigned char bitmapBattery[] PROGMEM = {
	0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0xc0, 0x80, 0x00, 0xc0, 0x80,
	0x00, 0xc0, 0x80, 0x00, 0xc0, 0x80, 0x00, 0x80, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00};
// 'speaker', 8x10px
const unsigned char bitmapSpeaker[] PROGMEM = {
	0x07, 0x0d, 0x19, 0xf1, 0xa1, 0xa1, 0xf1, 0x19, 0x0d, 0x07};
// 'vibr', 10x10px
const unsigned char bitmapVibr[] PROGMEM = {
	0x00, 0x00, 0x80, 0x40, 0x5e, 0x80, 0x92, 0x40, 0x52, 0x80, 0x92, 0x40, 0x52, 0x80, 0x9e, 0x40,
	0x40, 0x80, 0x00, 0x00};

int playMenu();
void testMenu(), infoMenu(), optionsMenu(), drawInfoPanel();

/*
	To make printing the string from the progmem easier.
	Can be used like:
		printProgmem(&(menuMain[0]));

	x - optional parameter, where to set the cursor for printing
	y - optional parameter, where to set the cursor for printing
*/
void printProgmem(const char *const *string, int x = -1, int y = -1)
{
	char *buffer = new char[strlen(*string) + 1];
	strcpy_P(buffer, (char *)pgm_read_ptr(string));

	if (x >= 0 && y >= 0)
	{
		display.setCursor(x, y);
	}

	display.print(buffer);
	delete buffer;
}

/*
	Another print helper function. Works with F macro.
*/
template <typename type>
void print(type string, int x = -1, int y = -1, uint16_t color = -1)
{
	if (x >= 0 && y >= 0)
		display.setCursor(x, y);

	if (color != -1)
		display.setTextColor(color);
	else
		display.setTextColor(COLOR_WHITE);

	display.print(string);
}
void printOnOff(bool on, int y)
{
	if (on)
		print(F("ON"), 100, y, COLOR_GREEN);
	else
		print(F("OFF"), 100, y, COLOR_RED);
}
/*
	Prints the given char array so the text in centered
*/
void printCentered(char *str, int y)
{
	display.setCursor(128 / 2 - 6 * strlen(str) / 2, y);
	display.print(str);
}

/*
	Used to draw menu from the PROGMEM char arrays.
	Use the drawMenu macro to print.
	Use like: drawMenu(menuMain);
*/
#define drawMenu(progmemArr) _drawMenu(progmemArr, sizeof(progmemArr) / sizeof(char *))
void _drawMenu(const char *const *menu, const int menuSize)
{
	display.fillScreen(0);
	drawInfoPanel();
	printProgmem(&(menu[0]), 10, 0);

	for (int i = 1; i < menuSize; i++)
	{
		printProgmem(&(menu[i]), menuOptionX, menuOptionY + menuOptionHeight * (i - 1));
	}
}

/*
	Used to draw the arrow cursor that points to the menu option.
	If calling first time for the given menu, pass with (>=0, maxPosition) arguments.
	Then call it without any parameters.
*/
byte getMenuSelector(int reset = -1, int maxPosition = -1)
{
	static byte menuSelector = 0;	  // position of the cursor
	static byte menuSelectorLast = 0; // what was the last position? used to detect when to update the screen
	static byte maxMenuSelector = 0;

	if (reset >= 0)
	{
		menuSelector = reset;
		menuSelectorLast = -1; // enforce update
	}

	if (maxPosition >= 0)
	{
		maxMenuSelector = maxPosition;
	}

	if (menuSelector != menuSelectorLast)
	{
		// check the selector bounds:
		if (menuSelector < 0)
			menuSelector = maxMenuSelector;
		if (menuSelector > maxMenuSelector)
			menuSelector = 0;

		// clear the previous position of the cursor
		print(F("->"), menuOptionX - 15, menuOptionY + menuOptionHeight * menuSelectorLast, ST7735_BLACK);

		// print new cursor position
		print(F("->"), menuOptionX - 15, menuOptionY + menuOptionHeight * menuSelector);

		menuSelectorLast = menuSelector;
	}

	if (button.up.state())
	{
		if (menuSelector == 0)
			menuSelector = maxMenuSelector;
		else
			menuSelector--;
	}

	if (button.down.state())
		menuSelector++;

	return menuSelector;
}

/*
	Draws the small status icons on the bottom
*/
void drawInfoPanel()
{
	print(F("Id "), 0, 150);
	print(SETTINGS.id, -1, -1);
	const int xOffset = 128 - 18 - 2;
	const int yOffset = 160 - 10;

	// print battery icon
	double voltage = analogRead(BATTERY) * 5.0 / 1024.0;
	if (voltage > 4.2)
		voltage = 4.2;
	if (voltage < 3.5)
		voltage = 3.5;

	uint16_t color = COLOR_WHITE;
	if (voltage < 3.5)
		color = COLOR_RED;
	display.drawBitmap(xOffset, yOffset, bitmapBattery, 18, 10, COLOR_WHITE);

	// fill battery icon
	color = COLOR_GREEN;
	if (voltage < 3.8)
		color = COLOR_GREEN | COLOR_RED;
	if (voltage < 3.5)
		color = COLOR_RED;
	display.fillRect(xOffset + 2, yOffset + 3, map(voltage * 10, 35, 42, 0, 14), 4, color);

	// print the speaker icon
	color = COLOR_RED;
	if (SETTINGS.sound)
		color = COLOR_GREEN;
	display.drawBitmap(xOffset - 11, yOffset, bitmapSpeaker, 8, 10, color);

	// print the vibration icon
	color = COLOR_RED;
	if (SETTINGS.vibrations)
		color = COLOR_GREEN;
	display.drawBitmap(xOffset - 24, yOffset, bitmapVibr, 10, 10, color);
}

#if DISABLE_TEST_MENU == 0
bool sendTimeWireless(unsigned long time)
{
	// prepare to transmit data
	radio.stopListening();
	bool radioResult = radio.write(&time, sizeof(time));
	radio.startListening();
	return radioResult;
}
#endif
/*
	Returns -1 if no option was selected. Otherwise returns which game was selected.
*/
int mainMenu()
{
	byte menuSelector = 0;

	while (1)
	{
		drawMenu(menuMain);

		// calling it with menuSelector as parameter will make it try to restore last cursor position in this menu
		menuSelector = getMenuSelector(menuSelector, sizeof(menuMain) / sizeof(char *) - 2);

		while (1)
		{
			menuSelector = getMenuSelector();

			// if selected some option:
			if (button.ok.state() == 1 || button.right.state() == 1)
			{
				/*
					CHANGE ME IF MAKING CHANGES TO MAIN MENU
				*/

				if (menuSelector == 0)
				{
					int returned = playMenu();
					if (returned != -1)
						return returned;
				}
				else if (menuSelector == 1)
				{
					optionsMenu();
				}
				else if (menuSelector == 2)
				{
					infoMenu();
				}
				else
				{
#if DISABLE_TEST_MENU == 0
					testMenu();
#endif
				}

				// set so this function resets after returning from whatever was picked by the user
				break;
			}

			if (button.esc.state() == 1)
				return -1;
		}
	}
}

int playMenu()
{
	drawMenu(menuPlay);
	byte menuSelector = getMenuSelector(0, sizeof(menuPlay) / sizeof(char *) - 2);

	while (1)
	{
		menuSelector = getMenuSelector();

		// launch
		if (button.ok.state() == 1 || button.right.state() == 1)
		{
			return menuSelector;
		}

		// escape
		if (button.left.state() == 1 || button.esc.state() == 1)
		{
			return -1;
		}
	}
}

#if DISABLE_TEST_MENU == 0
void testMenu()
{
	byte menuSelector = 0;

	while (1)
	{
		drawMenu(menuTest);

		// calling it with menuSelector as parameter will make it try to restore last cursor position in this menu
		menuSelector = getMenuSelector(menuSelector, sizeof(menuTest) / sizeof(char *) - 2);

		while (1)
		{
			menuSelector = getMenuSelector();

			// if selected some option:
			if (button.ok.state() == 1 || button.right.state() == 1)
			{
				/*
					CHANGE ME IF MAKING CHANGES TO MAIN MENU
				*/
				display.fillScreen(0);
				display.setCursor(0, 0);

				if (menuSelector == 0) // buzzer
				{
					int valueToWrite = 1000;
					int lastValue = -1;

					print(F("Buzzer test"));
					print(F("Hold OK to test"), 10, 30);
					print(F("Frequency: "), 10, 40);

					while (1)
					{
						if (valueToWrite != lastValue)
						{
							if (valueToWrite < 0)
								valueToWrite = 0;
							if (valueToWrite > 20000)
								valueToWrite = 20000;

							print(lastValue, 10, 50, ST7735_BLACK);
							print(F(" hz"), -1, -1, ST7735_BLACK);

							print(valueToWrite, 10, 50);
							print(F(" hz"));

							lastValue = valueToWrite;
						}

						if (button.up.state())
							valueToWrite += 10;
						if (button.down.state())
							valueToWrite -= 10;
						if (button.esc.state() == 1 || button.left.state() == 1)
						{
							noTone(BUZZER);
							break;
						}

						if (button.ok.state() == 2)
						{
							tone(BUZZER, valueToWrite);
						}
						else
						{
							noTone(BUZZER);
						}
					}
				}
				else if (menuSelector == 1) // vibrator
				{

					int valueToWrite = 1000; // set to max
					int lastValue = -1;
					int writtenValue = -1;

					print(F("Vibrator test"));
					print(F("Hold OK to test"), 10, 30);
					print(F("PWM value: "), 10, 40);

					while (1)
					{
						if (valueToWrite != lastValue)
						{
							if (valueToWrite < 0)
								valueToWrite = 0;
							if (valueToWrite > 255)
								valueToWrite = 255;

							print(lastValue, 10, 50, ST77XX_BLACK);
							print(F(" / 255"), -1, -1, ST77XX_BLACK);

							print(valueToWrite, 10, 50);
							print(F(" / 255"));

							lastValue = valueToWrite;
						}

						if (button.up.state())
							valueToWrite += 10;
						if (button.down.state())
							valueToWrite -= 10;
						if (button.esc.state() == 1 || button.left.state() == 1)
						{
							digitalWrite(VIBR, LOW);
							break;
						}

						if (button.ok.state() == 2)
						{
							if (writtenValue != valueToWrite)
							{
								analogWrite(VIBR, valueToWrite);
								writtenValue = valueToWrite;
							}
						}
						else
						{
							digitalWrite(VIBR, LOW);
							writtenValue = 0;
						}
					}
				}
				else if (menuSelector == 2) // wireless
				{
					print(F("Wireless test"));
					print(F("This is radio #"), 0, 30);
					print(SETTINGS.id);
					print(F("Press OK to try to\ncommunicate"), 0, 40);
					print(F("Received ping in"), 0, 100);
					print(F("-"), 0, 110);

					unsigned long timeSend = millis(); // randomize this number, so the display will update
					unsigned long lastTimeSend = 0;
					unsigned long timeReceived = 0;
					unsigned long lastTimeReceived = timeSend; // again, so the display updates
					unsigned long lastDisplayUpdate = 0;
					unsigned long timeAnyMsgReceived = 0; // used for ping received message

					bool removePingInfo = 0;
					bool removePingInfo2 = 0;

					radio.powerUp();
					radio.startListening();

					while (1)
					{
						// clear ping sent message if enough time has passed
						if (removePingInfo && millis() - timeSend > 1000)
						{
							display.fillRect(0, 60, 128, 10, ST7735_BLACK);
							removePingInfo = 0;
						}

						if (removePingInfo2 && millis() - timeAnyMsgReceived > 1000)
						{
							display.fillRect(0, 70, 128, 20, ST7735_BLACK);
							removePingInfo2 = 0;
						}

						if (timeReceived != lastTimeReceived && timeSend != lastTimeSend)
						{
							display.fillRect(0, 110, 128, 10, ST7735_BLACK);
							print(timeReceived - timeSend, 0, 110);
							print(F(" ms"));

							lastTimeReceived = timeReceived;
							lastTimeSend = timeSend;
						}

						if (millis() - lastDisplayUpdate > 100)
						{
							if (timeReceived != 0)
							{
								print(F("Last ping sent "), 0, 130);
								display.fillRect(0, 140, 128, 10, ST7735_BLACK);

								unsigned long time = millis() - lastTimeReceived;
								print(time / 1000, 0, 140);
								print(F("."));
								print((time / 100) % 10);
								print(F(" s ago"));
							}

							lastDisplayUpdate = millis();
						}

						// check if any data available
						uint8_t pipe;
						if (radio.available(&pipe)) // is there a payload? get the pipe number that received it
						{
							uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
							unsigned long receivedData = 0;
							radio.read(&receivedData, bytes); // fetch payload from FIFO

							// if received same value as this that was send, this is a response to this device's ping
							// if not, then we must answer as other device initiated the ping
							if (receivedData == timeSend)
							{
								print(F("Ping returned"), 0, 70, ST7735_CYAN);
								timeReceived = millis();
							}
							else
							{
								print(F("Ping requested"), 0, 80, ST7735_BLUE);
								sendTimeWireless(receivedData);
							}

							timeAnyMsgReceived = millis();
							removePingInfo2 = 1;
						}

						if (button.esc.state() == 1 || button.left.state() == 1)
						{
							radio.startListening();
							radio.powerDown();
							delay(100);
							break;
						}

						// can hold the button
						if (button.ok.state())
						{
							if (millis() - timeSend > 200)
							{
								timeSend = millis();
								sendTimeWireless(timeSend);
								print(F("Ping sent..."), 0, 60, ST7735_YELLOW);
								removePingInfo = true;
							}
						}
					}
				}
				else if (menuSelector == 3) // buttons
				{
					int bOk = 0, bMenu = 0, bR = 0, bL = 0, bD = 0, bU = 0;
					bool updateRequired = 1;

					while (1)
					{
						if (button.esc.state())
							break;
						if (button.ok.state())
						{
							bOk++;
							updateRequired = 1;
						}
						if (button.menu.state())
						{
							bMenu++;
							updateRequired = 1;
						}
						if (button.up.state())
						{
							bU++;
							updateRequired = 1;
						}
						if (button.right.state())
						{
							bR++;
							updateRequired = 1;
						}
						if (button.down.state())
						{
							bD++;
							updateRequired = 1;
						}
						if (button.left.state())
						{
							bL++;
							updateRequired = 1;
						}

						if (updateRequired)
						{
							display.fillScreen(COLOR_BLACK);
							print(F("Ok: "), 0, 0, COLOR_WHITE);
							print(bOk);
							print(F("\nMenu: "));
							print(bMenu);
							print("\nUp: ");
							print(bU);
							print(F("\nRight: "));
							print(bR);
							print(F("\nDown: "));
							print(bD);
							print(F("\nLeft: "));
							print(bL);

							updateRequired = 0;
						}
					}
				}
				else if (menuSelector == 4) // colors
				{
					// r = red, rl = redLast
					int color[3] = {0}, colorLast[3] = {0};
					bool updateRequired = 1;
					byte innerMenu = getMenuSelector(0, 2);

					while (1)
					{
						// check if display update required
						for (int i = 0; i < 3; i++)
						{
							if (color[i] != colorLast[i])
							{
								updateRequired = 1;
								break;
							}
						}

						// update display
						if (updateRequired)
						{
							display.fillRect(0, menuOptionY, 128, 30, COLOR_BLACK);
							print(F("Red: "), 30, menuOptionY, COLOR_WHITE);
							print(color[0]);
							print(F("Green: "), 30, menuOptionY + 10);
							print(color[1]);
							print(F("Blue: "), 30, menuOptionY + 20);
							print(color[2]);

							unsigned int colorToPrint = getColor(color[0], color[1], color[2]);

							display.fillRect(0, 0, 128, menuOptionY, colorToPrint);
							display.fillRect(0, menuOptionY + 30, 128, 160 - 30 - menuOptionY, colorToPrint);

							// save the color that is printed
							for (int i = 0; i < 3; i++)
							{
								colorLast[i] = color[i];
							}

							updateRequired = 0;
						}

						// check buttons, up/down are handled by getMenuSelector
						if (button.esc.state())
							break;
						if (button.left.state())
						{
							color[innerMenu]--;
							if (color[innerMenu] < 0)
								color[innerMenu] = 31;
						}
						if (button.right.state())
						{
							color[innerMenu]++;
							if (color[innerMenu] > 31)
								color[innerMenu] = 0;
						}
						innerMenu = getMenuSelector();
					}
				}
				else if (menuSelector == 5) // colors
				{
					unsigned long lastUpdated = 0;
					unsigned long updateTime = 100;
					int r = 31, g = 0, b = 0, step = 0;
					bool printHint = true, printHintLast = false;

					while (1)
					{
						if (millis() - lastUpdated > updateTime)
						{
							// red -> red + blue
							if (step == 0)
							{
								b++;
								if (b >= 31)
									step++;
							}
							// red + blue -> blue
							else if (step == 1)
							{
								r--;
								if (r <= 0)
									step++;
							}
							// blue -> blue + green
							else if (step == 2)
							{
								g++;
								if (g >= 31)
									step++;
							}
							// blue + green -> green
							else if (step == 3)
							{
								b--;
								if (b <= 0)
									step++;
							}
							// green -> green + red
							else if (step == 4)
							{
								r++;
								if (r >= 31)
									step++;
							}
							// green + red -> red
							else if (step == 5)
							{
								g--;
								if (g <= 0)
									step = 0; // go back to start
							}

							unsigned int color = getColor(r, g, b);
							if (!printHint)
								display.fillScreen(color);
							else
							{
								display.fillRect(0, 0, 128, 70, color);
								display.fillRect(0, 90, 128, 70, color);
							}

							if (printHint != printHintLast)
							{
								print(F("Up/down change speed"), 0, 70, COLOR_WHITE);
								print(F("Ok to hide this msg"), 0, 80, COLOR_WHITE);

								printHintLast = true;
							}

							lastUpdated = millis();
						}

						if (button.up.state())
							if (updateTime < 10000)
								updateTime += 10;
						if (button.down.state())
							if (updateTime > 10)
								updateTime -= 10;
						if (button.esc.state() || button.left.state())
							break;
						if (button.ok.state() == 1)
						{
							display.fillRect(0, 70, 128, 20, COLOR_BLACK);
							printHint = !printHint;
							if (!printHint)
								printHintLast = 0;
						}
					}
				}

				// set so this function resets after returning from whatever was picked by the user
				break;
			}

			if (button.esc.state() == 1 || button.left.state() == 1)
				return;
		}
	}
}
#endif

void infoMenu()
{
	display.fillScreen(0);
	drawInfoPanel();
	print(F("Arduino Brick Game"), 10, 0);
	print(F("Made by:"), 0, 50);
	print(F("Peter Pacholarz"), 10, 65);
	print(F("For a UIC CS362"), 0, 95);
	print(F("project 2023"), 15, 105);

	unsigned long lcdUpdate = 0;

	while (1)
	{

		if (millis() - lcdUpdate > 1000)
		{
			display.fillRect(15, 125, 128 - 15, 10, COLOR_BLACK);
			print(F("Battery: "), 15, 125);
			print(analogRead(BATTERY) * 5.0 / 1024.0);
			print(F(" V"));

			lcdUpdate = millis();
		}

		// escape
		if (button.left.state() == 1 || button.esc.state() == 1)
		{
			return;
		}
	}
}

void optionsMenu()
{
	byte menuSelector = 0;

	while (1)
	{
		drawMenu(menuOptions);

		// draw vibration settings
		printOnOff(SETTINGS.vibrations, menuOptionY);
		// draw sound settings
		printOnOff(SETTINGS.sound, menuOptionY + menuOptionHeight);

		menuSelector = getMenuSelector(menuSelector, sizeof(menuOptions) / sizeof(char *) - 2);

		while (true)
		{
			menuSelector = getMenuSelector();

			// launch
			if (button.ok.state() == 1 || button.right.state() == 1)
			{
				if (menuSelector == 0) // vibr
				{
					SETTINGS.vibrations = !SETTINGS.vibrations;
				}
				else if (menuSelector == 1) // sound
				{
					SETTINGS.sound = !SETTINGS.sound;
				}
				else if (menuSelector == 2) // save
				{
					saveSettings();
				}
				else if (menuSelector == 3 || menuSelector == 4) // set id
				{
					SETTINGS.id = menuSelector - 3;
					saveSettings();

					display.fillScreen(COLOR_BLACK);
					print(F("ID of this console"), 0, 0, COLOR_WHITE);
					print(F("is set to "), 0, 10);
					print(SETTINGS.id);
					print(F("Restart required."), 0, 50, COLOR_RED | COLOR_GREEN);
					print(F("Turn the console OFF"), 0, 60, COLOR_RED);

					while (1)
						digitalWrite(VIBR, 0);
				}
				else if (menuSelector == 5) // reset default
				{
					defaultSettings();
				}

				break;
			}

			// escape
			if (button.left.state() == 1 || button.esc.state() == 1)
			{
				return;
			}
		}
	}
}