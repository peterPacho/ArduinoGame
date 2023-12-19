#pragma once
#include <SPI.h>
#include <Adafruit_ST7735.h> // https://github.com/adafruit/Adafruit-ST7735-Library library for ST7735
#include <RF24.h>			 // https://github.com/nRF24/RF24 - RF24 by TMRh20
#include "ButtonEvent.h"
#include "MainMenu.h"
#include "Settings.h"

// all objects defined in main .ino file that will also be used here
extern Adafruit_ST7735 display;
extern RF24 radio;
extern void vibrate(unsigned long = __LONG_MAX__, byte = 255);
extern void toneHelper(unsigned int freq, unsigned int duration);

#define BALL_RADIUS 2
#define BALL_STARTING_VEL_X 1.5
#define BALL_STARTING_VEL_Y 1.5
#define BALL_INCREASE_V_PER_BOUNCE 0.07 // every time ball hits vertical wall its velX and velY will increase by this amount

#define SHOW_POINTS_TIMEOUT 1500 // how long to show score after some player scored
#define FIELD_WALL_THICKNESS 1
#define PLAYER_THICKNESS 1

#define VIBRATE_WALL_HIT 20
#define VIBRATE_POINT_LOST 300

#define TONE_WALL_HIT_FREQ 200
#define TONE_WALL_HIT_DUR 50

#define PLAYER_POINTS_MAX 100

const char _menu_0[] PROGMEM = "Pong";
const char _menu_1[] PROGMEM = "Single easy";
const char _menu_2[] PROGMEM = "Host multi";
const char _menu_3[] PROGMEM = "Join multi";
const char _menu_4[] PROGMEM = "Training";
const char *const menuMainPong[] PROGMEM = {_menu_0, _menu_1, _menu_2, _menu_3, _menu_4};

class Pong
{
private:
	class Platform;
	class Ball;

	void printField()
	{
		const unsigned int color = COLOR_WHITE;

		display.drawRect(0, 0, 128, 160, color);
	}

	/*
		This struct will be send over the radio in the multiplayer game.
	*/
	struct GameData
	{
		double ballPosX, ballPosY, ballVelX, ballVelY;
		byte platformPosX;
		byte score;

		GameData()
		{
		}
		GameData(const Platform &pl, const Ball &b)
		{
			this->ballPosX = b.posX;
			this->ballPosY = b.posY;
			this->ballVelX = b.velX;
			this->ballVelY = b.velY;

			this->platformPosX = pl.posX;
			this->score = pl.points;
		}
	};

	struct Ball
	{
		double posX, posY, velX, velY;

		Ball()
		{
		}
		Ball(double posX, double posY, double velX, double velY)
		{
			this->posX = posX;
			this->posY = posY;
			this->velX = velX;
			this->velY = velY;
		}

		// resets the ball position
		void reset(double velY)
		{
			draw(COLOR_BLACK);
			this->posX = 128 / 2 - BALL_RADIUS / 2;
			this->posY = 160 / 2 - BALL_RADIUS / 2;
			this->velX = 0;
			this->velY = velY;
		}

		void incSpeedHelper(double &speed)
		{
			if (speed < 0)
				speed -= BALL_INCREASE_V_PER_BOUNCE;
			else
				speed += BALL_INCREASE_V_PER_BOUNCE;
		}

		void incSpeed()
		{
			incSpeedHelper(velX);
			incSpeedHelper(velY);
		}

		void checkVerticalColl()
		{
			// if hit left or right wall, bounce back
			// using abs instead of just velX = -velX fixed the "bouncing" problem
			// with ball stuck inside the platform
			if (posX - BALL_RADIUS <= FIELD_WALL_THICKNESS + 1)
			{
				velX = abs(velX);
				vibrate(VIBRATE_WALL_HIT);
				toneHelper(TONE_WALL_HIT_FREQ, TONE_WALL_HIT_DUR);
				incSpeed();
			}
			else if (posX + BALL_RADIUS >= 127 - FIELD_WALL_THICKNESS)
			{
				velX = abs(velX) * -1.0;
				vibrate(VIBRATE_WALL_HIT);
				toneHelper(TONE_WALL_HIT_FREQ, TONE_WALL_HIT_DUR);
				incSpeed();
			}
		}

		void update()
		{
			// draw black to erase the ball from the screen, after updates it will be drawn in some color
			draw(COLOR_BLACK);

			checkVerticalColl();

			// update the position
			posX += velX;
			posY += velY;

			// make sure ball isn't off-screen / doesn't touch vertical walls
			if (posX > 128 - BALL_RADIUS - FIELD_WALL_THICKNESS)
				posX = 128 - BALL_RADIUS;
			if (posX < 0 + BALL_RADIUS + FIELD_WALL_THICKNESS)
				posX = BALL_RADIUS + FIELD_WALL_THICKNESS;

			draw(COLOR_WHITE);
		}

