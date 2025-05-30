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






#include <windows.h>
#include <stdio.h>
#include <DSound.h>
#include "VoIP_Recorder.h"
 

#ifndef _IDIRECTSOUNDCAPTUREBUFFERREPLACEMENT_H
#define _IDIRECTSOUNDCAPTUREBUFFERREPLACEMENT_H

class IDirectSoundCaptureBufferReplacement : public IDirectSoundCaptureBuffer
{
  private:
    IDirectSoundCaptureBuffer *mIDirectSoundCaptureBufferOriginal;
    DWORD mInputDataBufferSize;
    DWORD mInputOffset;
    DWORD mPreviousReadPosition;
    LPVOID mInputAudioDataPointer1;
    DWORD mInputAudioDataSize1;
    LPVOID mInputAudioDataPointer2;
    DWORD mInputAudioDataSize2;
    DWORD mThreadID;
    DWORD mAudioSliceSize;

    sAudioData *mAudioDataInput;
    DWORD mPacketNumber;

  public:
    IDirectSoundCaptureBufferReplacement();
    IDirectSoundCaptureBufferReplacement(IDirectSoundCaptureBuffer *pIDirectSoundCaptureBufferOriginal, DWORD pInputDataBufferSize);
    IDirectSoundCaptureBufferReplacement::~IDirectSoundCaptureBufferReplacement();
    HRESULT __stdcall Lock(DWORD dwOffset, DWORD dwBytes, LPVOID * ppvAudioPtr1, LPDWORD  pdwAudioBytes1, LPVOID * ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
    HRESULT __stdcall Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
    HRESULT __stdcall Start(DWORD dwFlags);
    HRESULT __stdcall Stop();


    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall GetCaps(LPDSCBCAPS pDSCBCaps);
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);
    HRESULT __stdcall Initialize(LPDIRECTSOUNDCAPTURE pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc);
    HRESULT __stdcall GetCurrentPosition(LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition);
    HRESULT __stdcall GetStatus(LPDWORD pdwStatus);
    HRESULT __stdcall GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
};

#endif