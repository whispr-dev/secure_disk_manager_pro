#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "TCPReassemble.h"
#include "PacketList.h"


extern CRITICAL_SECTION gCSConnectionsList;





/*
 *
 *
 */
PCONNODE InitConnectionList()
{
  PCONNODE lFirstSysNode = NULL;

  EnterCriticalSection(&gCSConnectionsList);

  if ((lFirstSysNode = (PCONNODE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CONNODE))) != NULL)
  {
    lFirstSysNode->first = 1;
    lFirstSysNode->next = NULL;
	lFirstSysNode->prev = NULL;
  } // if (tmp = ma...

  LeaveCriticalSection(&gCSConnectionsList);

  return(lFirstSysNode);
}




/*
 *
 *
 */
void AddConnectionToList(PPCONNODE pConNodes, char *pID)
{
  PCONNODE lTmpNode = NULL;


  EnterCriticalSection(&gCSConnectionsList);

  if (pConNodes != NULL && *pConNodes != NULL && pID != NULL)
  {
    if ((lTmpNode = (PCONNODE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CONNODE))) != NULL)
    {
      strncpy(lTmpNode->ID, pID, sizeof(lTmpNode->ID));
	  lTmpNode->Created = time(NULL);
      
      lTmpNode->prev = NULL;
      lTmpNode->first = 0;
      lTmpNode->next = *pConNodes;
      ((PCONNODE) *pConNodes)->prev = lTmpNode;
      *pConNodes = lTmpNode;
	}
  } // if (pSysMAC != NUL...

  LeaveCriticalSection(&gCSConnectionsList);
}




/*
 *
 *
 */
PCONNODE ConnectionNodeExists(PCONNODE pConNodes, char *pID)
{
  PCONNODE lRetVal = NULL;
  PCONNODE lTmpCon;
  int lCount = 0;


  EnterCriticalSection(&gCSConnectionsList);

  if (pID != NULL && (lTmpCon = pConNodes) != NULL)
  {

    /*
     * Go to the end of the list
     *
     */
    for (lCount = 0; lCount < MAX_CONNECTION_COUNT; lCount++)
    {
      if (lTmpCon != NULL)
      {
        if (!strncmp(lTmpCon->ID, pID, sizeof(lTmpCon->ID)))
		{
          lRetVal = lTmpCon;
		  break;
		} // if (strncmp(l..
      } // if (lTmp...

      if((lTmpCon = lTmpCon->next) == NULL)
        break;

    } // for (lCount = ...
  } // if (pMAC != ...


  LeaveCriticalSection(&gCSConnectionsList);

  return(lRetVal);
}



/*
 *
 *
 */
void ConnectionDeleteNode(PPCONNODE pConNodes, char *pConID)
{
  int lRetVal = 0;
  PCONNODE lTmpNode, q;
  char lTemp[MAX_BUF_SIZE + 1];

  if (pConNodes != NULL && *pConNodes != NULL && pConID != NULL )
  {
    ZeroMemory(lTemp, sizeof(lTemp));
    EnterCriticalSection(&gCSConnectionsList);

    // Remove first node.
    if (!strncmp(((PCONNODE)*pConNodes)->ID, pConID, MAX_ID_LEN) && ((PCONNODE)*pConNodes)->first == 0)
    {
      lTmpNode = *pConNodes;
      *pConNodes = ((PCONNODE) *pConNodes)->next;
      ((PCONNODE) *pConNodes)->prev = NULL;

      if (lTmpNode->Data != NULL)
        HeapFree(GetProcessHeap(), 0, lTmpNode->Data);

      HeapFree(GetProcessHeap(), 0, lTmpNode);
      goto END;
    } // if (!strncmp(((PCONNODE...


    q = (PCONNODE) *pConNodes;

    while(q->next != NULL && q->next->next != NULL && q->first == 0)
    {
      if (!strncmp(q->ID, pConID, MAX_ID_LEN))   
      {

        lTmpNode = q->next;
        q->next = lTmpNode->next;
        lTmpNode->next->prev = q;


		if (lTmpNode->Data != NULL && lTmpNode->DataLen)
		{
          printf("|%s|\n", lTmpNode->Data);
          HeapFree(GetProcessHeap(), 0, lTmpNode->Data);
		}

        HeapFree(GetProcessHeap(), 0, lTmpNode);
        goto END;
      } // if (!strncmp(q->...
      q = q->next;
    } // while(q->ne...
  } // if (pConNodes != NU...

END:

  LeaveCriticalSection(&gCSConnectionsList);

  return;
}