		/*
			Allows to control ball with buttons, for debugging.
		*/
		void updateFromButtons()
		{
			draw(COLOR_BLACK);
			if (button.up.state())
				posY--;
			if (button.down.state())
				posY++;
			if (button.left.state())
				posX--;
			if (button.right.state())
				posX++;
			checkVerticalColl();
			draw(COLOR_WHITE);
		}

		/*
			Returns:
				- true - if ball state ok
				- false - if ball fallen of vertical border of the screen / player failed

			(this function treats ball as it was a square, but it shouldn't matter)
		*/
		void bounce(const Platform &player)
		{
			vibrate(VIBRATE_WALL_HIT);
			toneHelper(TONE_WALL_HIT_FREQ, TONE_WALL_HIT_DUR);

			if (posY > 80)
			{
				velY = abs(velY) * -1.0;
			}
			else
			{
				velY = abs(velY);
			}

			// if ball was reset:
			if (velX == 0)
			{
				if (player.posX + player.width / 2 > (128 - FIELD_WALL_THICKNESS * 2) / 2)
				{
					velX = -(BALL_STARTING_VEL_X)-0.5;
				}
				else
				{
					velX = BALL_STARTING_VEL_X + 0.5;
				}

				velY *= 2;
			}
		}
		int checkPlatformCollision(const Platform &player)
		{
			// first check if this ball is close to vertical (y) position of the player
			// or can even the collision occur now or is the ball somewhere in the middle of the screen
			if (posY - BALL_RADIUS * 2 <= player.posY + PLAYER_THICKNESS && posY + BALL_RADIUS * 2 >= player.posY)
			{
				// hit the platform "flat", so just bounce back, same angle
				if (posX - BALL_RADIUS >= player.posX && posX + BALL_RADIUS <= player.posX + player.width)
				{
					bounce(player);
					return true;
				}
				/*
					hit the corner of the platform, so do more complicated vector change
				*/
				else if (posX + BALL_RADIUS >= player.posX && posX - BALL_RADIUS <= player.posX + player.width)
				{
					double velModifier = 0.25;
					if (velX < 0)
						velModifier = -velModifier;

					// if posX of the ball is smaller than middle of the platform - we are on leading corner
					if (posX < player.posX + player.width / 2)
					{
						if (velX >= 1.5)
						{
							velX -= velModifier;
							velY += abs(velModifier);
						}
					}
					else
					{
						if (velY >= 1.5)
						{
							velY -= abs(velModifier);
							velX += velModifier;
						}
					}

					bounce(player);
					return true;
				}

				// if reached this point then ball is crossing the horizontal line
				// but platform is too far
				return !(posY >= 159 || posY <= 0);
			}

			// display.fillRect( 10, 10, 100, 10, COLOR_BLACK );

			// if reached here, ball is nowhere to the platform
			return true;
		}

		void draw(unsigned int color)
		{
			display.drawCircle(posX, posY, BALL_RADIUS, color);
		}
	};

	struct Platform
	{
		byte posX, posY, width;
		unsigned int points = PLAYER_POINTS_MAX;

		Platform()
		{
		}
		Platform(byte posX, byte posY, byte width)
		{
			this->posX = posX;
			this->posY = posY;
			this->width = width;
		}

		void draw(unsigned int color)
		{
			display.fillRect(posX, posY, width, PLAYER_THICKNESS, color);
		}

		/*
			Returns true if platform was moved.
		*/
		bool getUserInput()
		{
			int increment = 0;

			if (button.left.raw())
			{
				if (posX > FIELD_WALL_THICKNESS + 1)
					increment = -2;
			}
			else if (button.right.raw())
			{
				if (posX < 128 - FIELD_WALL_THICKNESS - width)
					increment = 2;
			}

			// erase platform only if position changed
			if (increment != 0)
			{
				draw(COLOR_BLACK);
				posX += increment;
				return true;
			}

			return false;
		}
	};

	void drawField()
	{
		display.drawFastVLine(0, 0, 160, COLOR_WHITE);
		display.drawFastVLine(127, 0, 160, COLOR_WHITE);
	}

	Ball ball;
	Platform player1;
	Platform player2;
	byte mode = 0; // 0 playing single, 1 playing multi, 10 - training

