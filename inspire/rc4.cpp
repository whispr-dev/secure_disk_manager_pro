#include <stdio.h>
#include <string.h>
#include <windows.h>

#define SBOX_SIZE 255
#define MAX_BUF_SIZE 1024
#define snprintf _snprintf

unsigned char gSBox[SBOX_SIZE + 1];



/*
 * initialize the RC4 sbox
 */

int initializeSBox(unsigned char *pKey)
{
  int lLength = 0;
  int lCounter1 = 0;
  int lCounter2 = 0;
  int lCounter3 = 0;

  lLength = strlen((const char *) pKey);

  for (lCounter1 = 0; lCounter1 < 256; lCounter1++)
    gSBox[lCounter1] = lCounter1;

  for (lCounter1 = 0; lCounter1 < 256; lCounter1++)
  {
    lCounter2 = (lCounter2 + gSBox[lCounter1] + pKey[lCounter1 % lLength]) % 256;
    lCounter3 = gSBox[lCounter1];
    gSBox[lCounter1] = gSBox[lCounter2];
    gSBox[lCounter2] = lCounter3;
  }

  return(0);
}



/*
 * encrypt a data buffer
 */

int encryptBuffer(unsigned char *pKey, unsigned char *pPlainText, int pPlainTextLength, unsigned char *pCipherText, int *pCipherTextLength)
{
  int lCounter1 = 0;
  int lCounter2 = 0;
  int lCounter3 = 0;
  int lCounter4 = 0;
  char *lPlainText = NULL;
  unsigned char lKey[MAX_BUF_SIZE + 1];
  int lRetVal = 0;



  if ((lPlainText = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pPlainTextLength + 1)) == NULL)
  {
    lRetVal = 1;
	goto END;
  }

  ZeroMemory(lKey, sizeof(lKey));

  CopyMemory((char *) lKey, (char *) pKey, sizeof(lKey) - 1);
  CopyMemory((char *) lPlainText, (char *) pPlainText, pPlainTextLength);

  initializeSBox(lKey);
  *pCipherTextLength = 0;


  for (lCounter1 = 0; lCounter1 < pPlainTextLength; lCounter1++)
  {
    lCounter2 = (lCounter2 + 1) % 256;
    lCounter3 = (lCounter3 + gSBox[lCounter2]) % 256;
    lCounter4 = gSBox[lCounter2];
    gSBox[lCounter2] = gSBox[lCounter3];
    gSBox[lCounter3] = lCounter4;

    lCounter4 = (gSBox[lCounter2] + gSBox[lCounter3]) % 256;
    pCipherText[lCounter1] = lPlainText[lCounter1] ^ gSBox[lCounter4];
  }

  *pCipherTextLength = lCounter1;

END:

  if (lPlainText != NULL)
    HeapFree(GetProcessHeap(), 0, lPlainText);

  return(lRetVal);
}


/*
 * decrypt a data buffer
 */

int decryptBuffer(unsigned char *pKey, unsigned char *pCipherText, int pCipherTextLength, unsigned char *pPlainText, int *pPlainTextLength)
{
  return(encryptBuffer(pKey, pCipherText, pCipherTextLength, pPlainText, pPlainTextLength));
}


int main(int argc, char **argv)
{
  int lRetVal = 0;
  char lPlainText[256];
  int lPlainTextLength = 0;
  char lCipherText[256];
  int lCipherTextLength = 0;
  int lCounter = 0;
  char lKey[256];

  /*
   * Initializing all variables
   */

  ZeroMemory(lPlainText, sizeof(lPlainText));
  ZeroMemory(lCipherText, sizeof(lCipherText));
  ZeroMemory(lKey, sizeof(lKey));

  snprintf(lKey, sizeof(lKey) - 1, "megapanzer");
  snprintf(lPlainText, sizeof(lPlainText) - 1, "hello world!");


  /*
   * encryption
   */
  printf("Encrypting string \"%s\" now.\n", lPlainText);
  encryptBuffer((unsigned char *) lKey, (unsigned char *) lPlainText, strlen(lPlainText), (unsigned char *) lCipherText, &lCipherTextLength);
  printf("New cihpertext \"%s\"has %d characters\n", lCipherText, lCipherTextLength);


  /*
   * decryption
   */

  ZeroMemory(lPlainText, sizeof(lPlainText));
  printf("Decrypting string \"%s\" now.\n", lCipherText);
  decryptBuffer((unsigned char *) lKey, (unsigned char *) lCipherText, lCipherTextLength, (unsigned char *) lPlainText, &lPlainTextLength);
  printf("Unencrypted string : \"%s\"\n", lPlainText);



  return(lRetVal);
}
