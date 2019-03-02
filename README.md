# grbl_controller_esp32
This Grbl controller runs on a ESP32

This project allows to control a CNC running GRBL without having to use a pc.<br>

This is an alternative to Marlin or Repetier for CNC.

Note: GRBL has to run on a separate micro computer (e.g. an Arduino Uno, Mega or a STM32 blue pill).<br>
This project only intends to replace the PC by an ESP32 board, not to replace GRBL.

This application allows to:
- select a file on the SD card (with Gcode) and send it to GRBL for execution
- pause/resume/cancel sending the Gcode
- send predefined set of GRBL commands (= macros)
- unlock grbl alarm
- ask grbl for homing the CNC
- move X,Y, Z axis by 0.01, 0.1, 1, 10 mm steps
- set X, Y, Z Work position to 0 (based on the current position)
- forward GRBL commands from the PC (so you can still control your CNC using your pc with e.g. Universal Gcode sender).<br>
   This is e.g. useful to configure GRBL from the PC with GRBL "$" commands.<br>
   The Gcode/commands can be sent using or <br>
   - the USB (serial) interface that exist on the ESP32 developement board.<br>
   - a telnet protocol over Wifi (in this case, best is to use BCC software on pc side because it support telnet connection)
- let a browser session running on pc connect to the ESP32 over Wifi in order to upload/download files between the pc and the SD card.
   So you can avoid physical manipulation of SD card.
   This works currently only with the root directory on the SD card (not with subdirectory) 

This application displays some useful GRBL informations like
- the source of the Gcode being sent to GRBL (e.g. SD card, USB, Telnet)
- the GRBL status (Idle, Run, Alarm,...)
- the work position (Wpos) and the machine position (Mpos)
- the last error and alert message.

Optionnally you can connect a Nunchuk (kind of joystick) in order to move the X, Y, Z, axis.<br>
To move the axis, you have to move the joystick (up/down/left/right) and simultaneously 
- press the C button to move X/Y axis,
- press the Z button to move Z axis
Nunchuk is automatically disconnected when the source of Gcode USB or Telnet.

To implement this project you need:
- an ESP32 development board and
- a display module combining 3 components:
  - a touch screen 320 X 240 a ILI9341 (display controller),
  - a XPT2046 (control the touch panel) and
  - a SD card support.<br>
  
It should also be possible to use a separate SD card support.<br> 
Note: this configuration uses the ILI9341 with 4 wires (CLK, MOSI, MISO and CD).<br>
Currently, this project works only with a TFT having a ILI9341 chip, a resolution of 320X240 and XPT2046 chip for the touch screen. If you are using another display, you should change the code yourself.

This project compiles in Arduino IDE but it requires:
- to add in Arduino IDE the software that support ESP32. The process is explained e.g. in this link
	https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
- to edit the file config.h from this project in order to specify if you plan to use the WiFi and if so if you are using the ESP32 in station mode or in access point mode.
   When using the WiFi, you must also specify the SSID (= name of the access point) and the password (in access point mode, it must contain at least 8 char)
 
Take care that if you do not use my configuration, you can have:
-  to edit the file User_setup.h included in the TFT_eSPI_ms folder.<br>
   This file specify the type of display controller and the pins being used by the SPI, the display and the touch screen chip select.
-  to edit the file config.h from this project in order to specify some pins being used (e.g. the chip select for the SD card reader)
  
Note: the pins used for the SPI signals (MOSI, MISO, SCLK) are currently hardcoded.<br>
Please note that many ESP32 pins are reserved and can't ne used (reserved for bootup, for internal flash, input only,...).
See doc on ESP for more details.

Note: the first time you let ESP32 run this program, the program should execute a calibration of the touch panel.<br>
The screen should first display an arrow in a corner. You have to click on the corner.<br>
When done, an arrow should also be displayed sucessively in the 3 others corners. Click each time on the arrow.<br>
This should normally be done only once. The calibration data are stored automatically in the ESP32.<br>   
If you want to recalibrate your screen, you should change the "config.h" file (parameter 


Here the connections being used in my own setup.<br>
Between ESP32 and TFT.
- 5V  - - - - - -> Vcc (on my TFT, it provides internally voltage to SD card)
- Gpio13 - - -> CS(chip select)
- Grnd - - - -> Grnd (on my TFT, it provides internally grnd to SD card)
- Gpio12 - - -> Reset
- Gpio14 - - -> DC (data/command)
- Gpio25 - - -> CD
- Gpio18 - - -> CLK
- Gpio19 - - -> MISO
- Gpio23 - - -> MOSI


Between ESP32 and touch screen
- Gpio27 - - -> CS (chip select)
- Gpio18 - - -> CLK
- Gpio19 - - -> MISO(T_DO)
- Gpio23 - - -> MOSI(T_DIN)


Between ESP32 and SD card
- Gpio26 - - -> CS (chip select)
- Gpio18 - - -> CLK
- Gpio19 - - -> MISO
- Gpio23 - - -> MOSI


Between ESP32 and GRBL computer
- Gpio16 is the Serial port Rx from ESP32; It has to be connected to TX pin from GRBL
- Gpio17 is the Serial port Tx from ESP32; It has to be connected to RX pin from GRBL
Note : Grnd has to be common between ESP32 and GRBL computer.


Between ESP32 and Nunchuk
- 3.3V => Vcc
- Grnd => Grnd
- Gpio21 => SDA
- Gpio22 => SCL
	