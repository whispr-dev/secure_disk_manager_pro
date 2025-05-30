#include <stdio.h>
#include "Hooking.h"
#include "Disasm.h"

void InsertByte(DWORD Addr, unsigned char Byte)
{
	// Check if the calling process owns write access 
	// to this range of memory
	if(!IsBadReadPtr((void*)Addr, (UINT) sizeof(byte)))
		*((byte*) ((DWORD*)Addr)) = Byte;
}


void InsertDword(DWORD Addr, DWORD dWord)
{
	// Check if the calling process owns write access 
	// to this range of memory
	if(!IsBadReadPtr((void*)Addr, (UINT) sizeof(DWORD)))
		*((DWORD*)Addr) = dWord;
}


// Generates a jump to address "To", from address "From"
// From is considered as the memory address of the byte just before jump start
// in memory. In fact, the operand of the jump for a relative jump is To - From - 5
// Sample :
//       |.|jmp|@MyFunc|.|
// size:  1   1   3     1
//        ^ 
//        |
//        +-- From is this address 
void GenJmp(DWORD To, DWORD From)
{
	InsertByte (From+0, 0xe9);			// jmp	...
	InsertDword(From+1, To - From - 5); //		destination - src - 5
}

int ForgeHook(DWORD pAddr, DWORD pAddrToJump, byte **Buffer, char *pFunctionName)
{
  DWORD dSize=0, i=0, CollectedSpace=0, OldProtect=0;
  BYTE* pInstruction;
  DWORD CallGateSize=0;
  FILE *mFH = NULL;

  // Check parameters
  if(!pAddr || !pAddrToJump || !Buffer)
	return 0;

  // Start disassembling...
  pInstruction = (BYTE*)pAddr;
  // Loop until we get enough place to set a jump (5 bytes)
  while(CollectedSpace < SIZEOFJUMP)
  {
	// Get instruction lenght
	GetInstLenght((DWORD*)pInstruction, &dSize);

	// Jump to next instruction
	pInstruction += dSize;
	
	// Update collected space size
	CollectedSpace += dSize;
  }
  
  // Forge call gate :
  // allocate memory for call gate : stores saved bytes + the jump after hijacking zone
  CallGateSize = (CollectedSpace+SIZEOFJUMP) * sizeof(byte);
  (*Buffer) = (byte*) malloc(CallGateSize * sizeof(byte));
  
  if((*Buffer) == NULL)
    return(0); // allocation failed

  // Enforce execute mode for call gate
  VirtualProtect((*Buffer), CallGateSize, PAGE_EXECUTE_READWRITE, &OldProtect);
  FillMemory((*Buffer), CallGateSize, ASMNOP);			// clear call gate with NOPs
  CopyMemory((*Buffer), (void*)pAddr, CollectedSpace);	// copy instructions

  // generate jump to original function + SIZEOFJUMP (strides over jump hook)
  GenJmp( (DWORD)((void*)pAddr) + (DWORD) SIZEOFJUMP, (DWORD) (*Buffer) + (DWORD) CollectedSpace);



  // Forge hook
  // give read write execute read and write rights to memory zone
  VirtualProtect((void*)pAddr, CollectedSpace+SIZEOFJUMP, PAGE_EXECUTE_READWRITE, &OldProtect);
  // clear instructions
  FillMemory((void*)pAddr, CollectedSpace, ASMNOP);
  // generate jump
  GenJmp(pAddrToJump, pAddr);
  // restore previous memory protection
  VirtualProtect((void*)pAddr, CollectedSpace+SIZEOFJUMP, OldProtect, &OldProtect);

  return 1;
}
