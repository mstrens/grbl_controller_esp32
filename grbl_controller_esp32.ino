// compilé dans mon cas avec la board WEMOS LOLIN32

// attention: pour recevoir tous les caractères envoyés par GRBL en réponse à $$, il faut augmenter la taille du buffer des Serial
// pour cela, on peut employer une fonction prévue Serial2.setRxBufferSize(size_t)

// to do

// permettre l'accès au contenu du fichier SD seulement si le statut est print from SD en pause. 

// prévoir de pouvoir faire un "continue" quand on a une pause alors que l'on est en train d'envoyer des CMD ou des STRING vers GRBL 
// prévoir des icones pour les boutons; on peut créer des charactères en format RLE
// sans doute autoriser des déplacements en jog (avec la nunchuk notamment) si le statut est en HOLD ou en DOOR? (actuellement seul les statuts Jog et Idle sont autorisés pour la nunchuk
// déplacer les tests relatifs à la place restant dans le serial2 bufferwrite dans la fonction qui regroupe les transmissions à grbl - voir plusieurs fois (Serial2.availableForWrite() != 0x7F ) 
// afficher un message sur le tft si la liaison telnet ou BT vers grbl est demandée mais ne s'active pas
// afficher un message sur le tft si la liaison telnet ou BT vers grbl est demandée mais est perdue (? comment détecter une liaison telnet perdue?)
// à voir: ajouter un paramètre dans config pour dire si on utilise GRBL_ESP32, et si oui, quel sont les noms de BT sur cet ESP32 et sur GRBL_ESP32 et l'adresse (ou nom du serveur) telnet 
// remplacer certains messages envoyés à Serial par des messages affichés au TFT notamment au sujet des (dé)connectios de BT et Telnet vers GRBL.
// quel statut appliquer pendant l'exécution d'un print via sd grbl? Quel blocage implémenter. Commennt reconnaître la fin. Voir si le % donné dans la ligne de statut est valable 
// ajouter des libellés de code d'erreur dans langage. Attention: il y a plus d'erreur et pas mal de trous. Il faut peut être changer le système par exemple en les stockant dans preference.
// voir les modifs faites par bradley pour éviter certains warnings; voir ce qu'il a fait pour éviter la perte du telnet au démarrage suite au reset
// fusionner telnet et Bt dans un seul fichier pour réduire le nombre de tab
// bouger le caracère telnet dans l'écran COM et y dire s'il a wifi ou pas: Wifi mode: None, STA , AP ; Wifi connected/Wifi not connected
// vérifier que les messages de grbl sont stockés dans LastgrblMsg et non dans Msg.
// mesurer le delai entre le send d'une commande et le OK (via telnet cela semble aller, à voir via serial)
// vérifier si le soft reset provoque la perte de la liaison bluetooth ou telnet (car comment canceler un job lancer de GRBL SD card sinon); changer fCancel dans le tab actions
// pourquoi avoir parfois des messages "error 120"
// afficher le nom du fichier en cas d'usinage via la carte GRBL SD
// Mettre en pause si on lance le fichier via GRBL SD????
// le move au clavier ne marche pas toujours : grbl retourne une erreur 8 
// avec l'impression via grbl SD, quand on fait pause, l'écran doit être adapté (ne pas montrer l'icone show gcode) et certaines fonctions aussi (sans doute Cancel et Resume)



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
#include "setupTxt.h"
#include "FS.h"
#include "nunchuk.h"
#include "com.h"
//#include "SD.h"
#include "SdFat.h"
#include "menu_file.h"
#include "browser.h"
#include "telnet.h"
#include "cmd.h"
#include "log.h"
#include <Preferences.h>
#include "soc/uart_reg.h"
#include "soc/uart_struct.h"
#include "touch.h"
#include "telnetgrbl.h"
#include "bt.h"


//uart_dev_t * dev = (volatile uart_dev_t *)(DR_REG_UART_BASE) ;
//
//  uart_t _uart_bus_array[3] = {
//    {(volatile uart_dev_t *)(DR_REG_UART_BASE), 0, NULL, NULL},
//    {(volatile uart_dev_t *)(DR_REG_UART1_BASE), 1, NULL, NULL},
//    {(volatile uart_dev_t *)(DR_REG_UART2_BASE), 2, NULL, NULL}
//  };


Preferences preferences ; // object from ESP32 lib used to save/get data in flash 

extern TFT_eSPI tft ;       // Invoke custom library
extern TOUCH touchscreen; 


//       tft and touchscreen variables 
uint8_t prevPage , currentPage = 0 ;
boolean waitReleased = false ;
boolean updateFullPage = true ;
boolean updatePartPage = true ;
uint8_t justPressedBtn , justReleasedBtn, longPressedBtn ,currentBtn = 0 ; // 0 = nihil, 1 à 9 = position sur l'écran du bouton (donc pas la fonction du bouton)
uint32_t beginChangeBtnMillis ;
char lastMsg[80] = { 0} ;        // last message to display
uint16_t lastMsgColor ;          // color of last message
boolean lastMsgChanged = false ;
char grblLastMessage[STR_GRBL_BUF_MAX_SIZE] ; //= "1234567890123456789012345678901234567890"         ;
boolean grblLastMessageChanged;


