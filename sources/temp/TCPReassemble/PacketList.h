
#ifndef __PACKETLIST__
#define __PACKETLIST__


/*
 * Type declarations.
 *
 */

typedef struct PACKET
{
  unsigned long SeqNum;
  unsigned char Data[MAX_PAYLOAD + 1];
  int DateLen;
} PACKET, *PPACKET;


typedef struct CONNODE 
{
//  PPACKET Data;
  int first;

  char ID[MAX_ID_LEN + 1];
  time_t Created;
  int DataLen;
  unsigned char *Data;
  struct CONNODE *prev;
  struct CONNODE *next;
} CONNODE, *PCONNODE, **PPCONNODE;




/*
 * Function forward declarations.
 *
 */
PCONNODE InitConnectionList();
void AddConnectionToList(PPCONNODE pConNodes, char *pID);
PCONNODE ConnectionNodeExists(PCONNODE pConNodes, char *pID);
int CountConnectionNodes(PCONNODE pConNodes);
void ConnectionDeleteNode(PPCONNODE pConNodes, char *pID);
int ConnectionCountNodes(PCONNODE pConNodes);
void ConnectionAddData(PCONNODE pNode, char *pData, int pDataLen);
void RemoveOldConnections(PPCONNODE pConNodes);
#endif