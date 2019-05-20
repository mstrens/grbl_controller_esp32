// compilé dans mon cas avec la board WEMOS LOLIN32

// to do
// au démarrage, GRBL donne parfois des messages d'erreurs. Chercher pourquoi.
// tester l'impression par SD et par CMD
// tester l'impression par telnet et BCC
// prévoir des icones pour les boutons; on peut créer des charactères en format RLE
// sans doute autoriser des déplacements en jog (avec la nunchuk notamment) si le statut est en HOLD ou en DOOR? (actuellement seul les statuts Jog et Idle sont autorisés pour la nunchuk 
// let the user delete a cmd button in some way (e.g. printing a file name Cmd5.xxx
/*
Gestion r-cnc avec touch screen et esp32 avec carte sd.

- si pas fait récemment, voir si une touche est enfoncée
- si une touche est enfoncée, met à jour le menu à afficher
- traite le nunchuck (lit et active des données)
- reçoit de grbl et forward au pc si nécessaire
- envoie à grbl (soit de la carte sd, soit en passthrough, soit du menu)
- affiche l'écran (si nécessaire)



Taches
- examiner si l'écran est pressé, si oui, envoyer un message à la tache qui traite l'écran
- afficher l'écran choisi
- envoyer et recevoir de grbl
*/


/*
 Structure du programme pour un controller GRBL sur ESP32
Utilise un écran avec touch screen et SD card
L'écran fait 320 X 240
Utilise un esp32 avec prise usb
Prévoir l'utilisation d'un nunchuck (I2C)
Prévoir la réception de fichier sur la SD card par wifi

Sur l'écran de base, prévoir l'affichage des infos
//   Info screen 
//            USB->Grbl                  Idle                (or Run, Alarm, ... grbl status)  
//                                                 So, printing status (blanco, ,SD-->Grbl  xxx%, USB<-->Grbl , Pause,  Cmd)  = printing status
//                                                 and GRBL status (or Run, Alarm, ... grbl status)
//            Last message                  (ex : card inserted, card removed, card error, Error: 2 ; 
//            T                                   T = telnet connected or # = telnet not connected
//              Wpos          Mpos
//            X xxxxxpos      xxxxxpos                 
//            Y yyyyypos      yyyyypos       
//            Z zzzzzpos      zzzzzpos  
//            F 100           S 10000 
 */
#include "config.h"
#include "TFT_eSPI_ms/TFT_eSPI.cpp"   // setup file has to be edited for some parameters like screen device, pins
#include "language.h"
#include "draw.h"
#include "FS.h"
#include "nunchuk.h"
#include "com.h"
//#include "SD.h"
#include "SdFat.h"
#include "menu_file.h"
#include "browser.h"
#include "telnet.h"
#include "cmd.h"

extern TFT_eSPI tft ;       // Invoke custom library

//       tft and touchscreen variables 
uint8_t prevPage , currentPage = 0 ;
boolean waitReleased = false ;
boolean updateFullPage = true ;
boolean updatePartPage = true ;
uint8_t justPressedBtn , justReleasedBtn, longPressedBtn ,currentBtn = 0 ; // 0 = nihil, 1 à 9 = position sur l'écran du bouton (donc pas la fonction du bouton)
uint32_t beginChangeBtnMillis ;
char lastMsg[80] = { 0} ;        // last message to display
boolean lastMsgChanged = false ;
char grblLastMessage[STR_GRBL_BUF_MAX_SIZE] ;
boolean grblLastMessageChanged;


//         SD variables  
int8_t dirLevel ; //-1 means that card has to be (re)loaded
char fileNames[MAX_FILES][23] ; // 22 car per line + "\0"; utilisé dans la construction de la liste des fichiers
//SdFat sd;
//File root ;
//File workDir ;
//File fileToRead ; // file being printed
//File workDirParents[DIR_LEVEL_MAX] ;
//File aDir[DIR_LEVEL_MAX] ;
SdFat sd;
SdBaseFile aDir[DIR_LEVEL_MAX] ; 
//SdBaseFile fileToRead ; // file being printed

uint16_t sdFileDirCnt = 0 ;
uint16_t firstFileToDisplay ;   // 0 = first file in the directory
uint32_t sdFileSize ;
uint32_t sdNumberOfCharSent ;

//         Commande à exécuter
char cmdName[7][17] ;     // store the names of the commands
uint8_t cmdToSend = 0 ;   //store the cmd to be send to grbl

//         printing status
uint8_t statusPrinting = PRINTING_STOPPED ;

// grbl data
boolean newGrblStatusReceived = false ;
char machineStatus[9];           // Iddle, Run, Alarm, ...
volatile boolean waitOk = false ;

// Nunchuk data.
extern boolean nunchukOK ;  // keep flag to detect a nunchuk at startup

// status pour telnet
boolean statusTelnetIsConnected = false ; 

/***************   Prototypes of function to avoid forward references*********************************************************************/
//uint16_t fileCnt( void ) ;  // prototype
void initMenuOptions( void) ;     //prototype
//void sendGrblMove( int8_t dir , struct menustate* ms) ; 
 
/**************************************************************************************************/
 
