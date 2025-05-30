#define HAVE_REMOTE
#include <time.h>  
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <iphlpapi.h>
#include <Shlwapi.h>
#include <stdarg.h>
#include "TCPReassemble.h"
#include "PacketList.h"


#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")


CRITICAL_SECTION gCSConnectionsList; 
PCONNODE gConnectionList = NULL;

/*
 * Global variables
 *
 */



char gDBPath[MAX_BUF_SIZE + 1];






/*
 * Program entry point
 *
 */

int main(int argc, char *argv[])
{ 
  int lRetVal = 0;
  char lTemp[MAX_BUF_SIZE + 1];
  char lIFCName[MAX_BUF_SIZE + 1];
  int lCounter = 0;
  char *lTempPtr = NULL;
  SCANPARAMS lScanParams;









  /*
   * Initialisation
   *
   */

  ZeroMemory(lTemp, sizeof(lTemp));
  ZeroMemory(lIFCName, sizeof(lIFCName));
  ZeroMemory(&lScanParams, sizeof(lScanParams));


  /*
   * 
   *
   */
  if (argc == 2 && ! strcmp(argv[1], "-l")) 
  {
    ListIFCDetails();




  /*
   *
   *
   */
  } 
  else if (argc == 2) 
  {    
    InitializeCriticalSectionAndSpinCount(&gCSConnectionsList, 0x00000400);

    if ((gConnectionList = InitConnectionList()) != NULL)
	{
      // Interface num
      strncpy(lScanParams.IFCName, argv[1], sizeof(lScanParams.IFCName));

      // Retrieve IFC details
      GetIFCName(argv[1], lScanParams.IFCName, sizeof(lScanParams.IFCName) - 1);
      GetIFCDetails(argv[1], &lScanParams);

      // Start capturing packets
      StartSniffer(&lScanParams);
	} // if ((gConnection...


    DeleteCriticalSection(&gCSConnectionsList);
  } // if (argc == 2 &...


END:

  return(lRetVal);
}
 







/*
 *
 *
 */
void Stringify(unsigned char *pInput, int pInputLen, unsigned char *pOutput)
{
  int lCounter = 0;

  for (;lCounter < pInputLen; lCounter++)
  {
//    if (pInput[lCounter] < 32 || pInput[lCounter] > 176)
    if (pInput[lCounter] < 32 || pInput[lCounter] > 126)
      pOutput[lCounter] = '.';
	else
      pOutput[lCounter] = pInput[lCounter];
  }
}





/*
 *
 *
 */

