
#define HAVE_REMOTE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <iphlpapi.h>
#include <Shlwapi.h>
#include <windows.h>
#include "TCPReassemble.h"
#include "PacketList.h"

extern PCONNODE gConnectionList;

void POISONSnifferCallback(unsigned char *pScanParams, struct pcap_pkthdr *header, unsigned char *pkt_data)
{
//  SYSTEMNODE lPktDat;
  PETHDR pEthHdr = (PETHDR) pkt_data;
  PIPHDR pIPHdr = NULL;
  PTCPHDR pTCPHdr = NULL;
//  PUDPHDR pUDPHdr = NULL;
//  PDNS_HDR lDNS = NULL;
//  PDNS_QUERY lQuery = NULL;
  int lIPHdrLen = 0;
  int lTCPHdrLen = 0;
  int lTCPDataLen = 0;
  char lData[1500 + 1];
  char lRealData[1500 + 1];
  int lTotLen = 0;
  char *lTempPtr = NULL;
  char lTemp[MAX_BUF_SIZE + 1];
  char lTemp2[MAX_BUF_SIZE + 1];
  char lSMAC[MAX_BUF_SIZE + 1];
  unsigned short lSrcPort = 0;
  unsigned short lDstPort = 0;
  unsigned long lSeqNr = 0;
  unsigned long lSeqAckNr = 0;


  char lSrcIP[MAX_BUF_SIZE + 1];
  char lDstIP[MAX_BUF_SIZE + 1];
  char lIDClient[MAX_BUF_SIZE + 1];
  char lIDServer[MAX_BUF_SIZE + 1];
  int lNumCon = 0;
  PCONNODE lTmpNode = NULL;

  int lBufLen = 0; 
  int lRetVal = 0;
  unsigned char *lDataPipe = NULL;
//  PSCANPARAMS lTmpParams = (PSCANPARAMS) pScanParams;
//  SCANPARAMS lScanParams;
  unsigned int lSeqNum = 0;
  HANDLE lFH = INVALID_HANDLE_VALUE;

  /*
   *
   *
   */

//  ZeroMemory(&lScanParams, sizeof(lScanParams));
//  CopyMemory(&lScanParams, lTmpParams, sizeof(lScanParams));




  /*
   *
   *
   */
  if (htons(pEthHdr->ether_type) == 0x0800)
  {
    pIPHdr = (PIPHDR) (pkt_data + 14);
    lIPHdrLen = (pIPHdr->ver_ihl & 0xf) * 4;


    /*
     * TCP data
     *
     */
    if (pIPHdr->proto == 6) 
    {
      ZeroMemory(lSrcIP, sizeof(lSrcIP));
      ZeroMemory(lDstIP, sizeof(lDstIP));
      ZeroMemory(lIDClient, sizeof(lIDClient));
      ZeroMemory(lIDServer, sizeof(lIDServer));

      lTotLen = ntohs(pIPHdr->tlen);
      pTCPHdr = (PTCPHDR) ((u_char*) pIPHdr + lIPHdrLen);

      lTCPHdrLen = pTCPHdr->doff * 4;
      lTCPDataLen = lTotLen - lIPHdrLen - lTCPHdrLen;

      lSrcPort = ntohs(pTCPHdr->sport);
      lDstPort = ntohs(pTCPHdr->dport);

      snprintf(lDstIP, sizeof(lDstIP) - 1, "%d.%d.%d.%d", pIPHdr->daddr.byte1, 
               pIPHdr->daddr.byte2, pIPHdr->daddr.byte3, pIPHdr->daddr.byte4);

      snprintf(lSrcIP, sizeof(lSrcIP) - 1, "%d.%d.%d.%d", pIPHdr->saddr.byte1, 
               pIPHdr->saddr.byte2, pIPHdr->saddr.byte3, pIPHdr->saddr.byte4);

      lSeqNr = ntohl(pTCPHdr->seq);
      lSeqAckNr = ntohl(pTCPHdr->ack_seq);


      snprintf(lIDClient, sizeof(lIDClient) - 1, "%s:%d->%s:%d", lDstIP, lDstPort, lSrcIP, lSrcPort);  
      snprintf(lIDServer, sizeof(lIDServer) - 1, "%s:%d->%s:%d", lSrcIP, lSrcPort, lDstIP, lDstPort);


      /*
       * The data is attached to the connection buffer.
       *
       */
	  if (lTCPDataLen > 0)
      {
        ZeroMemory(lTemp, sizeof(lTemp));
        ZeroMemory(lTemp2, sizeof(lTemp2));
        ZeroMemory(lData, sizeof(lData));
        ZeroMemory(lRealData, sizeof(lRealData));


        /*
         * Copy and stringify the payload
         *
         */
        if (lTCPDataLen > MAX_PAYLOAD)
        {
          strncpy(lData, (unsigned char *) pTCPHdr + lTCPHdrLen, MAX_PAYLOAD);
          Stringify(lData, MAX_PAYLOAD, lRealData);
        }
        else if (lTCPDataLen > 0) 
        {
          strncpy(lData, (unsigned char *) pTCPHdr + lTCPHdrLen, lTCPDataLen);
          Stringify(lData, lTCPDataLen, lRealData);
        } // if (l...


        /*
         * Archive packet
         *
         */
        if ((lTmpNode = ConnectionNodeExists(gConnectionList, lIDClient)) != NULL)
          ConnectionAddData(lTmpNode, lRealData, strlen(lRealData));

      } // if (lTCPData...


      lNumCon = ConnectionCountNodes(gConnectionList);

      if ((pTCPHdr->syn == 1 && pTCPHdr->ack) || pTCPHdr->fin == 1 || pTCPHdr->rst == 1)
        printf("\n");



      /*
       * TCP status bits SYN + ACK are set. Create a new
       * list entry.
       *
       */
      if (pTCPHdr->syn == 1 && pTCPHdr->ack == 1)
      {
        if (ConnectionNodeExists(gConnectionList, lIDClient) || ConnectionNodeExists(gConnectionList, lIDServer))
        {
          ConnectionDeleteNode(&gConnectionList, lIDClient);
          ConnectionDeleteNode(&gConnectionList, lIDServer);
        } // if (ConnectionNodeExi...


        if (ConnectionNodeExists(gConnectionList, lIDClient) || ConnectionNodeExists(gConnectionList, lIDServer))
          ; //printf("%s/%s exists\n", lIDClient, lIDServer);
        else
        {
          AddConnectionToList(&gConnectionList, lIDClient);
          AddConnectionToList(&gConnectionList, lIDServer);
        } // if (Connectio...


        printf("datalen %d\n", lTCPDataLen);
        printf("Syn %d . . . .  seq# %lu\nAck . %d . . .  ack# %lu\nPsh . . %d . .\nFin . . . %d . \nRst . . . . %d\nCon# %d\n", pTCPHdr->syn, lSeqNr, pTCPHdr->ack, lSeqAckNr, pTCPHdr->psh, pTCPHdr->fin, pTCPHdr->rst, lNumCon);
      } // if (pTCPHd...


      /*
       * TCP status bits FIN or RST are set. Remove the
       * according list entries.
       *
       */
      if (pTCPHdr->fin == 1 || pTCPHdr->rst == 1)
      {
        ConnectionDeleteNode(&gConnectionList, lIDClient);
        ConnectionDeleteNode(&gConnectionList, lIDServer);

        printf("datalen %d\n", lTCPDataLen);
        printf("Syn %d . . . .  seq# %lu\nAck . %d . . .  ack# %lu\nPsh . . %d . .\nFin . . . %d . \nRst . . . . %d\nCon# %d\n", pTCPHdr->syn, lSeqNr, pTCPHdr->ack, lSeqAckNr, pTCPHdr->psh, pTCPHdr->fin, pTCPHdr->rst, lNumCon);
      } // if (pTC...


    } // if (pIPHdr->prot...
  } // if (htons(pEthH...

//RemoveOldConnections(&gConnectionList);


}