void setup() {
// initialiser le serial vers USB
// initialiser le UART vers GRBL
// initialiser l'écran et le touchscreen
// initialiser spiffs (pour relire la config)
// détecter SD card (en principe rien à faire; SD.begin est appelé avec l'écran SD)
// initialiser Wifi server
// teste la présence et initialise le nunchuck
// initialiser les status (notamment affichage de l'écran)

  Serial.begin(115200); // init UART for debug and for Gcode passthrough via USB PC
  
    // initialise le port série vers grbl
  Serial2.begin(115200, SERIAL_8N1, SERIAL2_RXPIN, SERIAL2_TXPIN); // initialise le port série vers grbl
  pinMode(TFT_LED_PIN , OUTPUT) ;
  digitalWrite(TFT_LED_PIN , HIGH) ;
  tftInit() ; // init screen and touchscreen, set rotation and calibrate
  if (! spiffsInit() ) {   // just to test. Todo : change for loading the cmd in memory if the file exist in spiffs 
    fillMsg(__SPIFFS_FORMATTED ) ;
  } else {
    if (! cmdNameInit() ) {
      fillMsg( __CMD_NOT_LOADED ) ;
    }
  }
//  listSpiffsDir( "/", 0 );   // uncomment to see the SPIFFS content
  
  initButtons() ; //initialise les noms des boutons, les boutons pour chaque page.
  dirLevel = -1 ;   // negative value means that SD card has to be uploaded

  nunchuk_init() ; 
  prevPage = _P_NULL ;     
  currentPage = _P_INFO ;
  updateFullPage = true ;
  // en principe les données pour les buttons sont initialisés automatiquement à 0
  //drawFullPage( ) ;
#if defined ( ESP32_ACT_AS_STATION ) || defined (ESP32_ACT_AS_AP)  
  initWifi() ;
  telnetInit() ;
#endif 
  Serial2.print(0X18) ; // send a soft reset
  Serial2.println(" ") ;Serial2.print("$10=3");Serial2.println(" ") ;   // $10=3 is used in order to get available space in GRBL buffer in GRBL status messages; il also means we are asking GRBL to sent always MPos.
  Serial2.flush();                                                      // this is used to avoid sending to many jogging movements when using the nunchuk  
}

//******************************** Main loop ***************************************
void loop() {
// Lit le touchscreen et détermine si une touche est enfoncée/relachée
// Si une touche est juste enfoncée/relachée, change la couleur du bord du(des boutons)
// Déternine l'action prioritaire à écuter et l'exécute (ne change pas encore l'affichage)
// Lit nunchuck et envoie les commandes éventuelles à GRBL
// Lit les char venant de GRL (et demande un réaffichage et transmet au pc)
// Lit les char venant du pc ou de la carte sd et les transmet à grbl
// Si on a reçu de nouvelles données de GRBL, active un flag pour faire un réaffichage partiel de l'écran
// réaffiche l'écran (complètement ou partiellement) et fait un reset de flags
#if defined ( ESP32_ACT_AS_STATION ) || defined (ESP32_ACT_AS_AP)
  processWifi();
  checkTelnetConnection();
  boolean tempTelnetIsConnected = telnetIsConnected() ;
  if ( statusPrinting == PRINTING_FROM_TELNET && !tempTelnetIsConnected ){
    statusPrinting = PRINTING_STOPPED ;
    fillMsg(__TELENET_DISCONNECTED "Telnet disconnected" );
  }
//  if ( tempTelnetIsConnected && !statusTelnetIsConnected ) {
//    fillMsg( "Telnet connected" ) ; 
//  }
  statusTelnetIsConnected = tempTelnetIsConnected ;
#endif 
 updateBtnState();  // check touch screen and update justPressedBtn ,justReleasedBtn , longPressedBtn and beginChangeBtnMillis
 drawUpdatedBtn() ;        // update color of button if pressed/released, apply actions foreseen for buttons (e.g. change currentPage) 
 executeMainActionBtn () ; // 

 // handle nunchuk if implemented
  if ( nunchukOK && statusPrinting == PRINTING_STOPPED && ( machineStatus[0] == 'I' || machineStatus[0] == 'J' || machineStatus[0] == '?') )  {  //read only if the GRBL status is Idle or Jog or ?? (this last is only for testing without GRBL
    handleNunchuk() ;
  }

  getFromGrblAndForward() ; // get char from serial GRBL and always decode them (check for OK, update machineStatus and positions),
                            // if statusprinting = PRINTING_FROM_USB or if telnet is active, then forward received char from GRBL to PC (via Serial)
                        

  sendToGrbl() ;           // s'il y de la place libre dans le Tx buffer, le rempli avec le fichier de SD, une CMD ou le flux du PC; envoie périodiquement "?" pour demander le statut
//  if (newGrblStatusReceived) Serial.println( "newStatus");

  if (newGrblStatusReceived == true && ( currentPage == _P_INFO || currentPage == _P_MOVE || currentPage == _P_SETXYZ || currentPage == _P_SETUP ) ) { //force a refresh if a message has been received from GRBL and we are in a info screen or in a info screen
    updatePartPage = true ;
  }
  newGrblStatusReceived = false ;
  if (lastMsgChanged == true && ( currentPage == _P_INFO || currentPage == _P_MOVE || currentPage == _P_SETXYZ || currentPage == _P_SETUP) ) { //force a refresh if a message has been filled
    updatePartPage = true ;
  }
  
  if (  ( updateFullPage ) ) {
    drawFullPage() ; 

  } else if ( updatePartPage ) {   // si l'écran doit être réaffiché, construit l'écran et l'affiche
    drawPartPage() ;                           // si l'écran doit être mis à jour, exécute une fonction plus limitée qui ne redessine pas les boutons        
  }    
  lastMsgChanged = false ; // lastMsgChanged is used in drawPartPage; so, it can not be set on false before
  updateFullPage = false ;
  updatePartPage = false ;
  
}