	void printPoints()
	{
		display.fillRect(FIELD_WALL_THICKNESS, 75, 128 - FIELD_WALL_THICKNESS * 2, 8, COLOR_BLACK);
		char buffer[100] = {0};
		snprintf(buffer, 99, "You %d - %d other", PLAYER_POINTS_MAX - player2.points, PLAYER_POINTS_MAX - player1.points);
		printCentered(buffer, 75);
	}

	/*
		Starts the game
	*/
public:
	void pong_play()
	{
		ball = Ball(128 / 2 - BALL_RADIUS / 2, 160 / 2 - BALL_RADIUS / 2, BALL_STARTING_VEL_X, BALL_STARTING_VEL_Y);
		player1 = Platform(128 / 2 - 8, 160 - PLAYER_THICKNESS, 16);
		player2 = Platform(128 / 2 - 8, 0, 16);

		// if training mode make other player's platform full screen width
		if (mode == 10)
		{
			player2 = Platform(0, 0, 128);
		}
		// if playing multi and this is console 1, ball will start going towards the other player
		else if (mode == 1 && SETTINGS.id == 1)
		{
			ball.reset(-(BALL_STARTING_VEL_Y / 2));
		}
		else
		{
			ball.reset(BALL_STARTING_VEL_Y / 2);
		}

		display.fillScreen(COLOR_BLACK);
		drawField();
		radio.setPayloadSize(sizeof(GameData));
		radio.flush_rx();
		radio.flush_tx();

		unsigned long lastGameUpdate = 0;
		unsigned long lastRadio = millis();
		bool updateBallPositionOnceMore = true; // used to detect if other player's ball bounced off

		/*
					If showPoints = 0 - don't do anything
					If showPoints != 0 but within SHOW_POINTS_TIMEOUT display points
					Else - clear the display and set showPoints to 0
		*/
		unsigned long showPoints = 0;

		/*
			During the multiplayer game:
				If radio send returns false, counter is increased.
				If radio send returns true, counter is reset to 0.
		*/
		int errorCounter = 0;

		/*
			Main game loop. Gets user input, refreshes the screen.
		*/
		while (1)
		{
			vibrate();

			// update the positions and draw on the screen
			if (millis() - lastGameUpdate > 25)
			{
				lastGameUpdate = millis();

				// draw points display if needed
				if (showPoints != 0)
				{
					if (millis() - showPoints < SHOW_POINTS_TIMEOUT)
					{
						printPoints();
					}
					else
					{
						display.fillRect(0, 75, 128, 20, COLOR_BLACK);
						showPoints = 0;
					}
				}

				if (!ball.checkPlatformCollision(player1))
				{
					player1.points--; // points are stored inverted, 100 means 0 points, 99 means 1 point, etc.
					showPoints = millis();
					ball.reset(BALL_STARTING_VEL_Y / 2);
					vibrate(VIBRATE_POINT_LOST);
				}

				// check player2 collision only if playing single player
				if ((mode == 0 || mode == 10) && !ball.checkPlatformCollision(player2))
				{
					player2.points--; // points are stored inverted, 100 means 0 points, 99 means 1 point, etc.
					showPoints = millis();
					ball.reset(-(BALL_STARTING_VEL_Y / 2));
					vibrate(VIBRATE_POINT_LOST);
				}

				ball.update();

				player1.getUserInput();

				// easy mode - try to move other platform
				if (mode == 0 && ball.velY < 0)
				{
					player2.draw(COLOR_BLACK);

					// get how much movement is required to have same position as ball
					int movement = ball.posX - player2.posX;

					if (player2.posX > FIELD_WALL_THICKNESS || player2.posX < 128 - FIELD_WALL_THICKNESS - player2.width)
					{
						if (movement > 3)
							player2.posX += 2;
						else if (movement < 3)
							player2.posX -= 2;
					}
				}

				player1.draw(COLOR_WHITE);
				player2.draw(COLOR_WHITE);

				drawField();

				/*
				Send game state. Do it only after display drawn everything it needed.
				If this is host, send player1 and ball.
				If this is client, send player2.
				*/
				if (mode == 1 && millis() - lastRadio > 100)
				{
					radio.stopListening();
					bool radioResult = false;

					GameData gd(player1, ball);
					radioResult = radio.write(&gd, sizeof(gd));

					if (radioResult)
						errorCounter = 0;
					else
						errorCounter++;

					if (errorCounter > 10)
					{
						vibrate(10000);
						radio.powerDown();

						display.fillScreen(COLOR_BLACK);
						print(F("Disconnected"), 10, 10, COLOR_RED | COLOR_GREEN);
						print(F("Final score was"), 20, 55);

						printPoints();

						while (1)
						{
							if (button.esc.state() == 1)
								return;
						}
					}

					radio.startListening();
					lastRadio = millis();
				}
			}

			/*
				If data from the other console available.
				If ball is moving towards this player, don't update the position.
			*/
			if (mode == 1 && radio.available())
			{
				uint8_t payloadSize = radio.getPayloadSize();
				GameData gd;

				if (payloadSize == sizeof(GameData))
				{
					radio.read(&gd, payloadSize);

					// update other platform position
					player2.draw(COLOR_BLACK);
					player2.posX = 128 - gd.platformPosX - player2.width;
					player2.draw(COLOR_WHITE);

					if (player2.points != gd.score)
					{
						player2.points = gd.score;
						showPoints = millis();
					}

					// if ball is moving towards the other player, update the position
					if (gd.ballVelY > 0 || updateBallPositionOnceMore)
					{
						if (gd.ballVelY <= 0)
							updateBallPositionOnceMore = false;
						else
							updateBallPositionOnceMore = true;

						ball.draw(COLOR_BLACK);
						ball = Ball(128 - gd.ballPosX, 160 - gd.ballPosY, -gd.ballVelX, -gd.ballVelY);
						ball.draw(COLOR_WHITE);
					}

					lastRadio = millis() - 50;
				}
			}

			// check for user input
			if (button.esc.state() == 1)
			{
				vibrate(10000);
				display.fillScreen(COLOR_BLACK);
				print(F("Game ended"), 10, 10, COLOR_RED | COLOR_GREEN);
				print(F("Final score was"), 20, 55);

				printPoints();

				// wait for any key press
				while (!button.esc.state() && !button.left.state())
					;

				return;
			}
			if (button.menu.state() == 1)
			{
			}
		}
	}

