/*
 * Tool name   : RC4Encryption
 * Description : A tool to encrypt or decrypt file with the RC4 algoritm. 
 *
 * Version     : 0.1
 * OS          : Tested on Microsoft Windows XP
 * Todo        : -
 *
 * Changes     : -
 * 
 *
 */


#include <stdio.h>
#include <string.h>
#include <windows.h>

#define SBOX_SIZE 255
#define MAX_BUF_SIZE 1024
#define snprintf _snprintf




/*
 * Constants, data types and function forward declarations.
 *
 */


unsigned char gSBox[SBOX_SIZE + 1];

int initializeSBox(unsigned char *pKey);
int encryptBuffer(unsigned char *pKey, unsigned char *pPlainText, int pPlainTextLength, unsigned char *pCipherText, int *pCipherTextLength);
int decryptBuffer(unsigned char *pKey, unsigned char *pCipherText, int pCipherTextLength, unsigned char *pPlainText, int *pPlainTextLength);
void printUsage(char *pToolName);





/*
 * Application entry point.
 *
 */


int main(int argc, char **argv)
{
  int lRetVal = 0;
  HANDLE lInputFH = INVALID_HANDLE_VALUE;
  HANDLE lOutputFH = INVALID_HANDLE_VALUE;
  DWORD lFileSize = 0;
  DWORD lBytesRead = 0;
  DWORD lBytesWritten = 0;
  char *lTempPointer = NULL;
  char *lTempPointer2 = NULL;
  int lCipherTextLength = 0;
  char lOutputFileName[MAX_BUF_SIZE + 1];


  /*
   * Encrypt a file
   *
   */

  if (argc >= 4 && !strncmp(argv[1], "-e", 2))
  {
    if ((lInputFH = CreateFile(argv[2],  GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
    {
      if ((lFileSize = GetFileSize(lInputFH, NULL)) > 0 && (lTempPointer = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lFileSize)) != NULL && (lTempPointer2 = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lFileSize + 10)) != NULL)
      {
        if (ReadFile(lInputFH, lTempPointer, lFileSize,  &lBytesRead, NULL))
        {

          /*
           * Encrypt file content and save it in a new file.
           *
           */

          encryptBuffer(argv[3], lTempPointer, lFileSize, lTempPointer2, &lCipherTextLength);

          ZeroMemory(lOutputFileName, sizeof(lOutputFileName));
          snprintf(lOutputFileName, sizeof(lOutputFileName) - 1, "%s.rc4", argv[2]);

          if ((lOutputFH = CreateFile(lOutputFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
		  {
            WriteFile(lOutputFH, lTempPointer2, lCipherTextLength, &lBytesWritten, NULL);
            CloseHandle(lOutputFH);
		  } // if ((lOutputFH = CreateFi...
		} // if (ReadFile(lFileH...
	  } // if ((lFileSize = GetFil...

      CloseHandle(lInputFH);
	} else {
      printf("Unable to open the file \"%s\". Error code is %d\n", argv[2], GetLastError());
	} // if ((lFileHandle = Create..





  /*
   * Encrypt a file
   *
   */

  } else if (argc >= 4 && !strncmp(argv[1], "-d", 2)) {
    if ((lInputFH = CreateFile(argv[2],  GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
    {
      if ((lFileSize = GetFileSize(lInputFH, NULL)) > 0 && (lTempPointer = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lFileSize)) != NULL && (lTempPointer2 = (char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lFileSize + 10)) != NULL)
      {
        if (ReadFile(lInputFH, lTempPointer, lFileSize,  &lBytesRead, NULL))
        {

          /*
           * Encrypt file content and save it in a new file.
           *
           */

          decryptBuffer(argv[3], lTempPointer, lFileSize, lTempPointer2, &lCipherTextLength);

          ZeroMemory(lOutputFileName, sizeof(lOutputFileName));
          snprintf(lOutputFileName, sizeof(lOutputFileName) - 1, "%s.txt", argv[2]);

          if ((lOutputFH = CreateFile(lOutputFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
		  {
            WriteFile(lOutputFH, lTempPointer2, lCipherTextLength, &lBytesWritten, NULL);
            CloseHandle(lOutputFH);
		  } // if ((lOutputFH = CreateFi...
		} // if (ReadFile(lFileH...
	  } // if ((lFileSize = GetFil...

      CloseHandle(lInputFH);
	} else {
      printf("Unable to open the file \"%s\". Error code is %d\n", argv[2], GetLastError());
	} // if ((lFileHandle = Create..

  } else {
    printUsage(argv[0]);
  }




  if (lTempPointer)
    HeapFree(GetProcessHeap(), 0, lTempPointer);

  if (lTempPointer)
    HeapFree(GetProcessHeap(), 0, lTempPointer);

  if (lInputFH)
    CloseHandle(lInputFH);

  if (lOutputFH)
    CloseHandle(lOutputFH);


  return(lRetVal);
}





/*
 * Initialize the RC4 sbox
 *
 */

int initializeSBox(unsigned char *pKey)
{
  int lLength = 0;
  int lCounter1 = 0;
  int lCounter2 = 0;
  int lCounter3 = 0;

  lLength = (int) strlen((const char *) pKey);

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
 * Encrypt a data buffer.
 *
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
 * Decrypt a data buffer
 *
 */

int decryptBuffer(unsigned char *pKey, unsigned char *pCipherText, int pCipherTextLength, unsigned char *pPlainText, int *pPlainTextLength)
{
  return(encryptBuffer(pKey, pCipherText, pCipherTextLength, pPlainText, pPlainTextLength));
}



/*
 * Print tool usage.
 *
 */

void printUsage(char *pToolName)
{
	printf("\n\nUsage : %s -[d|e] filename key\n\n", pToolName);
	printf("%s -e Users.txt M3g4p4nZ3r\t - Encrypt the file \"Users.txt\" with the key M3g4p4nZ3r\n", pToolName);
	printf("%s -d Users.txt M3g4p4nZ3r\t - Decrypt the file \"Users.txt\" with the key M3g4p4nZ3r\n", pToolName);
}