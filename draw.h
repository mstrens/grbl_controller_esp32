#ifndef _draw_h
#define _draw_h

void tftInit(void) ;
void testTft(void);
void  touch_calibrate();
boolean checkCalibrateOnSD();
#include <stdint.h>
#include "config.h"


// créer une stucture pour chaque sorte de bouton 
//  *texte, textsize , *iconName
// créer un array de structure de bouton
struct M_Button {
  //uint8_t  textsize ;  // To use if we want different text size.
  const char  * pLabel ;
};

// Liste des boutons disponibles
enum { _NO_BUTTON = 0 , _SETUP , _PRINT , _HOME, _UNLOCK , _RESET , _SD , _USB_GRBL , _TELNET_GRBL, _PAUSE , _CANCEL , _INFO , _CMD ,
_MOVE , _RESUME , _STOP_PC_GRBL , _XP , _XM , _YP , _YM , _ZP , _ZM, _AP, _AM, _D_AUTO , _D0_01 , _D0_1 , _D1, _D10 , _SET_WCS ,
_SETX , _SETY , _SETZ, _SETA , _SETXYZ , _SETXYZA ,_SET_CHANGE,  _SET_PROBE, _SET_CAL , _GO_CHANGE, _GO_PROBE , _TOOL , _BACK , _LEFT, _RIGHT , _UP ,
 _CMD1 ,_CMD2 ,_CMD3 ,_CMD4 ,_CMD5 ,_CMD6 ,_CMD7 , _CMD8 , _CMD9 , _CMD10 , _CMD11 , _MORE_PAUSE , _FILE0 , _FILE1 , _FILE2 , _FILE3 ,
 _MASKED1 , _PG_PREV , _PG_NEXT, _SD_SHOW , 
 _OVERWRITE, _OVER_SWITCH_TO_FEEDRATE , _OVER_SWITCH_TO_SPINDLE, _OVER_100 , _OVER_10P, _OVER_10M,_OVER_1P, _OVER_1M, _TOOGLE_SPINDLE, _MAX_BTN} ; // keep _MAX_BTN latest

// Liste des pages définies
enum { _P_NULL = 0  , _P_INFO , _P_SETUP , _P_PRINT , _P_PAUSE , _P_MOVE , _P_SETXYZ , _P_SD , _P_CMD , _P_LOG , _P_TOOL, 
      _P_SD_SHOW , _P_OVERWRITE, _P_MAX_PAGES} ; // keep _P_MAX_PAGE latest

// Liste des actions définies
enum { _NO_ACTION = 0 , _JUST_PRESSED  , _JUST_RELEASED , _JUST_LONG_PRESSED , _LONG_PRESSED , _JUST_LONG_PRESSED_RELEASED } ;

// Liste des statuts d'impression
enum { PRINTING_STOPPED = 0 , PRINTING_FROM_SD , PRINTING_ERROR , PRINTING_PAUSED , PRINTING_FROM_USB , PRINTING_CMD , PRINTING_FROM_TELNET , PRINTING_STRING} ;

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

// fonctions pour un bouton (index du bouton): 
//          mButtonDraw(position , btnIdx) // dessine le bouton à une place correspondant à l'index 
//          mButtonBorder(position, color) // change la couleur du bord 
//          
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

void clearScreen() ; // clear tft screen
void fillMPage (uint8_t _page , uint8_t _btnPos , uint8_t _boutons, uint8_t _actions , void (*_pfNext)(uint8_t) , uint8_t _parameters )  ;
void initButtons() ;   // initialise les noms des boutons, les boutons pour chaque page.

void blankTft(char * titel , uint16_t x , uint16_t y ) ; // clear tft screen
void printTft(char * text) ;

boolean convertPosToXY( uint8_t pos , int32_t *_x, int32_t *_y , uint16_t btnDef[12][4]) ;
uint8_t convertBtnPosToBtnIdx( uint8_t page , uint8_t btn ) ;

// à chaque loop,
void updateBtnState( ) ;       // tester le touchscreen et mettre à jour les valeurs de just pressed, long pressed, just released
void drawUpdatedBtn( ) ;       // update the color of the buttons on a page (based on currentPage, justPressedBtn , justReleasedBtn, longPressedBtn)
void executeMainActionBtn () ; // execute the action forseen for ONE button (if any) (change currentPage, printingStatus,... but do not change the display)

void drawFullPage() ;          // redraw totally the page
void drawPartPage() ;          // update only the data on creen (not the button)

void drawAllButtons() ;        // draw all buttons defined on the page (except the one that are _MASKED)
void mButtonDraw(uint8_t pos , uint8_t btnIdx) ;  // draw one button at position (from 1 to 12) 

// fonctions quand on active un bouton
void fInfoBase(void) ; // fonction pour l'affichage de base de la page info
void fNoBase(void) ; // Ne fait rien = fonction pour l'affichage d'une page avec uniquement des boutons
void fSetupBase(void) ; //fonction pour l'affichage de base de la page setup (= adresse IP)
void fMoveBase(void) ; // fonction pour l'affichage de l'écran Move
void fSetXYZBase(void) ; // fonction pour l'affichage de l'écran Set XYZ
void fSdBase(void) ; // fonction pour l'affichage de l'écran de la carte SD
void fCmdBase(void) ; // fonction pour l'affichage del'écran Cmd
void fLogBase(void) ; // fonction pour l'affichage de l'écran Log
void fToolBase(void) ;  // fonction pour l'affichage de l'écran Change tool
void fSdShowBase(void) ;  // fonction pour l'affichage de l'écran SdShow
void fOverBase(void) ;  // fonction pour l'affichage de l'écran Overwrite
void printOneLogLine(uint8_t col , uint8_t line ) ; // imprime une ligne de log

void updateButtonsInfoPage() ; // met à jour le set up de la page en fonction du statut d'impression
void drawMachineStatus() ;   // affiche le status GRBL dans le coin supérieur droit
void drawLastMsg() ;         // affiche le lastMsg 
void drawDataOnInfoPage()  ; // affiche les données sur la page d'info
void drawWposOnMovePage() ;
void drawDataOnSetupPage() ;  // affiche wpos et distance since entry on this screen
void drawDataOnSetXYZPage() ; // affiche wpos
void drawWposOnSetXYZPage() ; // affiche Wpos
void drawWifiOnSetupPage() ; // affiche l'adresse IP sur l'écran set up
void drawDataOnLogPage() ; // affiche une page de log (sans les boutons)
void drawDataOnToolPage() ; // affiche le statut GRBL et le lastMsg
void drawDataOnSdShowPage() ; // affiche une page de données de SD (sans les boutons)
void drawDataOnOverwritePage() ; // affiche les données Feedrate et RPM (en absolu et en % d'overwrite) et précise si les boutons concernent la modification de feedrate ou de RPM 

int16_t bufferLineLength(int16_t firstCharIdx , int16_t &nCharInFile ) ;   //return the number of char to be displayed on TFT       
int16_t prevBufferLineLength( int16_t beginSearchAt ,  int16_t &nCharInFile ) ;   //return the number of char to be displayed on TFT in the previous line      
uint8_t convertNCharToNLines ( int16_t nChar ) ; // return the number of lines needed on Tft to display one line in the show buffer
  
void fillMsg(  const char * msg , uint16_t msgColor = SCREEN_ALERT_TEXT ) ;

uint8_t getButton( int16_t x, int16_t y ) ; // convert raw position into tft position

//  void     initButtonUL(TFT_eSPI *gfx, int16_t x1, int16_t y1,
//  uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
//  uint16_t textcolor, char *label, uint8_t textsize);
// structure pour une page
// 



#endif