//         SD variables  
int8_t dirLevel ; //-1 means that card has to be (re)loaded
char fileNames[MAX_FILES][23] ; // 22 car per line + "\0"; utilisé dans la construction de la liste des fichiers

//File root ;
//File workDir ;
//File fileToRead ; // file being printed
//File workDirParents[DIR_LEVEL_MAX] ;
//File aDir[DIR_LEVEL_MAX] ;
SdFat32 sd;
SdBaseFile aDir[DIR_LEVEL_MAX] ; 
//SdBaseFile fileToRead ; // file being printed

uint16_t sdFileDirCnt = 0 ;
uint16_t firstFileToDisplay ;   // 0 = first file in the directory
uint32_t sdFileSize ;
uint32_t sdNumberOfCharSent ;


//         Commande à exécuter
char cmdName[11][17] ;     // store the names of the commands
uint8_t cmdToSend = 0 ;   //store the cmd to be send to grbl

//         printing status
uint8_t statusPrinting = PRINTING_STOPPED ;

// grbl data
boolean newGrblStatusReceived = false ;
char machineStatus[9];           // Iddle, Run, Alarm, ...
volatile boolean waitOk = false ;

// Nunchuk data.
extern boolean nunchukOK ;  // keep flag to detect a nunchuk at startup

// type of wifi being used
uint8_t wifiType ; // can be NO_WIFI(= 0), ESP32_ACT_AS_STATION(= 1), ESP32_ACT_AS_AP(= 2)
extern uint8_t grblLink ;

// status pour telnet
boolean statusTelnetIsConnected = false ; 

// for grbl file system
extern uint8_t grblFileReadingStatus;
extern uint8_t parseGrblFilesStatus ;  // status to know if we are reading [FILES: lines from GRBL or if it is just done 

extern float runningPercent ; // contains the percentage of char being sent to GRBL from SD card on GRBL_ESP32; to check if it is valid
extern boolean runningFromGrblSd  ; // indicator saying that we are running a job from the GRBL Sd card ; is set to true when status line contains SD:

extern M_Button mButton[] ;

/***************   Prototypes of function to avoid forward references*********************************************************************/
//uint16_t fileCnt( level ) ;  // prototype
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
  logBufferInit() ; // initialise the log buffer
  Serial.begin(115200); // init UART for debug and for Gcode passthrough via USB PC
  Serial.setRxBufferSize(1024);
  
  uart_dev_t * dev = (volatile uart_dev_t *)(DR_REG_UART_BASE) ;
  dev->conf1.rxfifo_full_thrhd = 1 ;  // set the number of char received on Serial to 1 before generating an interrupt (original value is 112 and is set by esp32-hal-uart.c)
                                      // this increase the number of interrupts but it allows to forward the char to Serial2 faster
  //Serial.print(" setup: rxfifo size before interrupt="); Serial.println(dev->conf1.rxfifo_full_thrhd) ;
  
    // initialise le port série vers grbl
  Serial2.begin(115200, SERIAL_8N1, SERIAL2_RXPIN, SERIAL2_TXPIN); // initialise le port série vers grbl
  Serial2.setRxBufferSize(1024);
  pinMode (SERIAL2_RXPIN, INPUT_PULLUP ); // added to force a level when serial wire is not connected
  pinMode(TFT_LED_PIN , OUTPUT) ;
  digitalWrite(TFT_LED_PIN , HIGH) ;
  
  initButtons() ; //initialise les noms des boutons et les boutons pour chaque page.
  tftInit() ; // init screen and touchscreen, set rotation and calibrate
  if (! spiffsInit() ) {   // try to load the cmd in memory when the files exist in spiffs 
    fillMsg(_SPIFFS_FORMATTED , SCREEN_NORMAL_TEXT ) ;
  } else {
    if (! cmdNameInit() ) {
      fillMsg(_CMD_NOT_LOADED ) ;
    }
  }
  
//  listSpiffsDir( "/", 0 );   // uncomment to see the SPIFFS content
  preferences.begin("savedData") ; //define the namespace for saving preferences (used for saving WIFI parameters, and z coord for change tool)
  grblLink = preferences.getChar("grblLink", GRBL_LINK_SERIAL) ; // retrieve the last used way of communication with GRBL
  dirLevel = -1 ;   // negative value means that SD card has to be uploaded

  nunchuk_init() ; 
  prevPage = _P_NULL ;     
  currentPage = _P_INFO ;
  updateFullPage = true ;
  // en principe les données pour les buttons sont initialisés automatiquement à 0
  //drawFullPage( ) ;
  initWifi() ;
  if ( (wifiType == ESP32_ACT_AS_STATION ) || (wifiType == ESP32_ACT_AS_AP ) ) {
    telnetInit() ;
  }  
  // to debug
