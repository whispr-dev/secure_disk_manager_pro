#ifndef _ENTRY_POINT_HOOKING_FUNCTIONS_H
#define _ENTRY_POINT_HOOKING_FUNCTIONS_H

#include <windows.h>

#define  SIZEOFJUMP		5
#define  ASMNOP			0x90


void InsertByte(DWORD Addr, unsigned char Byte);
void InsertDword(DWORD Addr, DWORD dWord);
void GenJmp(DWORD To, DWORD From);
int ForgeHook(DWORD pAddr, DWORD pAddrToJump, byte **Buffer, char *pFunctionName);
#endif