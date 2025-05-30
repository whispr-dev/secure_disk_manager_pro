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


 
#ifndef _IDIRECTSOUNDCAPTUREREPLACEMENT_H
#define _IDIRECTSOUNDCAPTUREREPLACEMENT_H

class IDirectSoundCaptureReplacement : public IDirectSoundCapture
{
  private:
    IDirectSoundCapture *mIDirectSoundCaptureOriginal;
    DWORD mInputDataBufferSize;
  public:
    IDirectSoundCaptureReplacement(IDirectSoundCapture *pIDirectSoundCaptureOriginal);
    IDirectSoundCaptureReplacement();
    ~IDirectSoundCaptureReplacement();
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppv);
    HRESULT __stdcall CreateCaptureBuffer(LPCDSCBUFFERDESC pcDSCBufferDesc, LPDIRECTSOUNDCAPTUREBUFFER *ppDSCBuffer, LPUNKNOWN pUnkOuter);
    HRESULT __stdcall GetCaps(LPDSCCAPS pDSCCaps);
    HRESULT __stdcall Initialize(LPCGUID pcGuidDevice);
};

#endif