//  grblLastMessage[0]= 0x80 ;
//  grblLastMessage[1]= 0x81 ;
//  grblLastMessage[2]= 0x82 ;
//  grblLastMessage[3]= 0x83 ;
  clearScreen() ;
  if (grblLink == GRBL_LINK_SERIAL) {
      while (Serial2.available()>0) Serial2.read() ; // clear the serial2 buffer
      toGrbl( (char) 0x18 ) ;  // send a soft reset
      delay(100);
  } else if (grblLink == GRBL_LINK_TELNET) {
      telnetGrblInit(); // this start the connection with grbl over telnet.
  }
  if (grblLink == GRBL_LINK_BT) {
      btGrblInit();  // this start the connection over Bluetooth
  }
  // Configure Grbl
  toGrbl("$10=3\n\r");
  //Serial2.println("$10=3");   // $10=3 is used in order to get available space in GRBL buffer in GRBL status messages; il also means we are asking GRBL to sent always MPos.
  delay(200);    // wait that all char are sent
  //while (Serial2.availableForWrite() != 0x7F ) ;                        // wait that all char are sent 
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
//#if defined ( ESP32_ACT_AS_STATION ) || defined (ESP32_ACT_AS_AP)
  if ( wifiType > 0) {   // handle the wifi if foreseen
    processWifi();
    checkTelnetConnection();
    boolean tempTelnetIsConnected = telnetIsConnected() ;
    if ( statusPrinting == PRINTING_FROM_TELNET && !tempTelnetIsConnected ){
      statusPrinting = PRINTING_STOPPED ;
      fillMsg(_TELENET_DISCONNECTED );
    }
    statusTelnetIsConnected = tempTelnetIsConnected ;
  } 
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

  if (parseGrblFilesStatus == PARSING_FILE_NAMES_DONE) {
    parseGrblFilesStatus = PARSING_FILE_NAMES_BLOCKED ; // avoid further execute
    executeGrblEndOfFileReading(); // Change the display based on the files being read 
  }

  if (newGrblStatusReceived == true) {
    if ( ( runningFromGrblSd ) && (machineStatus[0] == 'H' ) && ( statusPrinting == PRINTING_STOPPED || statusPrinting == PRINTING_FROM_GRBL ) ){
      statusPrinting = PRINTING_FROM_GRBL_PAUSED ;
      updateFullPage = true ; // We want to update the buttons
    } else if ( ( runningFromGrblSd ) && (machineStatus[0] != 'H' ) && ( statusPrinting == PRINTING_STOPPED || statusPrinting == PRINTING_FROM_GRBL_PAUSED ) ){
      statusPrinting = PRINTING_FROM_GRBL ;
      updateFullPage = true ; // We want to update the buttons
    } else if ( ( runningFromGrblSd == false ) && ( statusPrinting == PRINTING_FROM_GRBL || statusPrinting == PRINTING_FROM_GRBL_PAUSED)){
     statusPrinting = PRINTING_STOPPED ;
     updateFullPage = true ; // We want to update the buttons 
    } else if( statusPrinting == PRINTING_FROM_SD  && machineStatus[0] == 'H' ) { // If printing from SD and GRBL is paused
      // set PRINTING_PAUSED
      statusPrinting = PRINTING_PAUSED ;
      updateFullPage = true ; // We want to get the resume button 
    } else if( statusPrinting == PRINTING_PAUSED  && machineStatus[0] != 'H' ) { // If printing from SD and GRBL is paused
      // set PRINTING_PAUSED
      statusPrinting = PRINTING_STOPPED ;
      updateFullPage = true ; // We want to get the resume button 
    } else if ( currentPage == _P_INFO || currentPage == _P_MOVE || currentPage == _P_SETXYZ || currentPage == _P_SETUP || currentPage == _P_TOOL 
              || currentPage == _P_OVERWRITE || currentPage == _P_COMMUNICATION) { //force a refresh if a message has been received from GRBL and we are in a info screen or in a info screen
        updatePartPage = true ;
    } // end else if
  }
      
  newGrblStatusReceived = false ;
  if (lastMsgChanged == true && ( currentPage == _P_INFO || currentPage == _P_MOVE || currentPage == _P_SETXYZ || currentPage == _P_SETUP 
                                || currentPage == _P_TOOL || currentPage == _P_COMMUNICATION ) ) { //force a refresh if a message has been filled
    updatePartPage = true ;
  }
  
  if (  ( updateFullPage ) ) {
    drawFullPage() ; 

  } else if ( updatePartPage ) {   
    drawPartPage() ;                           // si l'écran doit être mis à jour, exécute une fonction plus limitée qui ne redessine pas les boutons        
  }    
  lastMsgChanged = false ; // lastMsgChanged is used in drawPartPage; so, it can not be set on false before
  updateFullPage = false ;
  updatePartPage = false ;
  //static char prevMachine ;
  //if ( prevMachine != machineStatus[0] ) {
  //  prevMachine = machineStatus[0] ;
  //  Serial.println( machineStatus ) ;
  //}
  yield();
}


