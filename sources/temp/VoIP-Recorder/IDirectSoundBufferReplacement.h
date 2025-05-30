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
 
#ifndef _IDIRECTSOUNDBUFFERREPLACEMENT_H
#define _IDIRECTSOUNDBUFFERREPLACEMENT_H





class IDirectSoundBufferReplacement : public IDirectSoundBuffer
{
  private:
    IDirectSoundBuffer *mIDirectSoundBufferOriginal;
    DWORD mOutputDataBufferSize;
    DWORD mOutputAudioDataSize;
    DWORD mOutputOffset;
    DWORD mPreviousWritePosition;
    LPVOID mOutputAudioDataPointer;
    DWORD mAudioBlocksBufferIndex;
    DWORD mThreadID;
    DWORD mAudioSliceSize;

    sAudioData *mAudioDataInput;
    DWORD mPacketNumber;

  public:
    IDirectSoundBufferReplacement(IDirectSoundBuffer *pIDirectSoundBufferOriginal, DWORD pInputDataBufferSize);
    IDirectSoundBufferReplacement();
    ~IDirectSoundBufferReplacement();
    static DWORD mIndex;
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);
    HRESULT __stdcall GetCaps(LPDSBCAPS pDSBCaps);
    HRESULT __stdcall GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
    HRESULT __stdcall GetCurrentPosition(LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition);
    HRESULT __stdcall GetVolume(LPLONG pdwVolume);
    HRESULT __stdcall GetPan(LPLONG pdwPan);
    HRESULT __stdcall GetFrequency(LPDWORD pdwFrequency);
    HRESULT __stdcall GetStatus(LPDWORD pdwStatus);
    HRESULT __stdcall Initialize( LPDIRECTSOUND lpDirectSound, LPCDSBUFFERDESC lpcDSBufferDesc);
    HRESULT __stdcall Lock(DWORD dwWriteCursor, DWORD dwWriteBytes, LPVOID *lplpvAudioPtr1, LPDWORD lpdwAudioBytes1, LPVOID *lplpvAudioPtr2, LPDWORD lpdwAudioBytes2, DWORD dwFlags);
    HRESULT __stdcall Play(DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags); 
    HRESULT __stdcall SetCurrentPosition(DWORD dwNewPosition);
    HRESULT __stdcall SetFormat(LPCWAVEFORMATEX lpcfxFormat);
    HRESULT __stdcall SetVolume(LONG lVolume);
    HRESULT __stdcall SetPan(LONG lPan);
    HRESULT __stdcall SetFrequency(DWORD dwFrequency);
    HRESULT __stdcall Stop();
    HRESULT __stdcall Unlock(LPVOID lpvAudioPtr1, DWORD dwAudioBytes1, LPVOID lpvAudioPtr2, DWORD dwAudioBytes2);
    HRESULT __stdcall Restore();
};

#endif