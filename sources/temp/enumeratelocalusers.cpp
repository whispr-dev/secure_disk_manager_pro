/*
 * Tool name   : EnumerateLocalUsers
 * Description : Tool to enumerate all existing users on
 *               a victim system.
 * Version     : 0.1
 * OS          : Tested on Microsoft Windows XP
 * Todo        : -
 *
 * Changes     : - 
 * 
 *
 */

#include <windows.h>
#include <WtsApi32.h>
#include <Ntsecapi.h>
#include <shellapi.h>
#include <lm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_SIZE 1024
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)


#pragma comment(lib, "Netapi32.lib")

/*
 * Constants, data types and function forward declarations.
 *
 */



typedef struct _ASTAT_
{
  ADAPTER_STATUS adapt;
  NAME_BUFFER NameBuff[30];
} ASTAT, *PASTAT;

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
} eOSTypes;

char *enumerateLocalUsers(void);
char *addDataToBuffer(char *pOldData, int *pOldDataSize, char *pNewData, int pNewDataSize);



/*
 * Program entry point
 *
 */

int main()
{
  int lRetVal = 0;
  char *lTempBuf = NULL;


  /*
   * Enumerate all local users.
   *
   */

  if ((lTempBuf = enumerateLocalUsers()) != NULL)
  {
    printf("\n\nLocal users\n___________\n\n%s\n\n\n", lTempBuf);
    HeapFree(GetProcessHeap(), 0, lTempBuf);
    lTempBuf = NULL;
  }


  printf("\n\nHit return to stop execution ...\n");
  getc(stdin);

  return(lRetVal);
}





/*
 * Enumerate all local users.
 *
 */

char *enumerateLocalUsers(void)
{
  char *lRetVal = NULL;
  LPUSER_INFO_10 lBuf = NULL;
  LPUSER_INFO_10 lTempBuffer;
  DWORD lLevel = 10;
  DWORD lPrefMaxLen = MAX_PREFERRED_LENGTH;
  DWORD lEntriesRead = 0;
  DWORD lTotalEntries = 0;
  DWORD lResumeHandle = 0;
  DWORD lCounter;
  NET_API_STATUS lStatus;
  LPTSTR pszServerName = NULL;
  char lUserName[MAX_BUF_SIZE + 1];
  char lFullUserName[MAX_BUF_SIZE + 1];
  char lTemp[MAX_BUF_SIZE + 1];
  int lBufSize = 0;

  do 
  {
    lStatus = NetUserEnum(pszServerName, lLevel, FILTER_NORMAL_ACCOUNT, // global users
                         (LPBYTE*) &lBuf, lPrefMaxLen, &lEntriesRead, &lTotalEntries, &lResumeHandle);

    if ((lStatus == NERR_Success) || (lStatus == ERROR_MORE_DATA))
    {
      if ((lTempBuffer = lBuf) != NULL)
      {
        for (lCounter = 0; lCounter < lEntriesRead; lCounter++)
        {
          if (lTempBuffer == NULL)
            break;

          ZeroMemory(lFullUserName, sizeof(lFullUserName));
          ZeroMemory(lUserName, sizeof(lUserName));
		  wcstombs(lFullUserName, lTempBuffer->usri10_full_name, sizeof(lFullUserName));
		  wcstombs(lUserName, lTempBuffer->usri10_name, sizeof(lUserName));

		  _snprintf(lTemp, sizeof(lTemp) - 1, "%-20s -\t%s\n", lUserName, lFullUserName);
          lRetVal = addDataToBuffer(lRetVal, &lBufSize, lTemp, strlen(lTemp));

          lTempBuffer++;
        } // for (lCounter = 0; lCo...
      } // if ((lTempBuffer = pBu...
    } // if ((lStatus == NE...

    if (lBuf != NULL)
    {
      NetApiBufferFree(lBuf);
      lBuf = NULL;
    }
  } while (lStatus == ERROR_MORE_DATA); // end do

  if (lBuf != NULL)
    NetApiBufferFree(lBuf);

  return(lRetVal);
}






/*
 *
 *
 */

char *addDataToBuffer(char *pOldData, int *pOldDataSize, char *pNewData, int pNewDataSize)
{
  char *lRetVal = NULL;


  /*
   * Allocate the Buffer on the heap.
   *
   */

  if (pOldData == NULL)
  {
    if ((lRetVal = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pNewDataSize + 1)) != NULL)
	{
      CopyMemory(lRetVal, pNewData, pNewDataSize);
	  *pOldDataSize = pNewDataSize;
	} // if (lRetVal = ...






  /*
   * Rellocate the Buffer on the heap. In case of failure
   * release all the allocated buffer area.
   *
   */

  } else {
    if ((lRetVal = (char *) HeapReAlloc(GetProcessHeap(), 0, pOldData, *pOldDataSize + pNewDataSize )) != NULL)
	{
      ZeroMemory(&lRetVal[*pOldDataSize], pNewDataSize);
	  CopyMemory(&lRetVal[*pOldDataSize], pNewData, pNewDataSize);
      *pOldDataSize += pNewDataSize;
	} else {
      HeapFree(GetProcessHeap(), 0, pOldData);
      lRetVal = NULL;
	}
  } // if (pOldBuffer == ...

  return(lRetVal);
}