/*
 *
 *
 */
int StartSniffer(PSCANPARAMS pScanParams)
{
  int lRetVal = 0;
  pcap_if_t *lAllDevs = NULL;
  pcap_if_t *lDevice = NULL;
  char lTemp[PCAP_ERRBUF_SIZE];
  char lAdapter[MAX_BUF_SIZE + 1];
  char lFilter[MAX_BUF_SIZE + 1];
  int lCounter = 0;
  int lIFCnum = 0;
  pcap_t *lIFCHandle = NULL;
  struct bpf_program lFCode;
  unsigned int lNetMask = 0;
 
  PSCANPARAMS lTmpParams = (PSCANPARAMS) pScanParams;
  SCANPARAMS lScanParams;
SECURITY_ATTRIBUTES lPipeSA = {sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};


  ZeroMemory(&lScanParams, sizeof(lScanParams));
  CopyMemory(&lScanParams, lTmpParams, sizeof(lScanParams));

  /*
   * Open device list.
   *
   */
  if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &lAllDevs, lTemp) != -1)
  {
    ZeroMemory(lAdapter, sizeof(lAdapter));
 

    /*
     * Enum through all available interfaces and pick the
     * right one out.
     *
     */
    for(lCounter = 0, lDevice = lAllDevs; lDevice; lDevice = lDevice->next, lCounter++)
    {
      if (StrStrI(lDevice->name, lScanParams.IFCName)) //pIFCName))
	  {
        strcpy(lAdapter, lDevice->name);
        break;
	  } // if (StrS...
    } // for(lCounter = 0, ...
    pcap_freealldevs(lAllDevs);



    /*
     * Open interface.
     *
     */ 
    if ((lIFCHandle = pcap_open(lAdapter, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, lTemp)) != NULL)
    {

      /* 
       * Compiling + setting the filter
       *
       */
      if (lDevice->addresses != NULL)
        lNetMask = ((struct sockaddr_in *)(lDevice->addresses->netmask))->sin_addr.S_un.S_addr;
      else
        lNetMask = 0xffffff; 

      ZeroMemory(&lFCode, sizeof(lFCode));
      ZeroMemory(lFilter, sizeof(lFilter));
      snprintf(lFilter, sizeof(lFilter) - 1, "tcp and port 80", lScanParams.LocalIP[0], lScanParams.LocalIP[1], lScanParams.LocalIP[2], lScanParams.LocalIP[3]);


      if (pcap_compile(lIFCHandle, &lFCode, lFilter, 1, lNetMask) >= 0)
      {
        if (pcap_setfilter(lIFCHandle, &lFCode) >= 0)
        {
          // We dont need this list anymore.
          pcap_freealldevs(lAllDevs);


          // Start intercepting data packets.
          pcap_loop(lIFCHandle, 0, POISONSnifferCallback, (unsigned char *) &lScanParams);
		} // if (pcap_se...
      } // if (pcap_compile(lIFCH...
	} // if ((lIFCHandle ...
  } // if (pcap_finda...
 
  return(lRetVal);
}