	/*
		Goes to game's main menu.
	*/
	int pong_menu()
	{
		byte menuSelector = 0;

		while (1)
		{
			radio.flush_rx();
			radio.flush_tx();
			display.fillScreen(COLOR_BLACK);
			printProgmem(menuMainPong, 10, 0);
			printProgmem(menuMainPong + 1, menuOptionX, menuOptionY);
			printProgmem(menuMainPong + 2 + SETTINGS.id, menuOptionX, menuOptionY + menuOptionHeight);
			printProgmem(menuMainPong + 4, menuOptionX, menuOptionY + menuOptionHeight * 2);

			menuSelector = getMenuSelector(menuSelector, 2);
			bool innerLoop = true;

			while (innerLoop)
			{
				menuSelector = getMenuSelector();

				if (button.esc.state() == 1 || button.left.state() == 1)
				{
					return 0;
				}
				if (button.ok.state() == 1 || button.right.state() == 1)
				{
					display.fillScreen(COLOR_BLACK);

					if (menuSelector == 0 || menuSelector == 2)
					{
						mode = menuSelector * 5; // if selecting 2, we want mode to be 10
						return 1;
					}
					else if (menuSelector == 1)
					{
						// multi player
						printProgmem(menuMainPong, 10, 0);
						player2 = Platform(128 / 2 - 8, 0, 16);

						display.setCursor(0, 20);
						display.println(F("Waiting for other"));
						display.println(F("player to join...\n"));
						display.print(F("This is console #"));
						display.print(SETTINGS.id);

						// enable radio and start listening
						radio.powerUp();
						delay(250);

						// if hosting, create this player and platform
						if (SETTINGS.id == 0)
						{
							radio.startListening();
						}
						else
						{
							radio.stopListening();
						}

						// used to detect when connection successful or
						// for time keeping if this console is client / joining
						unsigned long dummyData = 0;

						while (1)
						{
							// if radioNumber 0, then host, so wait until some client sends data
							if (SETTINGS.id == 0)
							{
								if (radio.available())
								{
									uint8_t bytes = radio.getPayloadSize();

									radio.read(&dummyData, bytes);
									if (dummyData != 0)
									{
										mode = 1;
										return 1;
									}
								}
							}
							else
							{
								if (millis() - dummyData > 1000)
								{
									dummyData = millis();
									bool radioResult = radio.write(&dummyData, sizeof(dummyData));

									if (radioResult)
									{
										mode = 1;
										return 1;
									}
								}
							}

							if (button.esc.state() == 1 || button.left.state() == 1)
							{
								display.fillScreen(COLOR_BLACK);
								radio.powerDown();
								innerLoop = false;
								break;
							}
						}
					}
				}
			}
		}
	}
};