/*
 *
 *
 */
void ConnectionAddData(PCONNODE pNode, char *pData, int pDataLen)
{
  EnterCriticalSection(&gCSConnectionsList);

  if (pNode != NULL && pData != NULL && pDataLen > 0)
  {
    /*
     * The first data chunk. Allocate memory and save
     * the copy there.
     *
     */
    if (pNode->Data == NULL)
	{
      if ((pNode->Data = (unsigned char *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pDataLen+3)) != NULL)
	  {
        memset(pNode->Data, '.', pDataLen + 2);
        CopyMemory(pNode->Data, pData, pDataLen);
        pNode->DataLen = pDataLen+2;
	  } // if ((pNode...



    /*
     * Append the new data block to the existing
     * data.
     *
     */
    }
	else
	{
      if (pNode->DataLen + pDataLen + 3 > MAX_CONNECTION_VOLUME)
        pNode->DataLen += pDataLen + 3;
      else if ((pNode->Data = (unsigned char *) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pNode->Data, pNode->DataLen + pDataLen + 3)) != NULL)
	  {
        memset(&pNode->Data[pNode->DataLen-1], '.', pDataLen + 2);
        CopyMemory(&pNode->Data[pNode->DataLen-1], pData, pDataLen);
        pNode->DataLen += (pDataLen+2);
	  } // if ((pNode->Data...
	} // if (pNode...
  } // if (pNode != NUL...

  LeaveCriticalSection(&gCSConnectionsList);
}




/*
 *
 *
 */
int ConnectionCountNodes(PCONNODE pConNodes)
{   
  int lRetVal = 0;

  EnterCriticalSection(&gCSConnectionsList);
  while(pConNodes != NULL)
  {
    pConNodes = pConNodes->next;
    lRetVal++;
  } // while(pConNodes !...
  EnterCriticalSection(&gCSConnectionsList);

  return(lRetVal);
}




/*
 * Remove all entries that contain more than MAX_CONNECTION_VOLUME bytes 
 * or are oldern than TCP_MAX_INACTIVITY.
 *
 */
void RemoveOldConnections(PPCONNODE pConNodes)
{
  PCONNODE lTmpNode, q;
  char lTemp[MAX_BUF_SIZE + 1];
  time_t lNow = time(NULL);


  EnterCriticalSection(&gCSConnectionsList);

  if (pConNodes != NULL && *pConNodes != NULL && ((PCONNODE)*pConNodes)->first == 0)
  {
    /*
     * The first entry in the linked list.
     *
     */

    if (lNow - ((PCONNODE)*pConNodes)->Created > TCP_MAX_INACTIVITY ||
		((PCONNODE)*pConNodes)->DataLen > MAX_CONNECTION_VOLUME)
    {
      lTmpNode = *pConNodes;
      *pConNodes = ((PCONNODE) *pConNodes)->next;
      ((PCONNODE) *pConNodes)->prev = NULL;

      if (lTmpNode->Data != NULL)
        HeapFree(GetProcessHeap(), 0, lTmpNode->Data);

      HeapFree(GetProcessHeap(), 0, lTmpNode);
      goto END;
    } // if (lNow - ((PCON...


    q = (PCONNODE) *pConNodes;
    while(q->next != NULL && q->next->next != NULL && q->first == 0)
    {
      if (lNow - q->Created > TCP_MAX_INACTIVITY || q->DataLen > MAX_CONNECTION_VOLUME)  
      {
        lTmpNode = q->next;
        q->next = lTmpNode->next;
        lTmpNode->next->prev = q;

        if (lTmpNode->Data != NULL)
          HeapFree(GetProcessHeap(), 0, lTmpNode->Data);

        HeapFree(GetProcessHeap(), 0, lTmpNode);
        goto END;
      } // if (!strncmp(q->...

      q = q->next;
    } // while(q->ne...


  } // if (pConNodes != NU...
END:

  LeaveCriticalSection(&gCSConnectionsList);
}