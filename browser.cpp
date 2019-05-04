// to do : 
//         remove Serial
//         run wifi in the same core as normal wifi core in a separate task (can perhaps gives an issue for sharing SPI)
//         allows to configure and run in AP mode instead of STA
//         implement buttons and remove button Home


#include <SPI.h>
#include "SdFat.h"
#include <WiFi.h>              // Built-in
#include <WebServer.h>
#include "config.h"
#include "language.h"
#include "browser.h"
#include "draw.h"
#define MS_WRITE 0b00010101 // we have to define this to open file because there is a conflict between ESP32 and SdFat lib about the code to be used in open cmd
                           // this is the value used by SdFat in version 1.1.0 for read, Write, create, at_end

//#define MS_WRITE 0b1100011 // we have to define this to open file because there is a conflict between ESP32 and SdFat lib about the code to be used in open cmd
                           // this is the value used by SdFat in version 1.0.7
String  webpage = "";
WebServer server(80);

extern SdFat sd;

 
File root ; // used for Directory 


void initWifi() {
#if defined ( ESP32_ACT_AS_STATION )
  blankTft("Connecting to Wifi access point", 5 , 20 ) ; // blank screen and display a text at x, y
  WiFi.begin(MY_SSID , MY_PASSWORD);
  uint8_t initWifiCnt = 40 ;   // maximum 40 retries for connecting to wifi
  while (WiFi.status() != WL_CONNECTED)  { // Wait for the Wi-Fi to connect; max 40 retries
    delay(250); // Serial.print('.');
    printTft("X") ;
    initWifiCnt--;
    if ( initWifiCnt == 0) {
      fillMsg(__WIFI_NOT_FOUND  );
      break;
    }
  }
  //Serial.println("\nConnected to "+WiFi.SSID()+" Use IP address: "+WiFi.localIP().toString()); // Report which SSID and IP is in use
#else if defined (ESP32_ACT_AS_AP)
  WiFi.softAP( MY_SSID , MY_PASSWORD);
  //Serial.println("\nESP has IP address: "+ WiFi.softAPIP().toString()); // Report which SSID and IP is in use
#endif  
  WiFi.setSleep(false);
  //----------------------------------------------------------------------   
  ///////////////////////////// Server Commands 
  server.on("/",         HomePage);
  server.on("/download", File_Download);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
  //server.on("/stream",   File_Stream);
  server.on("/delete",   File_Delete);
  server.on("/dir",      sd_dir);
  server.on("/confirmDelete",      confirmDelete);
  server.on("/confirmDownload",      confirmDownload);
  ///////////////////////////// End of Request commands
  server.begin();
  //Serial.println("HTTP server started");  
  
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void P( char * text) {    // used to debug; print a text and a new line
  Serial.println(text);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void processWifi(void){
  server.handleClient(); // Listen for client connections
}

boolean getWifiIp( char * ipBuf ) {          // return true if wifi status = connected
#if defined ( ESP32_ACT_AS_STATION ) 
  if (WiFi.status() == WL_CONNECTED ) {
    strcpy( ipBuf , WiFi.localIP().toString().c_str() ) ;
    return true ;
  } else {
    ipBuf[0] = 0 ;
    return false ;   
  }
#else if defined ( ESP32_ACT_AS_AP )
  strcpy( ipBuf , WiFi.softAPIP().toString().c_str() ) ;
  return true ;
#endif   
}

// All supporting functions from here...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HomePage(){
  SendHTML_Header();
//  webpage += F("<a href='/download'><button>Download</button></a>");
//  webpage += F("<a href='/upload'><button>Upload</button></a>");
//  webpage += F("<a href='/stream'><button>Stream</button></a>");
//  webpage += F("<a href='/delete'><button>Delete</button></a>");
//  webpage += F("<a href='/dir'><button>Directory</button></a>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}

//--------------------------------------------------------------------------
boolean checkSd() {
//  root.close();
  if ( ! sd.begin(SD_CHIPSELECT_PIN , SD_SCK_MHZ(5)) ) {  
      reportError("Fail to mount SD card - SD card present?" );
      return false;
  }
  if ( ! sd.exists( "/" ) ) { // check if root exist 
     reportError("Root ('/') not found on SD card");   
     return false;
  }
  return true ;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void confirmDelete(){ 
  if (server.args() > 0 ) { // check that arguments were received
    if (server.hasArg("fileName")) SelectInput("Confirm filename to delete","delete","delete" , server.arg(0)); 
  }
}

void confirmDownload(){ 
  if (server.args() > 0 ) { // check that arguments were received
    if (server.hasArg("fileName")) SelectInput("Confirm filename to download","download","download" , server.arg(0)); 
  }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) DownloadFile(server.arg(0));
  }
  else SelectInput("Enter filename to download","download","download","");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DownloadFile(String filename){
  //Serial.print("Download file: ") ;Serial.println(  filename ) ;
  //String fileNameS ;
  //fileNameS = "/" + filename ;
  //const char  * fileNameC ;
  //fileNameC = fileNameS.c_str() ;
  if (checkSd() ) { 
    if (sd.exists( filename.c_str() ) ) {
      //Serial.println("Open file") ;
      File download ;
      download = sd.open( filename.c_str() );
      if (download) {
        //Serial.println("File open successfully") ;
        server.sendHeader("Content-Type", "text/text");
        server.sendHeader("Content-Disposition", "attachment; filename="+filename);
        server.sendHeader("Connection", "close");
        server.streamFile(download, "application/octet-stream");
        download.close(); 
      } else {
        //Serial.println("File not opened") ;
        reportError("Error : could not open file to dowload"); 
      }
      download.close() ;
    } else reportError( "Error: file to download did not exist");
  } 
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Upload(){
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>"); 
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:80%' type='file' name='fupload' id = 'fupload' value=''><br>");
  webpage += F("<br><button class='buttons' style='width:20%' type='submit'>Upload File</button><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  server.send(200, "text/html",webpage);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File UploadFile;
boolean errorWhileUploading ; 
void handleFileUpload(){ // upload a new file to the Filing system
  if (checkSd() ) {
      HTTPUpload& uploadfile = server.upload(); // See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                                // For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
      if(uploadfile.status == UPLOAD_FILE_START) {
        errorWhileUploading = false ;
        String filename = uploadfile.filename;
        sd.remove(filename.c_str());                  // Remove a previous version, otherwise data is appended the file again
        UploadFile.close() ;
        UploadFile = sd.open(filename.c_str() , MS_WRITE);  // Open the file for writing in SPIFFS (create it, if doesn't exist)
        if ( ! UploadFile ) errorWhileUploading = true ;
      }
      else if (uploadfile.status == UPLOAD_FILE_WRITE)
      {
        if(UploadFile) { 
          int32_t bytesWritten = UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
          if ( bytesWritten == -1) errorWhileUploading = true ; 
        } else { 
          errorWhileUploading = true ;
        }
      } 
      else if (uploadfile.status == UPLOAD_FILE_END)
      {
        if(UploadFile && ( errorWhileUploading == false) )          // If the file was successfully created
        {                                    
          UploadFile.close();   // Close the file at the end
          webpage = "";
          append_page_header();
          webpage += F("<h3>File was successfully uploaded</h3>"); 
          webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
          webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br>"; 
          append_page_footer();
          server.send(200,"text/html",webpage);
        } 
        else
        {
          UploadFile.close(); // close for safety ; not sure it is really needed
          reportError("<h3>Could Not Create Uploaded File (write-protected?)</h3>");
        }
      }
  }      
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void sd_dir(){ 
  if (checkSd() ) { 
    root.close() ;
    root = sd.open("/");
    if (root) {
      root.rewind();
      SendHTML_Header();
      //webpage += F("<h3 class='rcorners_m'>SD Card Contents</h3><br>");
      webpage += F("<h3></h3><br>");
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th><th>Delete</th><th>Download</th></tr>");
      printDirectory("/",0);
      webpage += F("</table>");
      SendHTML_Content(); 
    } else {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
      SendHTML_Content();
    }
    root.close();
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   // Stop is needed because no content length was sent
  } 
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printDirectory(const char * dirname, uint8_t levels){
  File root1 = sd.open(dirname);
  char fileName[23] ;
  //String fileNameS ;
  if(!root1){
    root1.close() ;
    return;
  }
  if(!root1.isDir()){
    root1.close() ;
    return;
  }
  root1.rewind();
  //P("print Directory:  dirname is a directory") ;
  File file1 ;
  while(file1.openNext(&root1)){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    file1.getName(fileName , 22);
    //fileNameS = fileName ;
    //P(fileName);
    //if ( file1.isDir()) P("is dir") ;
    if(file1.isDir()){
      webpage += "<tr><td>" +String(fileName)+ "</td><td>Dir</td><td></td><td></td><td></td></tr>";
            //printDirectory(fileName, levels-1);
    } else {
      int bytes = file1.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<tr><td>"  + String(fileName) +  "</td><td>File</td><td>" + fsize + "</td>"; //</tr>";
      //<td onClick="location.href='http://www.stackoverflow.com';"> Cell content goes here </td>
      webpage += "<td><form action='/confirmDelete'> <input type='hidden' name='fileName' value='" + String(fileName) +  "' ><input type='submit' value='Delete' ></form></td>" ;
      webpage += "<td><form action='/confirmDownload'> <input type='hidden' name='fileName' value='" + String(fileName) +  "' ><input type='submit' value='Download' ></form></td>" ;
      webpage +=  "</tr>";
    }   
    file1.close();   
  }
  file1.close();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Delete(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("delete")) SD_file_delete(server.arg(0));
  }
  else SelectInput("Select a File to Delete","delete","delete","");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SD_file_delete(String filename) { // Delete the file 
  if (checkSd() ) { 
    SendHTML_Header();  
    File dataFile = sd.open( filename.c_str() ); //  
    if (dataFile) {
      if (sd.remove( filename.c_str() )) {
        //Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '" +filename+ "' has been erased</h3>"; 
      } else { 
        webpage += "<h3>File '"  +filename+ "' could not be erased !!!!!</h3>";
      }
    } else {
      webpage += "<h3>File '" +filename+ "' does not exist !!!!!</h3>";
    }
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } 
} 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SelectInput(String heading1, String command, String arg_calling_name, String initialValue){
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='hidden' name='"); webpage += arg_calling_name; webpage += F("' value='");  webpage += initialValue ; webpage += F("'>"); 
  webpage += F("<input type='text' name='fileNameBox' value='");  webpage += initialValue ; webpage += F("' disabled > <br><br>"); 
  webpage += F("<input type='submit' name='"); webpage += arg_calling_name; webpage += F("' value='Submit' class='buttons' ><br><br>");
 // webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void reportError(String textError){
  SendHTML_Header();
  webpage += "<br><br>" + textError; 
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}

void append_page_header() {
  webpage  = F("<!DOCTYPE html><html>");
  webpage += F("<head>");
  webpage += F("<title>File Server</title>"); // NOTE: 1em = 16px
  webpage += F("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
  webpage += F("<style>");
  webpage += F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:105%;text-align:center;color:blue;background-color:#F7F2Fd;}");
  webpage += F("ul{list-style-type:none;margin:0.1em;padding:0;border-radius:0.375em;overflow:hidden;background-color:#dcade6;font-size:1em;}");
  webpage += F("li{float:left;border-radius:0.375em;border-right:0.06em solid #bbb;}last-child {border-right:none;font-size:85%}");
  webpage += F("li a{display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:85%}");
  webpage += F("li a:hover{background-color:#EAE3EA;border-radius:0.375em;font-size:85%}");
  webpage += F("section {font-size:0.88em;}");
  webpage += F("h1{color:white;border-radius:0.5em;font-size:1em;padding:0.2em 0.2em;background:#558ED5;}");
  webpage += F("h2{color:orange;font-size:1.0em;}");
  webpage += F("h3{font-size:0.8em;}");
  webpage += F("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:85%;}"); 
  webpage += F("th,td {border:0.06em solid #dddddd;text-align:left;padding:0.3em;border-bottom:0.06em solid #dddddd;}"); 
  webpage += F("tr:nth-child(odd) {background-color:#eeeeee;}");
  webpage += F(".rcorners_n {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:75%;}");
  webpage += F(".rcorners_m {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:50%;color:white;font-size:75%;}");
  webpage += F(".rcorners_w {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:70%;color:white;font-size:75%;}");
  webpage += F(".column{float:left;width:50%;height:45%;}");
  webpage += F(".row:after{content:'';display:table;clear:both;}");
  webpage += F("*{box-sizing:border-box;}");
//  webpage += F("footer{background-color:#eedfff; text-align:center;padding:0.3em 0.3em;border-radius:0.375em;font-size:60%;}");
  webpage += F("button{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:130%;}");
  webpage += F(".buttons {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:80%;}");
//  webpage += F(".buttonsm{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:9%; color:white;font-size:70%;}");
//  webpage += F(".buttonm {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:70%;}");
//  webpage += F(".buttonw {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:40%;color:white;font-size:70%;}");
  webpage += F("a{font-size:75%;}");
  webpage += F("p{font-size:75%;}");
  webpage += F("</style></head><body><h1>File Server</h1>");
//  webpage += F("<ul>");
//  webpage += F("<li><a href='/'>Home</a></li>"); // Lower Menu bar command entries
//  webpage += F("<li><a href='/download'>Download</a></li>"); 
//  webpage += F("<li><a href='/upload'>Upload</a></li>"); 
  //webpage += F("<li><a href='/stream'>Stream</a></li>"); 
//  webpage += F("<li><a href='/delete'>Delete</a></li>"); 
//  webpage += F("<li><a href='/dir'>Directory</a></li>");
//  webpage += F("</ul>"); 
  webpage += F("<a href='/dir'><button>Directory</button></a>");
  //webpage += F("<a href='/download'><button>Download</button></a>");
  webpage += F("<a href='/upload'><button>Upload</button></a>");
//  webpage += F("<a href='/stream'><button>Stream</button></a>");
  //webpage += F("<a href='/delete'><button>Delete</button></a>");
  
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
  
  //webpage += "<footer></footer>" ;
 // webpage += "<footer>&copy;"+String(char(byte(0x40>>1)))+String(char(byte(0x88>>1)))+String(char(byte(0x5c>>1)))+String(char(byte(0x98>>1)))+String(char(byte(0x5c>>1)));
 // webpage += String(char((0x84>>1)))+String(char(byte(0xd2>>1)))+String(char(0xe4>>1))+String(char(0xc8>>1))+String(char(byte(0x40>>1)));
 // webpage += String(char(byte(0x64/2)))+String(char(byte(0x60>>1)))+String(char(byte(0x62>>1)))+String(char(0x70>>1))+"</footer>";
  webpage += F("</body></html>");
}
