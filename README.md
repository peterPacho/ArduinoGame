<img src="https://github.com/peterPacho/ArduinoGame/blob/main/Media/1.jpg?raw=true" width="250">

## Description
"Retro Brick Game" inspired Arduino game console with multiplayer support. 

[<img src="https://github.com/peterPacho/ArduinoGame/blob/main/Media/play.png?raw=true" width="100">](https://www.youtube.com/watch?v=qxOgujPc98Q)

[Link to the video](https://www.youtube.com/watch?v=qxOgujPc98Q)

Made as a project for University Of Illinois CS 362 class, Fall 2023.

## Parts list (for a single console):
- Arduino Nano
- 128x160 LCD Display
- NRF24L0+ module
- 18650 3.7 Li-on battery
- Li-on charger/power bank module
- DC to DC converter (to convert too-low and varying battery voltage to stable 5V to power Arduino)
- Passive buzzer
- Vibration motor
- 2x 3.3V voltage regulators AMS1117-3.3 (NRF24L0+ module requires 3.3V and vibration motor requires 3V, they are separated to hopefully reduce any noise/interference from the DC motor)
- 2x mosfets (to act as switch for buzzer and DC motor)
- 7x PCB buttons
- slide switch / power switch
- and other supporting components like resistors, capacitors, wires, etc.

<img src="https://github.com/peterPacho/ArduinoGame/blob/main/Media/schematics.png?raw=true">
<img src="https://github.com/peterPacho/ArduinoGame/blob/main/Media/2.png?raw=true">
<img src="https://github.com/peterPacho/ArduinoGame/blob/main/Media/3.jpg?raw=true">
