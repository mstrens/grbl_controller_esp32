# grbl_controller_esp32
This Grbl controller runs on a ESP32

This project allows to control a CNC running GRBL without having to use a pc.

This is an alternative to Marlin or Repetier for CNC.

It uses an ESP32 and a display module having 3 components: a touch screen 320 X 240 a ILI9341 (display controller) , a XPT2046 (control the touch panel)
and a SD card support. It should also possible to use separate components instead of the display module. 
Note: this configuration uses the ILI9341 with 4 wires (CLK, MOSI, MISO and CD).

This application allows to:
- select a file on the SD card (with Gcode) and send it to GRBL for execution
- pause/resume/cancel sending the Gcode
- send predefined set of GRBL commands (= macros)
- unlock grbl alarm
- ask grbl for homing the CNC
- move X,Y, Z axis by 0.01, 0.1, 1, 10 mm steps
- set X, Y, Z Work position to 0 (based on the current position)
- forward GRBL commands from the PC (so you can still control your CNC using your pc with e.g. Universal Gcode sender).
   This is e.g. useful to configure GRBL from the PC with "$" commands
   The commands can be sent using or 
      - the USB (serial) interface that exist on the ESP32 developement board.
      - a telnet protocol over Wifi (in this case, best is to use BCC on pc side because it support telnet connection)
- let a browser session running on pc connect to the ESP32 over Wifi in order to upload/download files between the pc and the SD card.
   So you can avoid physical manipulation of SD card.
   This works currently only with the root directory on the SD card (not with subdirectory) 

This application displays some useful GRBL informations like
- the source of the Gcode being sent to GRBL (e.g. SD card, USB, Telnet)
- the GRBL status (Idle, Run, Alarm,...)
- the work position (Wpos) and the machine position (Mpos)
- the last error and alert message.

Optionnally you can connect a Nunchuk (kind of joystick) in order to move the X, Y, Z, axis.
To move the axis, you have to move the joystick (up/down/left/right) and simultaneously 
- press the C button to move X/Y axis,
- press the Z button to move Z axis
Nunchuk is automatically disconnected when the source of Gcode USB or Telnet.

This project compiles in Arduino IDE but it requires:
- to add in Arduino IDE the software that support ESP32. The process is explained e.g. in this link
	https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/
- to edit the file config.h from this project in order to specify if you plan to use the WiFi and if so if you are using the ESP32 in station mode or in access point mode.
   When using the WiFi, you must also specify the SSID (= name of the access point) and the password (in access mode, it must contain at least 8 char)
 
Take care that if you do not use the configuration as me, you can have:
-  to edit the file User_setup.h included in the TFT_eSPI_ms folder.
   This file specify the type of display controller and the pins being used by the SPI, the display and the touch screen chip select.
-  to edit the file config.h from this project in order to specify some pins being used (e.g. the chip select for the SD card reader)
  
Note: the pins used for the SPI signals (MOSI, MISO, SCLK) are currently hardcoded.
Please note that many ESP32 pins are reserved and can't ne used (reserved for bootup, for internal flash, input only,...).
See doc on ESP for more details.

Note: the first time you let ESP32 run this program, the program should execute a calibration of the touch panel.
The screen should first display an arrow in a corner. You have to click on the corner.
When done, an arrow should also be displayed sucessively in the 3 others corners. Click each time on the arrow.
This should normally be done only once. The calibration data are stored automatically in the ESP32.   
If you want to recalibrate your screen, you should change the "config.h" file (parameter 


Here the connections being used in my own setup.
ESP32___TFT________________touch screen__________SD card
5V______Vcc_______________________________________________________ (this is used internally to provide voltage to touch screen and SD card)
Gpio13__CS(chip select)
Grnd____Grnd ______________________________________________________(this is used internally to provide grnd to touch screen and SD card)
Gpio12__Reset
Gpio14__DC (data/command)
Gpio27______________________CS (chip select)
Gpio26___________________________________________CS (chip select)
Gpio25__LCD
Gpio18__CLK_________________CLK__________________CLK________________(So the ESP32 pin is connected to 3 pins from TFT board)
Gpio19__MISO________________MISO(T_DO)___________MISO_______________(So the ESP32 pin is connected to 3 pins from TFT board)
Gpio23__MOSI________________MOSI(T_DIN)__________MOSI_______________(So the ESP32 pin is connected to 3 pins from TFT board)

Gpio16 is the Serial port Rx from ESP32; It has to be connected to TX pin from GRBL
Gpio17 is the Serial port Tx from ESP32; It has to be connected to RX pin from GRBL
Note : Grnd has to be common between ESP32 and GRBL computer.

For Nunchuk, we use 4 wires
3.3V = Vcc
Grnd = Grnd
Gpio21 = SDA
Gpio22 = SCL