
#include "setupTxt.h"
#include "config.h"
#include "language.h"
#include "SdFat.h"
#include "FS.h"
#include "draw.h"
#include "actions.h"
#include "icons.h"

M_Button mButton[_MAX_BTN] ;
M_Page mPages[_P_MAX_PAGES];
M_pLabel mText[_MAX_TEXT];
M_pLabel mGrblErrors[_MAX_GRBL_ERRORS] ;
M_pLabel mAlarms[_MAX_ALARMS]; 

extern SdFat32 sd;

extern char cmdName[11][17] ;
extern char fileNames[4][23] ; // 22 car per line + "\0"
extern char grblFileNamesTft[4][40]; // contains only the 4 names to be displayed on TFT (needed because name is altered during btn drawing 

// rempli le paramétrage d'un boutons d'une page 
void fillMPage (uint8_t _page , uint8_t _btnPos , uint8_t _boutons, uint8_t _actions , void (*_pfNext)(uint8_t) , uint8_t _parameters ) {
  mPages[_page].boutons[_btnPos] =  _boutons ;
  mPages[_page].actions[_btnPos] =  _actions ;
  mPages[_page].pfNext[_btnPos] =  _pfNext ;
  mPages[_page].parameters[_btnPos] =  _parameters ;
}

void initButtons() {  
mButton[_SETUP].pLabel = __SETUP  ;
mButton[_PRINT].pLabel = __PRINT  ;
mButton[_HOME].pLabel = __HOME  ;
mButton[_UNLOCK].pLabel = __UNLOCK  ;
mButton[_RESET].pLabel = __RESET  ;
mButton[_SD].pLabel = __SD  ;
mButton[_USB_GRBL].pLabel = __USB_GRBL  ;
mButton[_TELNET_GRBL].pLabel = __TELNET_GRBL  ;
mButton[_PAUSE].pLabel = __PAUSE  ;
mButton[_CANCEL].pLabel = __CANCEL  ;
mButton[_INFO].pLabel = __INFO  ;
mButton[_CMD].pLabel = __CMD  ;
mButton[_MOVE].pLabel = __MOVE  ;
mButton[_RESUME].pLabel = __RESUME  ;
mButton[_STOP_PC_GRBL].pLabel = __STOP_PC_GRBL ;
mButton[_XP].pLabel = "X+" ;
mButton[_XM].pLabel = "X-" ;
mButton[_YP].pLabel = "Y+" ;
mButton[_YM].pLabel = "Y-" ;
mButton[_ZP].pLabel = "Z+" ;
mButton[_ZM].pLabel = "Z-" ;
mButton[_AP].pLabel = "A+" ;
mButton[_AM].pLabel = "A-" ;
mButton[_D_AUTO].pLabel = __D_AUTO  ;
mButton[_D0_01].pLabel = "0.01" ;
mButton[_D0_1].pLabel = "0.1" ;
mButton[_D1].pLabel = "1" ;
mButton[_D10].pLabel = "10" ;
mButton[_SET_WCS].pLabel = __SET_WCS  ;
mButton[_SETX].pLabel = __SETX  ;
mButton[_SETY].pLabel = __SETY  ;
mButton[_SETZ].pLabel = __SETZ  ;
mButton[_SETA].pLabel = __SETA  ;
mButton[_SETXYZ].pLabel = __SETXYZ  ;
mButton[_SETXYZA].pLabel = __SETXYZA  ;
mButton[_TOOL].pLabel = __TOOL  ;
mButton[_SET_CHANGE].pLabel = __SET_CHANGE  ;
mButton[_SET_PROBE].pLabel = __SET_PROBE  ;
mButton[_SET_CAL].pLabel = __SET_CAL  ;
mButton[_GO_CHANGE].pLabel = __GO_CHANGE ; 
mButton[_GO_PROBE].pLabel = __GO_PROBE ;
mButton[_BACK].pLabel = __BACK  ;
mButton[_LEFT].pLabel = __LEFT  ;
mButton[_RIGHT].pLabel = __RIGHT  ;
mButton[_UP].pLabel = __UP  ;
mButton[_CMD1].pLabel = &cmdName[0][0] ;
mButton[_CMD2].pLabel = &cmdName[1][0] ;
mButton[_CMD3].pLabel = &cmdName[2][0] ;
mButton[_CMD4].pLabel = &cmdName[3][0] ;
mButton[_CMD5].pLabel = &cmdName[4][0] ;
mButton[_CMD6].pLabel = &cmdName[5][0] ;
mButton[_CMD7].pLabel = &cmdName[6][0] ;
mButton[_CMD8].pLabel = &cmdName[7][0] ;
mButton[_CMD9].pLabel = &cmdName[8][0] ;
mButton[_CMD10].pLabel = &cmdName[9][0] ;
mButton[_CMD11].pLabel = &cmdName[10][0] ;
mButton[_MORE_PAUSE].pLabel = __MORE_PAUSE ;
mButton[_FILE0].pLabel = fileNames[0] ;  // labels are defined during execution in a table
mButton[_FILE1].pLabel = fileNames[1] ;
mButton[_FILE2].pLabel = fileNames[2] ;
mButton[_FILE3].pLabel = fileNames[3] ;
mButton[_MASKED1].pLabel = "" ; // this is a hidden button; so must be empty
mButton[_PG_PREV].pLabel = __PG_PREV ;
mButton[_PG_NEXT].pLabel = __PG_NEXT ;
mButton[_SD_SHOW].pLabel = __SD_SHOW ; 
mButton[_OVERWRITE].pLabel = "" ; // this is a hidden button; so must be empty
mButton[_OVER_SWITCH_TO_FEEDRATE].pLabel = __OVER_SWITCH_TO_FEEDRATE ;
mButton[_OVER_SWITCH_TO_SPINDLE].pLabel = __OVER_SWITCH_TO_SPINDLE ;
mButton[_OVER_10P].pLabel = __OVER_10P ;
mButton[_OVER_10M].pLabel = __OVER_10M ;
mButton[_OVER_1P].pLabel = __OVER_1P ;
mButton[_OVER_1M].pLabel = __OVER_1M ;
mButton[_OVER_100].pLabel = __OVER_100 ;
mButton[_COMMUNICATION].pLabel = __COMMUNICATION ;
mButton[_SERIAL].pLabel = __SERIAL ;
mButton[_BLUETOOTH].pLabel = __BLUETOOTH ;
mButton[_TELNET].pLabel = __TELNET ;
mButton[_SD_GRBL].pLabel = __SD_GRBL ;
mButton[_FILE0_GRBL].pLabel = grblFileNamesTft[0] ;
mButton[_FILE1_GRBL].pLabel = grblFileNamesTft[1] ;
mButton[_FILE2_GRBL].pLabel = grblFileNamesTft[2] ;
mButton[_FILE3_GRBL].pLabel = grblFileNamesTft[3] ; 
mButton[_YES_BTN].pLabel = __YES ;
mButton[_NO_BTN].pLabel = __NO ;

#ifdef USE_ICONS
mButton[_SETUP].pIcon = setupIcon  ;
mButton[_PRINT].pIcon = printIcon  ;
mButton[_HOME].pIcon = homeIcon  ;
mButton[_UNLOCK].pIcon = unlockIcon  ;
mButton[_RESET].pIcon = resetIcon ;
mButton[_SD].pIcon = sdIcon  ;
mButton[_USB_GRBL].pIcon = usbIcon  ;
mButton[_TELNET_GRBL].pIcon = telnetIcon  ;
mButton[_PAUSE].pIcon = pauseIcon  ;
mButton[_CANCEL].pIcon = cancelIcon  ;
mButton[_INFO].pIcon = infoIcon  ;
mButton[_CMD].pIcon = cmdIcon  ;
mButton[_MOVE].pIcon = moveIcon  ;
mButton[_RESUME].pIcon = resumeIcon  ;
mButton[_STOP_PC_GRBL].pIcon = stopIcon ;
mButton[_XP].pIcon = xpIcon ;
mButton[_XM].pIcon = xmIcon ;
mButton[_YP].pIcon = ypIcon ;
mButton[_YM].pIcon = ymIcon ;
mButton[_ZP].pIcon = zpIcon ;
mButton[_ZM].pIcon = zmIcon ;
//mButton[_AP].pIcon = "A+" ;
//mButton[_AM].pIcon = "A-" ;
mButton[_D_AUTO].pIcon = dAutoIcon  ;
mButton[_D0_01].pIcon = d0_01Icon ;
mButton[_D0_1].pIcon = d0_1Icon ;
mButton[_D1].pIcon = d1Icon ;
mButton[_D10].pIcon = d10Icon ;
mButton[_SET_WCS].pIcon = setWCSIcon  ;
mButton[_SETX].pIcon = setXIcon ;
mButton[_SETY].pIcon = setYIcon ;
mButton[_SETZ].pIcon = setZIcon ;
//mButton[_SETA].pIcon = __SETA  ;
mButton[_SETXYZ].pIcon = setXYZIcon  ;
//mButton[_SETXYZA].pIcon = __SETXYZA  ;
mButton[_TOOL].pIcon = toolIcon  ;
//mButton[_SET_CHANGE].pIcon = __SET_CHANGE  ;
//mButton[_SET_PROBE].pIcon = __SET_PROBE  ;
//mButton[_SET_CAL].pIcon = __SET_CAL  ;
//mButton[_GO_CHANGE].pIcon = __GO_CHANGE ; 
//mButton[_GO_PROBE].pIcon = __GO_PROBE ;
mButton[_BACK].pIcon = backIcon  ;
//mButton[_LEFT].pIcon = leftIcon  ;
//mButton[_RIGHT].pIcon = rightIcon  ;
mButton[_UP].pIcon = upIcon  ;
//mButton[_CMD1].pLabel = &cmdName[0][0] ;
//mButton[_CMD2].pLabel = &cmdName[1][0] ;
//mButton[_CMD3].pLabel = &cmdName[2][0] ;
//mButton[_CMD4].pLabel = &cmdName[3][0] ;
//mButton[_CMD5].pLabel = &cmdName[4][0] ;
//mButton[_CMD6].pLabel = &cmdName[5][0] ;
//mButton[_CMD7].pLabel = &cmdName[6][0] ;
//mButton[_CMD8].pLabel = &cmdName[7][0] ;
//mButton[_CMD9].pLabel = &cmdName[8][0] ;
//mButton[_CMD10].pLabel = &cmdName[9][0] ;
//mButton[_CMD11].pLabel = &cmdName[10][0] ;
mButton[_MORE_PAUSE].pIcon = morePauseIcon ;
//mButton[_FILE0].pLabel = fileNames[0] ;  // labels are defined during execution in a table
//mButton[_FILE1].pLabel = fileNames[1] ;
//mButton[_FILE2].pLabel = fileNames[2] ;
//mButton[_FILE3].pLabel = fileNames[3] ;
//mButton[_MASKED1].pLabel = "" ; // this is a hidden button; so must be empty
mButton[_PG_PREV].pIcon = pgPrevIcon ;
mButton[_PG_NEXT].pIcon = pgNextIcon ;
mButton[_SD_SHOW].pIcon = sdShowIcon ; 
mButton[_OVERWRITE].pLabel = __OVERWRITE ; 
//mButton[_OVER_SWITCH_TO_FEEDRATE].pLabel = __OVER_SWITCH_TO_FEEDRATE ;
//mButton[_OVER_SWITCH_TO_SPINDLE].pLabel = __OVER_SWITCH_TO_SPINDLE ;
//mButton[_OVER_10P].pLabel = __OVER_10P ;
//mButton[_OVER_10M].pLabel = __OVER_10M ;
//mButton[_OVER_1P].pLabel = __OVER_1P ;
//mButton[_OVER_1M].pLabel = __OVER_1M ;
//mButton[_OVER_100].pLabel = __OVER_100 ;
//mButton[_COMMUNICATION].pLabel = __COMMUNICATION ;
//mButton[_SERIAL].pLabel = __SERIAL ;
//mButton[_BLUETOOTH].pLabel = __BLUETOOTH ;
//mButton[_TELNET].pLabel = __TELNET ;
//mButton[_SD_GRBL].pLabel = __SD_GRBL ;
//mButton[_FILE0_GRBL].pLabel = grblFileNamesTft[0] ;
//mButton[_FILE1_GRBL].pLabel = grblFileNamesTft[1] ;
//mButton[_FILE2_GRBL].pLabel = grblFileNamesTft[2] ;
//mButton[_FILE3_GRBL].pLabel = grblFileNamesTft[3] ;
mButton[_LOG].pLabel = __LOG ;

#endif //end of USE_ICONS



// ************** fill the parameters to declare the buttons for each pages
mPages[_P_INFO].titel = "" ;
mPages[_P_INFO].pfBase = fInfoBase ;
fillMPage (_P_INFO , 0 , _MASKED1 , _JUST_PRESSED , fGoToPage , _P_LOG ) ; // this button is masked but clicking on the zone call another screen
fillMPage (_P_INFO , 3 , _MASKED1 , _JUST_PRESSED , fGoToPage , _P_OVERWRITE ) ; // this button is masked but clicking on the zone call another screen
fillMPage (_P_INFO , 7 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;   // those buttons are changed dynamically based on status (no print, ...)
fillMPage (_P_INFO , 11 , _PRINT , _JUST_PRESSED , fGoToPage , _P_PRINT) ;   // those buttons are changed dynamically based on status (no print, ...)

mPages[_P_SETUP].titel = "" ;
mPages[_P_SETUP].pfBase = fSetupBase ;
fillMPage (_P_SETUP , 0 , _UNLOCK , _JUST_PRESSED , fUnlock , 0) ;
//fillMPage (_P_SETUP , 3 , _OVERWRITE , _JUST_PRESSED , fGoToPage , _P_OVERWRITE ) ; 
fillMPage (_P_SETUP , 4 , _HOME , _JUST_PRESSED , fHome , 0) ;
fillMPage (_P_SETUP , 5 , _RESET , _JUST_PRESSED , fReset , 0) ;
fillMPage (_P_SETUP , 6 , _CMD , _JUST_PRESSED , fGoToPage , _P_CMD ) ;
fillMPage (_P_SETUP , 7 , _MOVE , _JUST_PRESSED , fGoToPage , _P_MOVE ) ;
fillMPage (_P_SETUP , 8 , _SET_WCS , _JUST_PRESSED , fGoToPage , _P_SETXYZ ) ;
fillMPage (_P_SETUP , 9 , _TOOL , _JUST_PRESSED , fGoToPage , _P_TOOL ) ;
fillMPage (_P_SETUP , 10 , _COMMUNICATION , _JUST_PRESSED , fGoToPage , _P_COMMUNICATION ) ;
fillMPage (_P_SETUP , 11 , _INFO , _JUST_PRESSED , fGoToPageAndClearMsg ,  _P_INFO) ;

mPages[_P_PRINT].titel = "" ;
mPages[_P_PRINT].pfBase = fNoBase ;
fillMPage (_P_PRINT , 4 , _SD , _JUST_PRESSED , fGoToPage , _P_SD) ;
fillMPage (_P_PRINT , 5 , _USB_GRBL , _JUST_PRESSED , fStartUsb , 0) ;
fillMPage (_P_PRINT , 6 , _TELNET_GRBL , _JUST_PRESSED , fStartTelnet , 0) ;
fillMPage (_P_PRINT , 7 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP) ;
#if defined(TFT_CARD_VERSION) && (TFT_CARD_VERSION == 2)
fillMPage (_P_PRINT , 8 , _SD_GRBL , _JUST_PRESSED , fGoToPage , _P_SD_GRBL_WAIT) ;
#endif
fillMPage (_P_PRINT , 10 , _CMD , _JUST_PRESSED , fGoToPage , _P_CMD ) ;
fillMPage (_P_PRINT , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO) ;

mPages[_P_PAUSE].titel = "" ;
mPages[_P_PAUSE].pfBase = fNoBase ;
fillMPage (_P_PAUSE , 4 , _CANCEL , _JUST_PRESSED , fCancel , 0) ;
//fillMPage (_P_PAUSE , 5 , _RESUME , _JUST_PRESSED , fResume , 0) ;
fillMPage (_P_PAUSE , 6 , _SD_SHOW , _JUST_PRESSED , fGoToPage , _P_SD_SHOW ) ;
fillMPage (_P_PAUSE , 7 , _OVERWRITE , _JUST_PRESSED , fGoToPage , _P_OVERWRITE ) ; 
fillMPage (_P_PAUSE , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO) ;

mPages[_P_MOVE].titel = "" ;
mPages[_P_MOVE].pfBase = fMoveBase ;
#ifdef AA_AXIS
fillMPage (_P_MOVE , 0 , _XM , _JUST_LONG_PRESSED_RELEASED , fMove , _XM) ;
fillMPage (_P_MOVE , 1 , _YP , _JUST_LONG_PRESSED_RELEASED , fMove , _YP) ;
fillMPage (_P_MOVE , 2 , _XP , _JUST_LONG_PRESSED_RELEASED , fMove , _XP) ;
fillMPage (_P_MOVE , 3 , _ZP , _JUST_LONG_PRESSED_RELEASED , fMove , _ZP) ;
fillMPage (_P_MOVE , 4 , _AM , _JUST_LONG_PRESSED_RELEASED , fMove , _AM) ;
fillMPage (_P_MOVE , 5 , _YM , _JUST_LONG_PRESSED_RELEASED , fMove , _YM) ;
fillMPage (_P_MOVE , 6 , _AP , _JUST_LONG_PRESSED_RELEASED , fMove , _AP) ;
fillMPage (_P_MOVE , 7 , _ZM , _JUST_LONG_PRESSED_RELEASED , fMove , _ZM) ;
#else
fillMPage (_P_MOVE , 1 , _YP , _JUST_LONG_PRESSED_RELEASED , fMove , _YP) ;
fillMPage (_P_MOVE , 3 , _ZP , _JUST_LONG_PRESSED_RELEASED , fMove , _ZP) ;
fillMPage (_P_MOVE , 5 , _YM , _JUST_LONG_PRESSED_RELEASED , fMove , _YM) ;
fillMPage (_P_MOVE , 7 , _ZM , _JUST_LONG_PRESSED_RELEASED , fMove , _ZM) ;
fillMPage (_P_MOVE , 8 , _XM , _JUST_LONG_PRESSED_RELEASED , fMove , _XM) ;
fillMPage (_P_MOVE , 10 , _XP , _JUST_LONG_PRESSED_RELEASED , fMove , _XP) ;
#endif
fillMPage (_P_MOVE , POS_OF_MOVE_D_AUTO , _D_AUTO , _JUST_PRESSED , fDist, _D_AUTO) ;  // -1 because range here is 0...11 
fillMPage (_P_MOVE , 11 , _BACK , _JUST_PRESSED , fGoBack , 0) ;

mPages[_P_SETXYZ].titel = "" ;  
mPages[_P_SETXYZ].pfBase = fSetXYZBase ;
fillMPage (_P_SETXYZ , 4 , _SETX , _JUST_PRESSED , fSetXYZ , _SETX) ;
fillMPage (_P_SETXYZ , 5 , _SETY , _JUST_PRESSED , fSetXYZ , _SETY) ;
fillMPage (_P_SETXYZ , 6 , _SETZ, _JUST_PRESSED , fSetXYZ , _SETZ) ;
#ifdef AA_AXIS
fillMPage (_P_SETXYZ , 7 , _SETA, _JUST_PRESSED , fSetXYZ , _SETA) ;
fillMPage (_P_SETXYZ , 8 , _SETXYZ , _JUST_PRESSED , fSetXYZ , _SETXYZ) ;
fillMPage (_P_SETXYZ , 9 , _SETXYZA , _JUST_PRESSED , fSetXYZ , _SETXYZA) ;
#else
fillMPage (_P_SETXYZ , 8 , _SETXYZ , _JUST_PRESSED , fSetXYZ , _SETXYZ) ;
#endif
fillMPage (_P_SETXYZ , 10 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP ) ;
fillMPage (_P_SETXYZ , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_SD].titel = "" ;  // this screen has only 10 buttons instead of 12
mPages[_P_SD].pfBase = fSdBase ;   // cette fonction doit remplir les 4 premiers boutons en fonction des fichiers disponibles
fillMPage (_P_SD , 6 , _PG_PREV , _JUST_PRESSED , fSdMove , _PG_PREV ) ;
fillMPage (_P_SD , 7 , _UP , _JUST_PRESSED , fSdMove , _UP ) ;
fillMPage (_P_SD , 8 , _PG_NEXT , _JUST_PRESSED , fSdMove , _PG_NEXT) ;
fillMPage (_P_SD , 9 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_CMD].titel = "" ;
mPages[_P_CMD].pfBase = fCmdBase ; // 
if (cmdName[0][0] ) fillMPage (_P_CMD , 0 , _CMD1 , _JUST_PRESSED , fCmd , _CMD1) ; // le paramètre contient le n° du bouton
if (cmdName[1][0] ) fillMPage (_P_CMD , 1 , _CMD2 , _JUST_PRESSED , fCmd , _CMD2) ; // le paramètre contient le n° du bouton
if (cmdName[2][0] ) fillMPage (_P_CMD , 2 , _CMD3 , _JUST_PRESSED , fCmd , _CMD3) ; // le paramètre contient le n° du bouton
if (cmdName[3][0] ) fillMPage (_P_CMD , 3 , _CMD4 , _JUST_PRESSED , fCmd , _CMD4) ; // le paramètre contient le n° du bouton
if (cmdName[4][0] ) fillMPage (_P_CMD , 4 , _CMD5 , _JUST_PRESSED , fCmd , _CMD5) ; // le paramètre contient le n° du bouton
if (cmdName[5][0] ) fillMPage (_P_CMD , 5 , _CMD6 , _JUST_PRESSED , fCmd , _CMD6) ; // le paramètre contient le n° du bouton
if (cmdName[6][0] ) fillMPage (_P_CMD , 6 , _CMD7 , _JUST_PRESSED , fCmd , _CMD7) ; // le paramètre contient le n° du bouton
if (cmdName[7][0] ) fillMPage (_P_CMD , 7 , _CMD8 , _JUST_PRESSED , fCmd , _CMD8) ; // le paramètre contient le n° du bouton
if (cmdName[8][0] ) fillMPage (_P_CMD , 8 , _CMD9 , _JUST_PRESSED , fCmd , _CMD9) ; // le paramètre contient le n° du bouton
if (cmdName[9][0] ) fillMPage (_P_CMD , 9 , _CMD10 , _JUST_PRESSED , fCmd , _CMD10) ; // le paramètre contient le n° du bouton
if (cmdName[10][0] ) fillMPage (_P_CMD , 10 , _CMD11 , _JUST_PRESSED , fCmd , _CMD11) ; // le paramètre contient le n° du bouton
fillMPage (_P_CMD , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_LOG].titel = "" ;
mPages[_P_LOG].pfBase = fLogBase ;
fillMPage (_P_LOG , POS_OF_LOG_PG_PREV , _PG_PREV , _JUST_PRESSED , fLogPrev , 0) ;
fillMPage (_P_LOG , POS_OF_LOG_PG_NEXT , _PG_NEXT , _JUST_PRESSED , fLogNext , 0) ;
fillMPage (_P_LOG , 11 , _BACK , _JUST_PRESSED , fGoBack , 0) ;

mPages[_P_TOOL].titel = "" ;
mPages[_P_TOOL].pfBase = fToolBase ;
fillMPage (_P_TOOL , 4 , _SETXYZ , _JUST_PRESSED , fSetXYZ , _SETXYZ) ;
fillMPage (_P_TOOL , 5 , _SET_CAL , _JUST_PRESSED , fSetXYZ , _SET_CAL) ;
fillMPage (_P_TOOL , 6 , _GO_CHANGE , _JUST_PRESSED , fSetXYZ , _GO_CHANGE) ;
fillMPage (_P_TOOL , 7 , _GO_PROBE , _JUST_PRESSED , fSetXYZ , _GO_PROBE) ;
fillMPage (_P_TOOL , 8 , _SET_CHANGE , _JUST_PRESSED , fSetXYZ , _SET_CHANGE ) ;
fillMPage (_P_TOOL , 9 , _SET_PROBE , _JUST_PRESSED , fSetXYZ , _SET_PROBE) ;
fillMPage (_P_TOOL , 10 , _SETUP , _JUST_PRESSED , fGoToPage , _P_SETUP ) ;
fillMPage (_P_TOOL , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_SD_SHOW].titel = "" ;
mPages[_P_SD_SHOW].pfBase = fSdShowBase ;
fillMPage (_P_SD_SHOW , POS_OF_SD_SHOW_PG_PREV , _PG_PREV , _JUST_PRESSED , fSdShowPrev , 0) ;
fillMPage (_P_SD_SHOW , POS_OF_SD_SHOW_PG_NEXT , _PG_NEXT , _JUST_PRESSED , fSdShowNext , 0) ;
fillMPage (_P_SD_SHOW , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;


mPages[_P_OVERWRITE].titel = "" ;
mPages[_P_OVERWRITE].pfBase = fOverBase ;
fillMPage (_P_OVERWRITE , POS_OF_OVERWRITE_OVERWRITE , _OVER_SWITCH_TO_SPINDLE , _JUST_PRESSED , fOverSwitch , _OVER_SWITCH_TO_SPINDLE ) ;
fillMPage (_P_OVERWRITE , 4 , _OVER_10M , _JUST_PRESSED , fOverModify , _OVER_10M) ;
fillMPage (_P_OVERWRITE , 5 , _OVER_1M , _JUST_PRESSED , fOverModify , _OVER_1M) ;
fillMPage (_P_OVERWRITE , 6 , _OVER_1P , _JUST_PRESSED , fOverModify , _OVER_1P) ;
fillMPage (_P_OVERWRITE , 7 , _OVER_10P , _JUST_PRESSED , fOverModify , _OVER_10P) ;
fillMPage (_P_OVERWRITE , 10 , _OVER_100 , _JUST_PRESSED , fOverModify , _OVER_100) ;
fillMPage (_P_OVERWRITE , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_COMMUNICATION].titel = "" ;
mPages[_P_COMMUNICATION].pfBase = fCommunicationBase ;
fillMPage (_P_COMMUNICATION , 0 , _LOG , _JUST_PRESSED , fGoToPage , _P_LOG ) ; 
#if defined(TFT_CARD_VERSION) && (TFT_CARD_VERSION == 2)
fillMPage (_P_COMMUNICATION , 8 , _SERIAL , _JUST_PRESSED , fSerial , 0) ;
fillMPage (_P_COMMUNICATION , 9 , _BLUETOOTH , _JUST_PRESSED , fBluetooth , 0) ;
fillMPage (_P_COMMUNICATION , 10 , _TELNET , _JUST_PRESSED , fTelnet , 0) ;
#endif
fillMPage (_P_COMMUNICATION , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_SD_GRBL_WAIT].titel = "" ;  // this screen has only 10 buttons instead of 12
mPages[_P_SD_GRBL_WAIT].pfBase = fSdGrblWaitBase ;   // cette fonction doit provoque l'envoi d'une commande à GRBL

mPages[_P_SD_GRBL].titel = "" ;  // this screen has only 10 buttons instead of 12
mPages[_P_SD_GRBL].pfBase = fSdGrblBase ;   // cette fonction doit remplir les 4 premiers boutons en fonction des fichiers disponibles
fillMPage (_P_SD_GRBL , 6 , _PG_PREV , _JUST_PRESSED , fSdGrblMove , _PG_PREV ) ;
fillMPage (_P_SD_GRBL , 7 , _UP , _JUST_PRESSED , fSdGrblMove , _UP ) ;
fillMPage (_P_SD_GRBL , 8 , _PG_NEXT , _JUST_PRESSED , fSdGrblMove , _PG_NEXT) ;
fillMPage (_P_SD_GRBL , 9 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO ) ;

mPages[_P_SD_CONFIRM].titel = "" ;  // this screen has only 2 buttons 9 and 10
mPages[_P_SD_CONFIRM].pfBase = fConfirmYesNoBase ;   // cette fonction doit afficher le texte (et idéalement le nom du message)
fillMPage (_P_SD_CONFIRM , 9 , _YES_BTN , _JUST_PRESSED , fConfirmedYes , 0 ) ;
fillMPage (_P_SD_CONFIRM , 10 , _NO_BTN , _JUST_PRESSED , fConfirmedNo , 0 ) ;

mPages[_P_PAUSE_GRBL].titel = "" ;
mPages[_P_PAUSE_GRBL].pfBase = fNoBase ;
fillMPage (_P_PAUSE_GRBL , 4 , _CANCEL , _JUST_PRESSED , fCancelGrbl , 0) ;
//fillMPage (_P_PAUSE_GRBL , 5 , _RESUME , _JUST_PRESSED , fResumeGrbl , 0) ;
fillMPage (_P_PAUSE_GRBL , 7 , _OVERWRITE , _JUST_PRESSED , fGoToPage , _P_OVERWRITE ) ; 
fillMPage (_P_PAUSE_GRBL , 11 , _INFO , _JUST_PRESSED , fGoToPage , _P_INFO) ;


// ************************** Fill the messages ****************************
mText[_WPOS].pLabel = __WPOS ;
mText[_MPOS].pLabel = __MPOS ;
mText[_FEED].pLabel = __FEED ;
mText[_RPM].pLabel = __RPM ;
mText[_TOUCH_CORNER].pLabel = __TOUCH_CORNER ;
mText[_SET_REPEAT_CAL].pLabel = __SET_REPEAT_CAL ;
mText[_CAL_COMPLETED].pLabel = __CAL_COMPLETED ;
mText[_SPIFFS_FORMATTED].pLabel = __SPIFFS_FORMATTED ;
mText[_CMD_NOT_LOADED].pLabel = __CMD_NOT_LOADED ;
mText[_TELENET_DISCONNECTED].pLabel = __TELENET_DISCONNECTED ;
mText[_INVALID_BTN_HOME].pLabel = __INVALID_BTN_HOME ;
mText[_CMD_NOT_RETRIEVED].pLabel = __CMD_NOT_RETRIEVED ;
mText[_NO_TELNET_CONNECTION].pLabel = __NO_TELNET_CONNECTION ;
mText[_WIFI_NOT_FOUND].pLabel = __WIFI_NOT_FOUND ;
mText[_SPIFFS_FAIL_TO_OPEN].pLabel = __SPIFFS_FAIL_TO_OPEN ;
mText[_FAILED_TO_CREATE_CMD].pLabel = __FAILED_TO_CREATE_CMD ;
mText[_CAN_JOG_MISSING_OK].pLabel = __CAN_JOG_MISSING_OK ;
mText[_CMD_JOG_MISSING_OK].pLabel = __CMD_JOG_MISSING_OK ;
mText[_DIR_NAME_NOT_FOUND].pLabel = __DIR_NAME_NOT_FOUND ;
mText[_BUG_UPDATE_FILE_BTN].pLabel = __BUG_UPDATE_FILE_BTN ;
mText[_CARD_MOUNT_FAILED].pLabel = __CARD_MOUNT_FAILED ;
mText[_ROOT_NOT_FOUND].pLabel = __ROOT_NOT_FOUND ;
mText[_CHDIR_ERROR].pLabel = __CHDIR_ERROR ;
mText[_FAILED_TO_OPEN_ROOT].pLabel = __FAILED_TO_OPEN_ROOT ;
mText[_FIRST_DIR_IS_NOT_ROOT].pLabel = __FIRST_DIR_IS_NOT_ROOT ;
mText[_CURRENT_DIR_IS_NOT_A_SUB_DIR].pLabel = __CURRENT_DIR_IS_NOT_A_SUB_DIR ;
mText[_FILES_MISSING].pLabel = __FILES_MISSING ;
mText[_FAILED_TO_OPEN_A_FILE].pLabel = __FAILED_TO_OPEN_A_FILE ;
mText[_NO_FILE_NAME].pLabel = __NO_FILE_NAME ;
mText[_SELECTED_FILE_MISSING].pLabel = __SELECTED_FILE_MISSING ;
mText[_FILE_NAME_NOT_FOUND].pLabel = __FILE_NAME_NOT_FOUND ;
mText[_CMD_DELETED].pLabel = __CMD_DELETED ;
mText[_CMD_NOT_CREATED].pLabel = __CMD_NOT_CREATED ;
mText[_CMD_CREATED].pLabel = __CMD_CREATED ;
mText[_CMD_PART_NOT_READ].pLabel = __CMD_PART_NOT_READ ;
mText[_CMD_COULD_NOT_SAVE].pLabel = __CMD_COULD_NOT_SAVE ;
mText[_NO_NUNCHUK].pLabel = __NO_NUNCHUK ;
mText[_MISSING_OK_WHEN_SENDING_FROM_SD].pLabel = __MISSING_OK_WHEN_SENDING_FROM_SD ;
mText[_NOT_IDLE].pLabel = __NOT_IDLE ;
mText[_CHANGING_FEEDRATE1].pLabel = __CHANGING_FEEDRATE1 ;
mText[_CHANGING_FEEDRATE2].pLabel = __CHANGING_FEEDRATE2 ;
mText[_CHANGING_SPINDLE1].pLabel = __CHANGING_SPINDLE1 ;
mText[_CHANGING_SPINDLE2].pLabel = __CHANGING_SPINDLE2 ;
mText[_SETX_EXECUTED].pLabel = __SETX_EXECUTED ;
mText[_SETY_EXECUTED].pLabel = __SETY_EXECUTED ;
mText[_SETZ_EXECUTED].pLabel = __SETZ_EXECUTED ;
mText[_SETA_EXECUTED].pLabel = __SETA_EXECUTED ;
mText[_SETXYZ_EXECUTED].pLabel = __SETXYZ_EXECUTED ;
mText[_SETXYZA_EXECUTED].pLabel = __SETXYZA_EXECUTED ;
mText[_SET_CHANGE_EXECUTED].pLabel = __SET_CHANGE_EXECUTED ;
mText[_SET_PROBE_EXECUTED].pLabel = __SET_PROBE_EXECUTED ;
mText[_SET_CAL_EXECUTED].pLabel = __SET_CAL_EXECUTED ;
mText[_GO_CHANGE_EXECUTED].pLabel = __GO_CHANGE_EXECUTED ;
mText[_GO_PROBE_EXECUTED].pLabel = __GO_PROBE_EXECUTED ;
mText[_UNKNOWN_BTN_EXECUTED].pLabel = __UNKNOWN_BTN_EXECUTED ;
mText[_UNABLE_TO_CONNECT_IN_BT].pLabel = __UNABLE_TO_CONNECT_IN_BT ;
mText[_BT_DISCONNECTED].pLabel = __BT_DISCONNECTED ;
mText[_BT_CONNECTED_WITH_GRBL].pLabel = __BT_CONNECTED_WITH_GRBL ;
mText[_GRBL_SERIAL_CONNECTED].pLabel = __GRBL_SERIAL_CONNECTED ;
mText[_UNABLE_TO_CONNECT_TO_GRBL_TELNET].pLabel = __UNABLE_TO_CONNECT_TO_GRBL_TELNET ;
mText[_TELNET_CONNECTED_WITH_GRBL].pLabel = __TELNET_CONNECTED_WITH_GRBL ;
mText[_CURRENT_GRBL_DIR].pLabel = __CURRENT_GRBL_DIR ;
mText[_READING_FILES_ON_GRBL].pLabel = __READING_FILES_ON_GRBL ;
mText[_PLEASE_WAIT ].pLabel = __PLEASE_WAIT  ;
mText[_NO_IP_ASSIGNED].pLabel = __NO_IP_ASSIGNED ;
mText[_USES_BT_TO_GRBL].pLabel = __USES_BT_TO_GRBL ;
mText[_USES_TELNET_TO_GRBL].pLabel = __USES_TELNET_TO_GRBL ;
mText[_TRY_TO_CONNECT_WITH_BT].pLabel = __TRY_TO_CONNECT_WITH_BT ;
mText[_WAIT_1_MIN].pLabel = __WAIT_1_MIN ;
mText[_WIFI_NOT_CONNECTED].pLabel = __WIFI_NOT_CONNECTED ;
mText[_TRY_TO_CONNECT_WITH_TELNET].pLabel = __TRY_TO_CONNECT_WITH_TELNET ;
mText[_WIFI_CONNECTED].pLabel = __WIFI_CONNECTED ;
mText[_CONFIRM_SD_FILE].pLabel = __CONFIRM_SD_FILE ;
//mText[].pLabel = _ ;



// **************** Fill the error messages sent by Grbl
mGrblErrors[_UNKNOWN_ERROR].pLabel = __UNKNOWN_ERROR ;
mGrblErrors[_EXPECTED_CMD_LETTER].pLabel = __EXPECTED_CMD_LETTER ;
mGrblErrors[_BAD_NUMBER_FORMAT].pLabel = __BAD_NUMBER_FORMAT ;
mGrblErrors[_INVALID_$_SYSTEM_CMD].pLabel = __INVALID_$_SYSTEM_CMD ;
mGrblErrors[_NEGATIVE_VALUE].pLabel = __NEGATIVE_VALUE ;
mGrblErrors[_HOMING_NOT_ENABLED].pLabel = __HOMING_NOT_ENABLED ;
mGrblErrors[_STEP_PULSE_LESS_3_USEC].pLabel = __STEP_PULSE_LESS_3_USEC ;
mGrblErrors[_EEPROM_READ_FAIL].pLabel = __EEPROM_READ_FAIL ;
mGrblErrors[_$_WHILE_NOT_IDLE].pLabel = __$_WHILE_NOT_IDLE ;
mGrblErrors[_LOCKED_ALARM_OR_JOG].pLabel = __LOCKED_ALARM_OR_JOG ;
mGrblErrors[_SOFT_LIMIT_NO_HOMING].pLabel = __SOFT_LIMIT_NO_HOMING ;
mGrblErrors[_LINE_OVERFLOW].pLabel = __LINE_OVERFLOW ;
mGrblErrors[_STEP_RATE_TO_HIGH].pLabel = __STEP_RATE_TO_HIGH ;
mGrblErrors[_SAFETY_DOOR_DETECTED].pLabel = __SAFETY_DOOR_DETECTED ;
mGrblErrors[_LINE_LENGHT_EXCEEDED].pLabel = __LINE_LENGHT_EXCEEDED ;
mGrblErrors[_JOG_TRAVEL_EXCEEDED].pLabel = __JOG_TRAVEL_EXCEEDED ;
mGrblErrors[_INVALID_JOG_COMMAND].pLabel = __INVALID_JOG_COMMAND ;
mGrblErrors[_LASER_REQUIRES_PWM].pLabel = __LASER_REQUIRES_PWM ;
mGrblErrors[_NO_HOMING_DEFINED].pLabel = __NO_HOMING_DEFINED ;
mGrblErrors[_UNKNOWN_ERROR_19].pLabel = __UNKNOWN_ERROR_19 ;
mGrblErrors[_UNSUPPORTED_COMMAND].pLabel = __UNSUPPORTED_COMMAND ;
mGrblErrors[_MODAL_GROUP_VIOLATION].pLabel = __MODAL_GROUP_VIOLATION ;
mGrblErrors[_UNDEF_FEED_RATE].pLabel = __UNDEF_FEED_RATE ;
mGrblErrors[_CMD_REQUIRES_INTEGER].pLabel = __CMD_REQUIRES_INTEGER ;
mGrblErrors[_SEVERAL_AXIS_GCODE].pLabel = __SEVERAL_AXIS_GCODE ;
mGrblErrors[_REPEATED_GCODE].pLabel = __REPEATED_GCODE ;
mGrblErrors[_AXIS_MISSING_IN_GCODE].pLabel = __AXIS_MISSING_IN_GCODE ;
mGrblErrors[_INVALID_LINE_NUMBER].pLabel = __INVALID_LINE_NUMBER ;
mGrblErrors[_VALUE_MISSING_IN_GCODE].pLabel = __VALUE_MISSING_IN_GCODE ;
mGrblErrors[_G59_WCS_NOT_SUPPORTED].pLabel = __G59_WCS_NOT_SUPPORTED ;
mGrblErrors[_G53_WITHOUT_G01_AND_G1].pLabel = __G53_WITHOUT_G01_AND_G1 ;
mGrblErrors[_AXIS_NOT_ALLOWED].pLabel = __AXIS_NOT_ALLOWED ;
mGrblErrors[_G2_G3_REQUIRE_A_PLANE].pLabel = __G2_G3_REQUIRE_A_PLANE ;
mGrblErrors[_INVALID_MOTION_TARGET].pLabel = __INVALID_MOTION_TARGET ;
mGrblErrors[_INVALID_ARC_RADIUS].pLabel = __INVALID_ARC_RADIUS ;
mGrblErrors[_G2_G3_REQUIRE_OFFSET].pLabel = __G2_G3_REQUIRE_OFFSET ;
mGrblErrors[_UNSUSED_VALUE].pLabel = __UNSUSED_VALUE ;
mGrblErrors[_G431_TOOL_LENGTH].pLabel = __G431_TOOL_LENGTH ;
mGrblErrors[_TOOL_NUMBER_EXCEED_MAX].pLabel = __TOOL_NUMBER_EXCEED_MAX ;
mGrblErrors[_PARAMETER_P_EXCEED_MAX].pLabel = __PARAMETER_P_EXCEED_MAX ;
mGrblErrors[_SD_FAILED_TO_MOUNT].pLabel = __SD_FAILED_TO_MOUNT ;
mGrblErrors[_SD_FAILED_FOR_READING].pLabel = __SD_FAILED_FOR_READING ;
mGrblErrors[_SD_FAILED_FOR_DIRECTORY].pLabel = __SD_FAILED_FOR_DIRECTORY ;
mGrblErrors[_SD_DIR_NOT_FOUND].pLabel = __SD_DIR_NOT_FOUND ;
mGrblErrors[_SD_FILE_EMPTY].pLabel = __SD_FILE_EMPTY ;
mGrblErrors[_RESERVE_65].pLabel = __RESERVE_65 ;
mGrblErrors[_RESERVE_66].pLabel = __RESERVE_66 ;
mGrblErrors[_RESERVE_67].pLabel = __RESERVE_67 ;
mGrblErrors[_RESERVE_68].pLabel = __RESERVE_68 ;
mGrblErrors[_RESERVE_69].pLabel = __RESERVE_69 ;
mGrblErrors[_BT_FAILED].pLabel = __BT_FAILED ;
//mGrblErrors[].pLabel = _ ;

// **************** Fill the alarm messages sent by Grbl
mAlarms[_UNKNOWN_ALARM].pLabel = __UNKNOWN_ALARM ;
mAlarms[_HARD_LIMIT_REACHED].pLabel = __HARD_LIMIT_REACHED ;
mAlarms[_MOTION_EXCEED_CNC].pLabel = __MOTION_EXCEED_CNC ;
mAlarms[_RESET_IN_MOTION].pLabel = __RESET_IN_MOTION ;
mAlarms[_PROBE_INIT_FAIL].pLabel = __PROBE_INIT_FAIL ;
mAlarms[_PROBE_TRAVEL_FAIL].pLabel = __PROBE_TRAVEL_FAIL ;
mAlarms[_RESET_DURING_HOMING].pLabel = __RESET_DURING_HOMING ;
mAlarms[_DOOR_OPEN_HOMING].pLabel = __DOOR_OPEN_HOMING ;
mAlarms[_LIMIT_ON_HOMING].pLabel = __LIMIT_ON_HOMING ;
mAlarms[_LIMIT_MISSING_HOMING].pLabel = __LIMIT_MISSING_HOMING ;

loadLanguage() ;  // load buttons, text, errors and Alarm from language (look on SD card and then preferences)
}  // end of init


enum { _undefined_ = 0  , _Buttons_ , _Text_ , _GrblErrors_ , _Alarms_ , _End_};

  uint8_t sectionLang  = _undefined_; // section being read
  int sectionLangIdx = 0 ;
 
void loadLanguage(void){
  // try to read a language file on sd card
  // if found, copy it to SPIFFSS
  // Read the file from SPIFFSS; if not found, keep names defined in english at compilation
  // Décode the file and upload data in different arrays (mButton; mText, mGrblErrors, mAlarms)  
  char line[200];  // line being read
  int lineIdx = 0;
  char c ;
  copyLanguage() ; // try to copy language.txt from SD to SPIFSS  // to do TODO
  fs::File language;
  if(!SPIFFS.begin( false)){
        Serial.println("[MSG: SPIFFS Mount Failed for loading language");
        return;
  }
  language = SPIFFS.open("/language.txt") ;
  if ( ! language) {
    //Serial.println("fail to open language.txt");
    return ;
  }
  //Serial.print("At begining free heap="); Serial.println(ESP.getFreeHeap()) ;
  while (language.available() ) {
    c = language.read() ;
    //Serial.println(c,HEX); 
    if ( c != '\r' && c!='\n') {
      if (c == 0xC3 ) {  // convert accent because tft uses a reduced set of char
        c = language.read() ;
        if ( c == 0xA9 ) {c = 0x80 ; // é
        } else if ( c == 0xA4 ) {c = 0x81 ; // ä
        } else if ( c == 0xB6 ) {c = 0x82 ; // ö
        } else if ( c == 0xBC ) {c = 0x83 ; // ü
        } else {
          c = '?' ;  // replace unexpect car with ?
        }
      }
      line[lineIdx] = c ;  // store the char
        if (lineIdx < 198) lineIdx++;
    } else {
      line[lineIdx] = '\0' ; // add end of line
      handleLangLine(line) ; // handle the line.
      lineIdx = 0 ; 
      line[0] = '\0' ; // reset line
    } // end if testing a car
  } // end while
  language.close() ; 
  //Serial.print("Free heap at end of language=");Serial.println(ESP.getFreeHeap());
}

void handleLangLine(char* line) {  // process one line of language.txt
      // first search for a section name
      // then look for "" or "abc..." and store the name in memory and change the pointer
      char * pTextBegin ;
      char * pTextEnd ;
      char text[100];
      char * pBtn ;  
      if (strncmp(line, "#Buttons" , strlen("#Buttons") ) == 0) {
        sectionLang = _Buttons_ ;
        sectionLangIdx = 0;
      }else if (strncmp(line, "#Text" , strlen("#Text") ) == 0) {
        sectionLang = _Text_ ;
        if (sectionLangIdx != _MAX_BTN){
          Serial.print("Number of lines in #Buttons section of file language is not correct");
          Serial.print(" - Nbr lines in file=") ; Serial.print(sectionLangIdx);Serial.print(" Expected=") ; Serial.println(_MAX_BTN); 
        }
        sectionLangIdx = 0;
      }else if (strncmp(line, "#GrblErrors" , strlen("#GrblErrors") ) == 0) {
        sectionLang = _GrblErrors_ ;
        if (sectionLangIdx != _MAX_TEXT){
          Serial.print("Number of lines in #Text section of file language is not correct");
          Serial.print(" - Nbr lines in file=") ; Serial.print(sectionLangIdx);Serial.print(" Expected=") ; Serial.println(_MAX_TEXT); 
        }
        sectionLangIdx = 0;
      }else if (strncmp(line, "#Alarms" , strlen("#Alarms") ) == 0) {
        sectionLang = _Alarms_ ;
        if (sectionLangIdx != _MAX_GRBL_ERRORS){
          Serial.print("Number of lines in #GrblErrors section of file language is not correct");
          Serial.print(" - Nbr lines in file=") ; Serial.print(sectionLangIdx);Serial.print(" Expected=") ; Serial.println(_MAX_GRBL_ERRORS); 
        }
        sectionLangIdx = 0;
      }else if (strncmp(line, "#End" , strlen("#End") ) == 0) {
        sectionLang = _End_ ;
        if (sectionLangIdx != _MAX_ALARMS){
          Serial.print("Number of lines in #Alarms section of file language is not correct");
          Serial.print(" - Nbr lines in file=") ; Serial.print(sectionLangIdx);Serial.print(" Expected=") ; Serial.println(_MAX_ALARMS); 
     
        }
        sectionLangIdx = 0;      
      } else { // it is not a section tag; so search to " to find Text to handle
            text[0] = '\0' ;
            pTextBegin = strchr(line, '"' ); // search first "
            pTextEnd = strrchr(line, '"'); // search last "
            if (pTextBegin == NULL) {  // discard line without a "
              //Serial.println("line without double quote in language definition");
            } else if ( pTextBegin == pTextEnd ){                    // dicard when there is only one "
               Serial.println("line discarded: only one double quote in language definition");
            } else {  // handle the text; note :"" is not stored but index is increase 
                if (( pTextEnd - pTextBegin) > 1 ) { // there is at least one char between ""
                      memcpy(text , pTextBegin + 1 , pTextEnd - pTextBegin-1 ) ; // fill text as string
                      text[ pTextEnd - pTextBegin -1] = '\0' ; // add end string
                      switch (sectionLang) {
                            case _Buttons_:
                              if (sectionLangIdx >= _MAX_BTN) {
                                Serial.println("to many buttons defined in language file");
                              } else {
                                pBtn = (char *)malloc( sizeof(char) * ( strlen(text) + 1 ) );
                                strcpy(pBtn , text);
                                mButton[sectionLangIdx].pLabel = pBtn; 
                                //Serial.print("Button ") ;Serial.print(sectionLangIdx); Serial.print(" txt="); Serial.print( text ) ;
                                //Serial.print(" label="); Serial.println( mButton[sectionLangIdx].pLabel );
                              }
                              break ;
                            case _Text_ :
                              if (sectionLangIdx >= _MAX_TEXT) {
                                Serial.println("to many Texts defined in language file");
                              } else {
                                pBtn = (char *)malloc( sizeof(char) * ( strlen(text) + 1 ) );
                                strcpy(pBtn , text);
                                mText[sectionLangIdx].pLabel = pBtn; 
                                //Serial.print("Text nr: ") ;Serial.print(sectionLangIdx); Serial.print(" txt="); Serial.print( text ) ;
                                //Serial.print(" label="); Serial.println( mText[sectionLangIdx].pLabel );
                              }
                              break;
                            case _GrblErrors_ :
                              if (sectionLangIdx >= _MAX_GRBL_ERRORS) {
                                Serial.println("to many errors defined in language file");
                              } else {
                                pBtn = (char *)malloc( sizeof(char) * ( strlen(text) + 1 ) );
                                strcpy(pBtn , text);
                                mGrblErrors[sectionLangIdx].pLabel = pBtn;
                              }
                              break; 
                            case _Alarms_ :
                              //Serial.print(line);Serial.println("reading an Alarm");
                              if (sectionLangIdx >= _MAX_ALARMS) {
                                Serial.println("to many alarms defined in language file");
                              } else {

                                pBtn = (char *)malloc( sizeof(char) * ( strlen(text) + 1 ) );
                                strcpy(pBtn , text);
                                mAlarms[sectionLangIdx].pLabel = pBtn;
                              }
                              break; 
                      } // end switch
                      
                } // end handling the when there is at least one text 
                sectionLangIdx++ ; // increase index to write the next text (only when we have at least "" or "abc...")    
            }     
      } // end handling a line
}      

boolean copyLanguage(void){ // copy language.txt from SD to SPIFFS
      SdBaseFile langFile ;
      fs::File language;
      if ( ! sd.begin(SD_CHIPSELECT_PIN , SD_SCK_MHZ(5)) ) {  
          //Serial.println(" fail to begin sd");
          return false;       
      }
      if ( ! sd.exists( "/" ) ) { // check if root exist   
          //Serial.println(" fail to exists sd");
          return false;  
      }
      if ( ! sd.chdir( "/" ) ) {
          //Serial.println(" fail to chdir sd");
          return false;  
      }
      if ( ! langFile.open("/language.txt" ) ) { // try to open language.txt 
          //Serial.println(" fail to open language.txt on sd");
          return false;  
      }
      if(!SPIFFS.begin( false)){
        //Serial.println(" fail to begin spifss");
        return false;
      }
      language = SPIFFS.open("/language.txt" , "w") ;
      if ( ! language) {
        //Serial.println("fail to open language.txt on spiffs");
        return false;
      }    
      while (langFile.available() > 0) {
        language.write(langFile.read()); 
      }
      langFile.close() ;
      language.close() ;

      Serial.println("[MSG: File language.txt uploaded from SD card]") ;
      return true ;
}
