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
#include <DSound.h>



#ifndef DWORD_PTR
  #define DWORD_PTR DWORD *
#endif

#ifndef _SKYPE_2_FUNCTIONS_H
#define _SKYPE_2_FUNCTIONS_H

MMRESULT WINAPI MywaveInClose(HWAVEIN hwi);
MMRESULT WINAPI MywaveOutClose(HWAVEOUT hwo);

MMRESULT WINAPI MywaveOutOpen(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
MMRESULT WINAPI MywaveInOpen(LPHWAVEIN phwi, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);


typedef MMRESULT (CALLBACK *MYWAVEINCLOSE)(HWAVEIN hwi);
typedef MMRESULT (CALLBACK *MYWAVEOUTCLOSE)(HWAVEOUT hwo);
typedef MMRESULT (CALLBACK *MYWAVEINOPEN)(LPHWAVEIN phwi, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);
typedef MMRESULT (CALLBACK *MYWAVEOUTOPEN)(LPHWAVEOUT phwo, UINT_PTR uDeviceID, LPWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwCallbackInstance, DWORD fdwOpen);

typedef void (CALLBACK *ORIGINALWAVEINCALLBACK)(HWAVEIN hwo, UINT pMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
typedef void (CALLBACK *ORIGINALWAVEOUTCALLBACK)(HWAVEOUT hwo, UINT pMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

#endif