int ListIFCDetails()
{
  int lRetVal = 0;
  PIP_ADAPTER_INFO lAdapterInfoPtr = NULL;
  PIP_ADAPTER_INFO lAdapter = NULL;
  DWORD lFuncRetVal = 0;
  UINT lCounter;
  struct tm lTimeStamp;
  char lTemp[MAX_BUF_SIZE +1 ];
  errno_t error;
  ULONG lOutBufLen = sizeof (IP_ADAPTER_INFO);



  if ((lAdapterInfoPtr = (IP_ADAPTER_INFO *) HeapAlloc(GetProcessHeap(), 0, sizeof (IP_ADAPTER_INFO))) == NULL)
  {
    lRetVal = 1;
    goto END;
  } // if ((lAdapter...


  if (GetAdaptersInfo(lAdapterInfoPtr, &lOutBufLen) == ERROR_BUFFER_OVERFLOW) 
  {
    HeapFree(GetProcessHeap(), 0, lAdapterInfoPtr);
    if ((lAdapterInfoPtr = (IP_ADAPTER_INFO *) HeapAlloc(GetProcessHeap(), 0, lOutBufLen)) == NULL)
    {
      lRetVal = 2;

      goto END;
    } // if ((lAdapte...
  } // if (GetA...



  /*
   *
   *
   */
  if ((lFuncRetVal = GetAdaptersInfo(lAdapterInfoPtr, &lOutBufLen)) == NO_ERROR) 
  {
    for (lAdapter = lAdapterInfoPtr; lAdapter; lAdapter = lAdapter->Next)
    {
      printf("\n\nIfc no : %d\n", lAdapter->ComboIndex);
      printf("\tAdapter Name: \t%s\n", lAdapter->AdapterName);
      printf("\tAdapter Desc: \t%s\n", lAdapter->Description);
      printf("\tAdapter Addr: \t");

      for (lCounter = 0; lCounter < lAdapter->AddressLength; lCounter++) 
      {
        if (lCounter == (lAdapter->AddressLength - 1))
          printf("%.2X\n", (int) lAdapter->Address[lCounter]);
        else
          printf("%.2X-", (int) lAdapter->Address[lCounter]);
      }

      printf("\tIndex: \t%d\n", lAdapter->Index);
      printf("\tType: \t");

      switch (lAdapter->Type) 
      {
         case MIB_IF_TYPE_OTHER:
              printf("Other\n");
              break;
         case MIB_IF_TYPE_ETHERNET:
              printf("Ethernet\n");
              break;
         case MIB_IF_TYPE_TOKENRING:
              printf("Token Ring\n");
              break;
         case MIB_IF_TYPE_FDDI:
              printf("FDDI\n");
              break;
         case MIB_IF_TYPE_PPP:
              printf("PPP\n");
              break;
         case MIB_IF_TYPE_LOOPBACK:
              printf("Lookback\n");
              break;
         case MIB_IF_TYPE_SLIP:
              printf("Slip\n");
              break;
         default:
              printf("Unknown type %ld\n", lAdapter->Type);
              break;
      }

      printf("\tIP Address: \t%s\n", lAdapter->IpAddressList.IpAddress.String);
      printf("\tIP Mask: \t%s\n", lAdapter->IpAddressList.IpMask.String);
      printf("\tGateway: \t%s\n", lAdapter->GatewayList.IpAddress.String);

      if (lAdapter->DhcpEnabled) 
      {
        printf("\tDHCP Enabled: Yes\n");
        printf("\t  DHCP Server: \t%s\n", lAdapter->DhcpServer.IpAddress.String);
        printf("\t  Lease Obtained: ");

        if (error = _localtime32_s(&lTimeStamp, (__time32_t*) &lAdapter->LeaseObtained))
          printf("Invalid Argument to _localtime32_s\n");
        else {
          if (error = asctime_s(lTemp, sizeof(lTemp), &lTimeStamp))
            printf("Invalid Argument to asctime_s\n");
          else
            printf("%s", lTemp);
        }

        printf("\t  Lease Expires:  ");

        if (error = _localtime32_s(&lTimeStamp, (__time32_t*) &lAdapter->LeaseExpires))
          printf("Invalid Argument to _localtime32_s\n");
        else {
          // Convert to an ASCII representation 
          if (error = asctime_s(lTemp, sizeof(lTemp), &lTimeStamp))
            printf("Invalid Argument to asctime_s\n");
          else
            printf("%s", lTemp);
        }
      } else
        printf("\tDHCP Enabled: No\n");

      if (lAdapter->HaveWins) 
      {
        printf("\tHave Wins: Yes\n");
        printf("\t  Primary Wins Server:    %s\n", lAdapter->PrimaryWinsServer.IpAddress.String);
        printf("\t  Secondary Wins Server:  %s\n", lAdapter->SecondaryWinsServer.IpAddress.String);
      } else
         printf("\tHave Wins: No\n");
    }
  }

END:
  if (lAdapterInfoPtr)
    HeapFree(GetProcessHeap(), 0, lAdapterInfoPtr);

  return(lRetVal);
}






/*
 *
 *
 */
