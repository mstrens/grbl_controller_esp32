#ifndef _icons_h
#define _icons_h

#include "config.h"

#if defined( TFT_SIZE) and (TFT_SIZE == 3)

const uint8_t logoIcon[] = {
#include "icons/bmp74/logo.h "  
};

const uint8_t setupIcon[] = {
#include "icons/bmp74/setup_button.h "  
};
const uint8_t printIcon[] = {
#include "icons/bmp74/mill_button.h "  
};
const uint8_t homeIcon[] = {
#include "icons/bmp74/home_button.h "  
};
const uint8_t unlockIcon[] = {
#include "icons/bmp74/unlock_button.h "  
};
const uint8_t resetIcon[] = {
#include "icons/bmp74/reset_button.h "  
};
const uint8_t sdIcon[] = {
#include "icons/bmp74/sdcard_button.h "  
};
const uint8_t usbIcon[] = {
#include "icons/bmp74/usb_button.h "  
};
const uint8_t telnetIcon[] = {
#include "icons/bmp74/telnet_button.h "  
};
const uint8_t pauseIcon[] = {
#include "icons/bmp74/pause_button.h "  
};
const uint8_t cancelIcon[] = {
#include "icons/bmp74/cancel_button.h "  
};
const uint8_t infoIcon[] = {
#include "icons/bmp74/info_button.h "  
};
const uint8_t cmdIcon[] = {
#include "icons/bmp74/cmd_button.h "  
};
const uint8_t moveIcon[] = {
#include "icons/bmp74/move_button.h "  
};
const uint8_t resumeIcon[] = {
#include "icons/bmp74/resume_button.h "  
};
const uint8_t stopIcon[] = {
#include "icons/bmp74/stop_PC_button.h "  
};
const uint8_t xpIcon[] = {
#include "icons/bmp74/arrow_Xright_button.h "  
};
const uint8_t xmIcon[] = {
#include "icons/bmp74/arrow_Xleft_button.h "  
};
const uint8_t ypIcon[] = {
#include "icons/bmp74/arrow_Yup_button.h "  
};
const uint8_t ymIcon[] = {
#include "icons/bmp74/arrow_Ydown_button.h "  
};
const uint8_t zpIcon[] = {
#include "icons/bmp74/arrow_Zup_button.h "  
};
const uint8_t zmIcon[] = {
#include "icons/bmp74/arrow_Zdown_button.h "  
};
const uint8_t dAutoIcon[] = {
#include "icons/bmp74/pitch_auto_button.h "  
};
const uint8_t d0_01Icon[] = {
#include "icons/bmp74/pitch_0_01_button.h "  
};
const uint8_t d0_1Icon[] = {
#include "icons/bmp74/pitch_0_1_button.h "  
};
const uint8_t d1Icon[] = {
#include "icons/bmp74/pitch_1_button.h "  
};
const uint8_t d10Icon[] = {
#include "icons/bmp74/pitch_10_button.h "  
};
const uint8_t d100Icon[] = {                  
#include "icons/bmp74/pitch_100_button.h "  
};
const uint8_t setWCSIcon[] = {
#include "icons/bmp74/set_WCS_button.h "  
};
const uint8_t setXIcon[] = {
#include "icons/bmp74/setX_button.h "  
};
const uint8_t setYIcon[] = {
#include "icons/bmp74/setY_button.h "  
};
const uint8_t setZIcon[] = {
#include "icons/bmp74/setZ_button.h "  
};
const uint8_t setXYZIcon[] = {
#include "icons/bmp74/setXYZ_button.h "  
};
const uint8_t toolIcon[] = {
#include "icons/bmp74/tool_button.h "  
};
const uint8_t backIcon[] = {
#include "icons/bmp74/back_button.h "  
};
//const uint8_t leftIcon[] = {
//#include "icons/bmp74/.h "  
//};
//const uint8_t rightIcon[] = {
//#include "icons/bmp74/ .h "  
//};
const uint8_t upIcon[] = {
#include "icons/bmp74/up_button.h "  
};
const uint8_t morePauseIcon[] = {
#include "icons/bmp74/more_button.h "  
};
const uint8_t pgPrevIcon[] = {
#include "icons/bmp74/previous_button.h "  
};
const uint8_t pgNextIcon[] = {
#include "icons/bmp74/next_button.h "  
};
const uint8_t sdShowIcon[] = {
#include "icons/bmp74/see_gcode_button.h "  
};

//added lines by HTheatre (until //********************************************* 4" TFT)

const uint8_t overwriteIcon[] = {
#include "icons/bmp74/feedrate_rpm_button.h "  
};
const uint8_t over_switch_to_FeedrateIcon[] = {
#include "icons/bmp74/feedrate_button.h "  
};
const uint8_t over_switch_to_SpindleIcon[] = {
#include "icons/bmp74/rpm_button.h "  
};
const uint8_t communicationIcon[] = {
#include "icons/bmp74/com_button.h "  
};
const uint8_t serialIcon[] = {
#include "icons/bmp74/grbl_serial_button.h "  
};
const uint8_t bluetoothIcon[] = {
#include "icons/bmp74/grbl_bluetooth_button.h "  
};
const uint8_t telnetGrblIcon[] = {
#include "icons/bmp74/grbl_telnet_button.h "  
};
const uint8_t sdGrblIcon[] = {
#include "icons/bmp74/grbl_sd_button.h "  
};
const uint8_t logGrblIcon[] = {
#include "icons/bmp74/grbl_msg_button.h "  
};
const uint8_t yesIcon[] = {
#include "icons/bmp74/yes_button.h "  
};
const uint8_t noIcon[] = {
#include "icons/bmp74/no_button.h "  
};


