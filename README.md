# grbl_controller_esp32
Grbl controller running on ESP32

This project allows to control a CNC running GRBL without having to use a pc.

This is an alternative to Marlin or Repetier for CNC.

It uses an ESP32 and a touch screen 320 X 240 with a ILI9341 (display controller) and a XPT2046 (control the touch panel)
It also control a SD card.

This applications allows to:
- select a file on the SD card with Gcode and to send it to GRBL
- pause/resume/cancel sending the Gcode
- send predefined GRBL commands (based on the setup in the config.h file)
- unlock alarm
- ask for homing the CNC
- move X,Y, Z axis by 0.01, 0.1, 1, 10 mm steps
- set X, Y, Z Work position to 0 (based on the current position)
- forward GRBL commands from the PC (so you can still control your CNC using your pc with e.g. Universal Gcode sender).
  This uses the USB interface that exist on the ESP32 developement board.
- let a pc connect to the ESP32 with the WiFi in order to upload/download files from pc to the SD card connected to ESP32
  This works currently with the root directory on the SD card (not with subdirectory) 

This application displays some GRBL informations like
- the GRBL status (Idle, Run, Alarm,...)
- the work position (Wpos) and the machine position (Mpos)
- the last error and alert message.

Optionnally you can connect a Nunchuck in order to move the X, Y, Z, axis with the joystick 
- press the C button to move X/Y axis,
- press the Z button to move Z axis

This project compiles in Arduino IDE but it requires:
- to add in Arduino IDE the software that support ESP32. The process is explained e.g. in this link
	https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
- to install a library to manage the display. The library is tft_espi. It is avaiblable at 
	https://github.com/Bodmer/TFT_eSPI
	See Arduino doc about how to install a library.
Take care that you have to edit the file User_setup.h included in the TFT_espi library to specify the type of display controller and the pins being used by the SPI, the display and the touch screen chip select.
You have also to replace the content of the files Touch.h and Touch.cpp that are in the directory "Extensions" by those provided on this github repository in subdirectory Extensions.

Finally, you have to edit the file config.h from this project in order to 
- specify some pins being used (e.g. the chip select for the SD card reader)
- the name and content of the grbl commands you want to predefine (in order to activate them from the touch screen)
- specify if you plan to use the wifi and if so using the ESP32 in station mode or in access point mode.  
 
Note: the pins used for the SPI signals (MOSI, MISO, SCLK) are currently hardcoded.
Please note that many ESP32 pins are reserved and can't ne used (reserved for bootup, for internal flash, input only,...). See doc on ESP for more details.

Note: the first time you let ESP32 run this program, the program should execute a calibration of the touch panel.
The screen should first display an arrow in a corner. You have to click on the corner.
When done, an arrow should also be displayed in the 3 others corners. Click each time on the arrow.
This should normally be done only once. The calibration data are stored automatically in the ESP32.   


Here the connections being used in my own setup.
ESP32   TFT                touch screen          SD card
5V      Vcc (this is used internally to provide voltage to touch screen and SD card)
Gpio13  CS(chip select)
Grnd	Grnd (this is used internally to provide grnd to touch screen and SD card)
Gpio12  Reset
Gpio14  DC (data/command)
Gpio27                      CS (chip select)
Gpio26                                           CS (chip select)
Gpio25  LCD
Gpio18  CLK                 CLK                  CLK                (So the ESP32 pin is connected to 3 pins from TFT board)
Gpio19  MISO                MISO                 MISO               (So the ESP32 pin is connected to 3 pins from TFT board)
Gpio23  MOSI                MOSI                 MOSI               (So the ESP32 pin is connected to 3 pins from TFT board)

Gpio16 is the Serial port Rx from ESP32; It has to be connected to TX pin from GRBL
Gpio17 is the Serial port Tx from ESP32; It has to be connected to RX pin from GRBL
Note : Grnd has to be common between ESP32 and GRBL computer.