#ifndef _setupTxt_h
#define _setupTxt_h

#include <stdint.h>

void fillMPage (uint8_t _page , uint8_t _btnPos , uint8_t _boutons, uint8_t _actions , void (*_pfNext)(uint8_t) , uint8_t _parameters )  ;
void initButtons() ;   // initialise les noms des boutons, les boutons pour chaque page.
void loadLanguage(void) ; // change names (for Buttons, Messages, Errors and Alarm based on language file on SD or SPIFFS   
void handleLangLine(char* line) ;
bool copyLanguage(void) ; // copy file from SD to SPIFSS

// créer une stucture pour chaque sorte de bouton 
//  *texte, textsize , *iconName
// créer un array de structure de bouton
struct M_Button {
  //uint8_t  textsize ;  // To use if we want different text size.
  const char  * pLabel = 0; // when text is empty, then it is a hidden button (not displayed but can be pressed)
  const uint8_t * pIcon  = 0 ; // when 0 it means there is no icon; then use the text
};

struct M_pLabel {
  const char  * pLabel ;
};



// structure pour une page : PageMenu
//          un titre = pointeur vers une chaine de char
//          un pointeur vers une fonction pour gérer l'affichage en dehors des boutons
//          array de n° de bouton [12] dans l'ordre des 12 max à afficher, 0= ne pas afficher cet emplacement
//          array de code de type d'action par bouton (action = just pressed, long pressed, just released)
//          array de fonction à exécuter
//          array de paramètres à utiliser par la fonction
struct M_Page {            // defini une page
  const char  * titel ;          // un titre à afficher en haut de la page = pointeur vers une chaine de char
  void (*pfBase)(void);    // un pointeur vers une fonction pour gérer l'affichage en dehors des boutons
  uint8_t boutons[12] ;     // array de n° de bouton [12] dans l'ordre des 12 max à afficher, 0= ne pas afficher cet emplacement
  uint8_t actions[12] ;     // array de code de type d'action par bouton (action = just pressed, long pressed, just pressed or long pressed, just released)
  void (*pfNext[12])(uint8_t);    // array de fonction à exécuter pour le type d'action ci-avant; une fonction par bouton, elle prend 1 paramètre (en principe le n° de la page où sauter)
  uint8_t parameters[12] ;  // array de paramètres à utiliser par la fonction; un paramètre par bouton
};

// Liste des boutons disponibles
enum { _NO_BUTTON = 0 , _SETUP , _PRINT , _HOME, _UNLOCK , _RESET , _SD , _USB_GRBL , _TELNET_GRBL, _PAUSE ,
    _CANCEL , _INFO , _CMD ,_MOVE , _RESUME , _STOP_PC_GRBL , _XP , _XM , _YP ,_YM ,
    _ZP , _ZM, _AP, _AM, _D_AUTO , _D10 ,  _D1, _D0_1 ,  _D0_01 , _SET_WCS ,
    _SETX , _SETY , _SETZ, _SETA , _SETXYZ , _SETXYZA ,_SET_CHANGE,  _SET_PROBE, _SET_CAL , _GO_CHANGE,
    _GO_PROBE , _TOOL , _BACK , _LEFT, _RIGHT , _UP , _CMD1 ,_CMD2 ,_CMD3 ,_CMD4 ,
    _CMD5 ,_CMD6 ,_CMD7 , _CMD8 , _CMD9 , _CMD10 , _CMD11 , _MORE_PAUSE , _FILE0 , _FILE1 ,
    _FILE2 , _FILE3 , _MASKED1 , _PG_PREV , _PG_NEXT, _SD_SHOW ,  _OVERWRITE, _OVER_SWITCH_TO_FEEDRATE , _OVER_SWITCH_TO_SPINDLE, _OVER_100 ,
    _OVER_10P, _OVER_10M,_OVER_1P, _OVER_1M, _TOOGLE_SPINDLE, _COMMUNICATION , _SERIAL , _BLUETOOTH , _TELNET , _SD_GRBL,
    _FILE0_GRBL , _FILE1_GRBL , _FILE2_GRBL , _FILE3_GRBL , _YES_BTN, _NO_BTN , _LOG ,
    _MAX_BTN} ; // keep _MAX_BTN latest

// Liste des pages définies
enum { _P_NULL = 0  , _P_INFO , _P_SETUP , _P_PRINT , _P_PAUSE , _P_MOVE , _P_SETXYZ , _P_SD , _P_CMD , _P_LOG , _P_TOOL, 
      _P_SD_SHOW , _P_OVERWRITE, _P_COMMUNICATION, 
      _P_SD_GRBL_WAIT ,_P_SD_GRBL , _P_SD_CONFIRM , _P_PAUSE_GRBL ,
      _P_MAX_PAGES} ; // keep _P_MAX_PAGE latest

