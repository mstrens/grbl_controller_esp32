#ifndef _draw_h
#define _draw_h

uint16_t hCoord(uint16_t x) ;

uint16_t vCoord(uint16_t y) ;

void tftInit(void) ;
void testTft(void);
void  touch_calibrate();
void perform_calibration(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size, int16_t _width, int16_t _height);
boolean checkCalibrateOnSD();
#include <stdint.h>
#include "config.h"

// Liste des actions définies
enum { _NO_ACTION = 0 , _JUST_PRESSED  , _JUST_RELEASED , _JUST_LONG_PRESSED , _LONG_PRESSED , _JUST_LONG_PRESSED_RELEASED } ;

// Liste des statuts d'impression
enum { PRINTING_STOPPED = 0 , PRINTING_FROM_SD , PRINTING_ERROR , PRINTING_PAUSED , PRINTING_FROM_USB , PRINTING_CMD , PRINTING_FROM_TELNET , PRINTING_STRING ,
      PRINTING_FROM_GRBL , PRINTING_FROM_GRBL_PAUSED} ;

void clearScreen() ; // clear tft screen


void blankTft(const char * titel , uint16_t x , uint16_t y ) ; // clear tft screen
void drawLineText( char * text, uint16_t x, uint16_t y, uint8_t font , uint8_t fontSize, uint16_t color) ;
void clearLine( uint16_t y , uint8_t font , uint8_t fontSize , uint16_t color ) ; // clear a line based on font and font Size.
void printTft(const char * text) ;
void drawLogo() ;


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
void mButtonBorder(uint8_t pos , uint16_t outline) ; // draw the border of a button at position (from 1 to 12)
void mButtonClear(uint8_t pos , uint8_t btnIdx) ;  // clear one button at position (from 1 to 12)

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
void printOneLogLine(uint16_t col , uint16_t line ) ; // imprime une ligne de log
void fCommunicationBase(void) ; // fonction pour l'affichage de l'écran communication
void fSdGrblWaitBase(void) ; // fonction pour afficher la commande de lecture des fichiers à Grbl
void executeGrblEndOfFileReading() ;// function call when list of file has been provided.
void fSdGrblBase(void) ; // fonction pour afficher la liste des fichiers sur SD card de  Grbl
void fConfirmYesNoBase(void) ; // fonction pour afficher le nom du fichier à imprimer (que l'on doit confirmer)

void updateButtonsInfoPage() ; // met à jour le set up de la page en fonction du statut d'impression
void drawMachineStatus() ;   // affiche le status GRBL dans le coin supérieur droit
void drawLastMsg() ;         // affiche le lastMsg
void drawLastMsgAt( uint16_t col , uint16_t line ) ; // affiche le lastMsg à partir d'une position donnée
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
void drawDataOnCommunicationPage() ; // affiche l'adresse IP et qq autre info sur l'écran communication

int16_t bufferLineLength(int16_t firstCharIdx , int16_t &nCharInFile ) ;   //return the number of char to be displayed on TFT       
int16_t prevBufferLineLength( int16_t beginSearchAt ,  int16_t &nCharInFile ) ;   //return the number of char to be displayed on TFT in the previous line      
uint8_t convertNCharToNLines ( int16_t nChar ) ; // return the number of lines needed on Tft to display one line in the show buffer
  
void fillMsg(  uint8_t msgIdx , uint16_t msgColor = SCREEN_ALERT_TEXT ) ; // fill with a translated text based on Idx
void clearMsg(uint16_t color  = SCREEN_ALERT_TEXT ) ;   // clear message
void fillStringMsg( char * msg, uint16_t color) ;  // fill with a string
void drawMsgOnTft( const char * msg1 , const char * msg2); // affiche 2 lignes de messages
void fSdGrblBase(void) ;                // cette fonction doit vérifier que la carte est accessible et actualiser les noms des fichiers disponibles sur la carte sd attahée à grbl

uint8_t getButton( int16_t x, int16_t y ) ; // convert raw position into tft position


#endif