//********************************************* 4" TFT
#elif defined(TFT_SIZE) and (TFT_SIZE == 4)

const uint8_t logoIcon[] = {   // currently use the logo defined for 3.2" TFT
#include "icons/bmp74/logo.h "  
};

const uint8_t setupIcon[] = {
#include "icons/bmp100/setup1_button.h "  
};
const uint8_t printIcon[] = {
#include "icons/bmp100/mill1_button.h "  
};
const uint8_t homeIcon[] = {
#include "icons/bmp100/home1_button.h "  
};
const uint8_t unlockIcon[] = {
#include "icons/bmp100/unlock1_button.h "  
};
const uint8_t resetIcon[] = {
#include "icons/bmp100/reset1_button.h "  
};
const uint8_t sdIcon[] = {
#include "icons/bmp100/sdcard1_button.h "  
};
const uint8_t usbIcon[] = {
#include "icons/bmp100/usb1_button.h "  
};
const uint8_t telnetIcon[] = {
#include "icons/bmp100/telnet1_button.h "  
};
const uint8_t pauseIcon[] = {
#include "icons/bmp100/pause1_button.h "  
};
const uint8_t cancelIcon[] = {
#include "icons/bmp100/cancel1_button.h "  
};
const uint8_t infoIcon[] = {
#include "icons/bmp100/info1_button.h "  
};
const uint8_t cmdIcon[] = {
#include "icons/bmp100/cmd1(v_2)_button.h "  
};
const uint8_t moveIcon[] = {
#include "icons/bmp100/move1_button.h "  
};
const uint8_t resumeIcon[] = {
#include "icons/bmp100/resume1_button.h "  
};
const uint8_t stopIcon[] = {
#include "icons/bmp100/stop_PC1(v_3)_button.h "  
};
const uint8_t xpIcon[] = {
#include "icons/bmp100/arrow_Xright1_button.h "  
};
const uint8_t xmIcon[] = {
#include "icons/bmp100/arrow_Xleft1_button.h "  
};
const uint8_t ypIcon[] = {
#include "icons/bmp100/arrow_Yup1_button.h "  
};
const uint8_t ymIcon[] = {
#include "icons/bmp100/arrow_Ydown1_button.h "  
};
const uint8_t zpIcon[] = {
#include "icons/bmp100/arrow_Zup1_button.h "  
};
const uint8_t zmIcon[] = {
#include "icons/bmp100/arrow_Zdown1_button.h "  
};
const uint8_t dAutoIcon[] = {
#include "icons/bmp100/auto1_button.h "  
};
const uint8_t d0_01Icon[] = {
#include "icons/bmp100/d0_01_1_button.h "  
};
const uint8_t d0_1Icon[] = {
#include "icons/bmp100/d0_1_1_button.h "  
};
const uint8_t d1Icon[] = {
#include "icons/bmp100/d1_1_button.h "  
};
const uint8_t d10Icon[] = {
#include "icons/bmp100/d10_1_button.h "  
};
const uint8_t setWCSIcon[] = {
#include "icons/bmp100/set_WCS1_button.h "  
};
const uint8_t setXIcon[] = {
#include "icons/bmp100/setX1(v_2)_button.h "  
};
const uint8_t setYIcon[] = {
#include "icons/bmp100/setY1(v_2)_button.h "  
};
const uint8_t setZIcon[] = {
#include "icons/bmp100/setZ1(v_2)_button.h "  
};
const uint8_t setXYZIcon[] = {
#include "icons/bmp100/setXYZ1_button.h "  
};
const uint8_t toolIcon[] = {
#include "icons/bmp100/tool1(v_2)_button.h "  
};
const uint8_t backIcon[] = {
#include "icons/bmp100/back1_button.h "  
};
//const uint8_t leftIcon[] = {
//#include "icons/bmp100/.h "  
//};
//const uint8_t rightIcon[] = {
//#include "icons/bmp100/ .h "  
//};
const uint8_t upIcon[] = {
#include "icons/bmp100/up1_button.h "  
};
const uint8_t morePauseIcon[] = {
#include "icons/bmp100/more1_button (1).h "  
};
const uint8_t pgPrevIcon[] = {
#include "icons/bmp100/previous1_button.h "  
};
const uint8_t pgNextIcon[] = {
#include "icons/bmp100/next1_button.h "  
};
const uint8_t sdShowIcon[] = {
#include "icons/bmp100/see_gcode1_button (1).h "  
};


#else
#error TFT_SIZE must be 3 or 4 (see config.h)
#endif




#endif


