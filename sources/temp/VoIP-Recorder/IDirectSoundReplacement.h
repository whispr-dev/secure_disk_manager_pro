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



#ifndef _IDIRECTSOUNDREPLACEMENT_H
#define _IDIRECTSOUNDREPLACEMENT_H

class IDirectSoundReplacement : public IDirectSound
{
  private:
    IDirectSound *mIDirectSoundOriginal;
  public:
    IDirectSoundReplacement();
    IDirectSoundReplacement(IDirectSound *mIDirectSoundOriginal); // constructor
    ~IDirectSoundReplacement();
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *ppv);
    HRESULT __stdcall GetCaps(LPDSCAPS pDSCaps);
    HRESULT __stdcall Initialize(LPCGUID pcGuidDevice);


    HRESULT __stdcall DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER * ppDSBufferDuplicate);
    HRESULT __stdcall SetCooperativeLevel(HWND hwnd, DWORD dwLevel);
    HRESULT __stdcall Compact();
    HRESULT __stdcall GetSpeakerConfig(LPDWORD pdwSpeakerConfig);
    HRESULT __stdcall CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER * ppDSBuffer, LPUNKNOWN pUnkOuter);
    HRESULT __stdcall SetSpeakerConfig(DWORD dwSpeakerConfig);
};

#endif