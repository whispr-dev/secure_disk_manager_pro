#ifndef _SKYPEDLLINJECTOR_H
#define _SKYPEDLLINJECTOR_H

#include <windows.h>


#define MAX_BUF_SIZE 1024
#define snprintf _snprintf

#define TARGET1		"Skype.exe"
#define TARGET2		"skype.exe"
#define TARGET3		"SKYPE.EXE"



typedef int (WINAPI *MYSEND)(SOCKET s, const char *buf, int len, int flags);
typedef int (WINAPI *MYRECV)(SOCKET s, const char *buf, int len, int flags);


int WINAPI MySend(SOCKET s, const char *buf, int len, int flags);
int WINAPI MyRecv(SOCKET s, const char *buf, int len, int flags);
void printDebug(char *pMsg, int pMsgLength);

#endif