// Liste des Text définis
enum { _WPOS = 0, _MPOS , _FEED , _RPM , _TOUCH_CORNER , 
    _SET_REPEAT_CAL , _CAL_COMPLETED , _SPIFFS_FORMATTED, _CMD_NOT_LOADED , _TELENET_DISCONNECTED ,
    _INVALID_BTN_HOME , _CMD_NOT_RETRIEVED , _NO_TELNET_CONNECTION , _WIFI_NOT_FOUND , _SPIFFS_FAIL_TO_OPEN ,
    _FAILED_TO_CREATE_CMD , _CAN_JOG_MISSING_OK , _CMD_JOG_MISSING_OK , _DIR_NAME_NOT_FOUND , _BUG_UPDATE_FILE_BTN ,
    _CARD_MOUNT_FAILED , _ROOT_NOT_FOUND , _CHDIR_ERROR, _FAILED_TO_OPEN_ROOT , _FIRST_DIR_IS_NOT_ROOT ,
    _CURRENT_DIR_IS_NOT_A_SUB_DIR , _FILES_MISSING , _FAILED_TO_OPEN_A_FILE , _NO_FILE_NAME , _SELECTED_FILE_MISSING,
    _FILE_NAME_NOT_FOUND , _CMD_DELETED, _CMD_NOT_CREATED , _CMD_CREATED, _CMD_PART_NOT_READ,
    _CMD_COULD_NOT_SAVE , _NO_NUNCHUK, _MISSING_OK_WHEN_SENDING_FROM_SD, _NOT_IDLE, _CHANGING_FEEDRATE1,
    _CHANGING_FEEDRATE2, _CHANGING_SPINDLE1, _CHANGING_SPINDLE2, _SETX_EXECUTED, _SETY_EXECUTED ,
    _SETZ_EXECUTED  , _SETA_EXECUTED , _SETXYZ_EXECUTED , _SETXYZA_EXECUTED, _SET_CHANGE_EXECUTED ,
    _SET_PROBE_EXECUTED , _SET_CAL_EXECUTED, _GO_CHANGE_EXECUTED, _GO_PROBE_EXECUTED, _UNKNOWN_BTN_EXECUTED ,
    _UNABLE_TO_CONNECT_IN_BT, _BT_DISCONNECTED, _BT_CONNECTED_WITH_GRBL , _GRBL_SERIAL_CONNECTED ,_UNABLE_TO_CONNECT_TO_GRBL_TELNET ,
    _TELNET_CONNECTED_WITH_GRBL, _CURRENT_GRBL_DIR , _READING_FILES_ON_GRBL ,  _PLEASE_WAIT ,_NO_IP_ASSIGNED ,
    _USES_BT_TO_GRBL, _USES_TELNET_TO_GRBL ,_TRY_TO_CONNECT_WITH_BT , _WAIT_1_MIN , _WIFI_NOT_CONNECTED ,
    _TRY_TO_CONNECT_WITH_TELNET, _WIFI_CONNECTED ,_YES_TXT, _NO_TXT, _CONFIRM_SD_FILE , 
    _MAX_TEXT } ; // keep _MAX_TEXt latest



// Liste des GrblErrors définis ; 5 par lignes
enum { _UNKNOWN_ERROR= 0, _EXPECTED_CMD_LETTER, _BAD_NUMBER_FORMAT, _INVALID_$_SYSTEM_CMD, _NEGATIVE_VALUE,
    _HOMING_NOT_ENABLED, _STEP_PULSE_LESS_3_USEC, _EEPROM_READ_FAIL, _$_WHILE_NOT_IDLE, _LOCKED_ALARM_OR_JOG,
    _SOFT_LIMIT_NO_HOMING, _LINE_OVERFLOW, _STEP_RATE_TO_HIGH, _SAFETY_DOOR_DETECTED, _LINE_LENGHT_EXCEEDED,
    _JOG_TRAVEL_EXCEEDED, _INVALID_JOG_COMMAND, _LASER_REQUIRES_PWM, _NO_HOMING_DEFINED, _UNKNOWN_ERROR_19,
    _UNSUPPORTED_COMMAND, _MODAL_GROUP_VIOLATION, _UNDEF_FEED_RATE, _CMD_REQUIRES_INTEGER, _SEVERAL_AXIS_GCODE,
    _REPEATED_GCODE, _AXIS_MISSING_IN_GCODE, _INVALID_LINE_NUMBER, _VALUE_MISSING_IN_GCODE, _G59_WCS_NOT_SUPPORTED,
    _G53_WITHOUT_G01_AND_G1, _AXIS_NOT_ALLOWED, _G2_G3_REQUIRE_A_PLANE, _INVALID_MOTION_TARGET, _INVALID_ARC_RADIUS,
    _G2_G3_REQUIRE_OFFSET, _UNSUSED_VALUE, _G431_TOOL_LENGTH, _TOOL_NUMBER_EXCEED_MAX , _PARAMETER_P_EXCEED_MAX,
    _SD_FAILED_TO_MOUNT, _SD_FAILED_FOR_READING , _SD_FAILED_FOR_DIRECTORY, _SD_DIR_NOT_FOUND, _SD_FILE_EMPTY ,
    _RESERVE_65, _RESERVE_66, _RESERVE_67, _RESERVE_68, _RESERVE_69, 
    _BT_FAILED, _MAX_GRBL_ERRORS } ; // keep _MAX_GRBL_ERRORS latest

// Liste des Alarms définies              
enum { _UNKNOWN_ALARM = 0 , _HARD_LIMIT_REACHED, _MOTION_EXCEED_CNC, _RESET_IN_MOTION, _PROBE_INIT_FAIL,
    _PROBE_TRAVEL_FAIL, _RESET_DURING_HOMING, _DOOR_OPEN_HOMING, _LIMIT_ON_HOMING, _LIMIT_MISSING_HOMING,
    _MAX_ALARMS } ; // keep _MAX_ALARMS latest

#ifdef AA_AXIS
#define POS_OF_MOVE_D_AUTO 10  // sequence number of definition D_AUTO on MOVE (in range 0...11)
#else
#define POS_OF_MOVE_D_AUTO 9  // sequence number of definition D_AUTO on MOVE (in range 0...11)
#endif
#define POS_OF_SD_SHOW_PG_NEXT 7
#define POS_OF_SD_SHOW_PG_PREV 3
#define POS_OF_LOG_PG_NEXT 7
#define POS_OF_LOG_PG_PREV 3
#define POS_OF_OVERWRITE_OVERWRITE 3

          

#endif
