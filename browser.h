#ifndef browser_h
#define browser_h

void HomePage();
void File_Download();
void DownloadFile(String filename) ;
void File_Upload();
void handleFileUpload();
void File_Stream();
void File_Delete();
void SD_file_delete(String filename) ;
void sd_dir();
void printDirectory(const char * dirname, uint8_t levels) ;
void SendHTML_Header();
void SendHTML_Content();
void SendHTML_Stop(); 
void reportError(String textError);
void SelectInput(String heading1, String command, String arg_calling_name , String initialValue);
String file_size(int bytes) ;
void initWifi() ;
void processWifi(void);
void P( char * text) ;
void append_page_header() ;
void append_page_footer();
boolean getWifiIp( char * ipBuf ) ;
void confirmDelete();
void confirmDownload() ;
#endif                                       