int GetIFCName(char *pIFCName, char *pRealIFCName, int pBufLen)
{
  int lRetVal = 0;
  pcap_if_t *lAllDevs = NULL;
  pcap_if_t *lDevice = NULL;
  char lTemp[PCAP_ERRBUF_SIZE];
  char lAdapter[MAX_BUF_SIZE + 1];
  int lCounter = 0;
  int lIFCnum = 0;
 

  /*
   * Open device list.
   *
   */
 
  if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &lAllDevs, lTemp) == -1)
  {
    lRetVal = 1;
    goto END;
  } // if (pcap_fi...

 
  ZeroMemory(lAdapter, sizeof(lAdapter));
  lCounter = 0;
 
  for(lCounter = 0, lDevice = lAllDevs; lDevice; lDevice = lDevice->next, lCounter++)
  {
    if (StrStrI(lDevice->name, pIFCName))
    {
      strncpy(pRealIFCName, lDevice->name, pBufLen);
      break;
    } // if (StrStr..
  } // for(lCount...
 
 
END:
 
  /*
   * Release all allocated resources.
   *
   */
 
  if (lAllDevs)
    pcap_freealldevs(lAllDevs);
 
  return(lRetVal);
}





/*
 *
 *
 */
int GetIFCDetails(char *pIFCName, PSCANPARAMS pScanParams)
{
  int lRetVal = 0;
  unsigned long lLocalIPAddr = 0;
  unsigned long lGWIPAddr = 0;
  ULONG lGWMACAddr[2];
  ULONG lGWMACAddrLen = 6;
  PIP_ADAPTER_INFO lAdapterInfoPtr = NULL;
  PIP_ADAPTER_INFO lAdapter = NULL;
  DWORD lFuncRetVal = 0;
  ULONG lOutBufLen = sizeof (IP_ADAPTER_INFO);


  if ((lAdapterInfoPtr = (IP_ADAPTER_INFO *) HeapAlloc(GetProcessHeap(), 0, sizeof (IP_ADAPTER_INFO))) == NULL)
  {
    lRetVal = 1;
    goto END;
  } // if ((lAdapterInfo...


  if (GetAdaptersInfo(lAdapterInfoPtr, &lOutBufLen) == ERROR_BUFFER_OVERFLOW) 
  {
    HeapFree(GetProcessHeap(), 0, lAdapterInfoPtr);
    if ((lAdapterInfoPtr = (IP_ADAPTER_INFO *) HeapAlloc(GetProcessHeap(), 0, lOutBufLen)) == NULL)
    {
      lRetVal = 2;
      goto END;
    } // if ((lAdap...
  } // if (GetAdapte...




  /*
   *
   *
   */
  if ((lFuncRetVal = GetAdaptersInfo(lAdapterInfoPtr, &lOutBufLen)) == NO_ERROR) 
  {
    for (lAdapter = lAdapterInfoPtr; lAdapter; lAdapter = lAdapter->Next)
    {
      if (StrStrI(lAdapter->AdapterName, pIFCName))
	  {
        // Get local MAC address
        CopyMemory(pScanParams->LocalMAC, lAdapter->Address, BIN_MAC_LEN);


        // Get local IP address
        lLocalIPAddr = inet_addr(lAdapter->IpAddressList.IpAddress.String);
        CopyMemory(pScanParams->LocalIP, &lLocalIPAddr, 4);


		// Get gateway IP address
        lGWIPAddr = inet_addr(lAdapter->GatewayList.IpAddress.String);
        CopyMemory(pScanParams->GWIP, &lGWIPAddr, 4);


        // Get gateway MAC address
        CopyMemory(pScanParams->GWIP, &lGWIPAddr, 4);
        ZeroMemory(&lGWMACAddr, sizeof(lGWMACAddr));
        SendARP(lGWIPAddr, 0, lGWMACAddr, &lGWMACAddrLen);
        CopyMemory(pScanParams->GWMAC, lGWMACAddr, 6);

        break;
	  } // if (StrSt...
    } // for (lAdapt...
  } // if ((lFunc...

END:
  if (lAdapterInfoPtr)
    HeapFree(GetProcessHeap(), 0, lAdapterInfoPtr);


  return(lRetVal);
}
