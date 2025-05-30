/*determine broadcast address */

#define WIN32_LEAN_AND_MEAN
 
#include <winsock2.h>
#include <stdio.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <Shlwapi.h>
 
#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Shlwapi")
 
 
 
/*
 * Data types
 *
 */
typedef struct sNetConfig
{
  unsigned long LocalIPAddr;
  unsigned long SubnetAddr;
  unsigned long BroadcastAddr;
  unsigned long NetworkAddr;
} NETCONFIG, *PNETCONFIG;
 
 
 
/*
 * Function forward declaration
 *
 */
int searchInterface(char *pIFCName, PNETCONFIG pNetCfg);
 
 
 
/*
 * Program entry point
 *
 */
int main(int argc, char *argv[])
{
  NETCONFIG lNetCfg;
  unsigned char *lIPAddressPtr;
  unsigned char *lBroadcastAddressPtr;
  unsigned char *lNetworkAddressPtr;
  unsigned char *lSubnetAddressPtr;
 
 
  if (argc > 1)
  {
    ZeroMemory(&lNetCfg, sizeof(lNetCfg));
    if (searchInterface(argv[1], &lNetCfg) == 0)
    {
      lIPAddressPtr = (unsigned char *) &lNetCfg.LocalIPAddr;
      lNetworkAddressPtr = (unsigned char *) &lNetCfg.NetworkAddr;
      lBroadcastAddressPtr = (unsigned char *) &lNetCfg.BroadcastAddr;
      lSubnetAddressPtr = (unsigned char *) &lNetCfg.SubnetAddr;
 
      printf("IFC name: %s\n", argv[1]);
      printf("IP addr: %lu/%d.%d.%d.%d\n", lNetCfg.LocalIPAddr, lIPAddressPtr[0], lIPAddressPtr[1], lIPAddressPtr[2], lIPAddressPtr[3]);
      printf("Subnet addr: %lu/%d.%d.%d.%d\n", lNetCfg.SubnetAddr, lSubnetAddressPtr[0], lSubnetAddressPtr[1], lSubnetAddressPtr[2], lSubnetAddressPtr[3]);
      printf("Broadcast addr: %lu/%d.%d.%d.%d\n", lNetCfg.BroadcastAddr, lBroadcastAddressPtr[0], lBroadcastAddressPtr[1], lBroadcastAddressPtr[2], lBroadcastAddressPtr[3]);
      printf("Network addr: %lu/%d.%d.%d.%d\n", lNetCfg.NetworkAddr, lNetworkAddressPtr[0], lNetworkAddressPtr[1], lNetworkAddressPtr[2], lNetworkAddressPtr[3]);
    }
  }
 
  return(0);
}
 
 
 
 
/*
 *
 *
 */
int searchInterface(char *pIFCName, PNETCONFIG pNetCfg)
{
  int lRetVal = 0;
  PIP_ADAPTER_INFO lAdapterInfoPtr = NULL;
  PIP_ADAPTER_INFO lAdapter = NULL;
  ULONG lOutBufLen = sizeof (IP_ADAPTER_INFO);
  unsigned char *lIPAddressPtr;
  unsigned char *lBroadcastAddressPtr;
  unsigned char *lNetworkAddressPtr;
  unsigned char *lSubnetAddressPtr;
  int i;
 
 
  if ((lAdapterInfoPtr = (IP_ADAPTER_INFO *) HeapAlloc(GetProcessHeap(), 0, sizeof (IP_ADAPTER_INFO))) == NULL)
  {
    printf("GetIFCDetails() : Error allocating memory needed to call GetAdaptersinfo\n");
    lRetVal = 1;
    goto END;
  } // if ((lAdapterInfo...
 
 
  if (GetAdaptersInfo(lAdapterInfoPtr, &lOutBufLen) == ERROR_BUFFER_OVERFLOW) 
  {
    HeapFree(GetProcessHeap(), 0, lAdapterInfoPtr);
    if ((lAdapterInfoPtr = (IP_ADAPTER_INFO *) HeapAlloc(GetProcessHeap(), 0, lOutBufLen)) == NULL)
    {
      printf("GetIFCDetails() : Error allocating memory needed to call GetAdaptersinfo\n");
      lRetVal = 2;
      goto END;
    } // if ((lAdap...
  } // if (GetAdapte...
 
 
 
  /*
   *
   */
  if (GetAdaptersInfo(lAdapterInfoPtr, &lOutBufLen) == NO_ERROR) 
  {
    for (lAdapter = lAdapterInfoPtr; lAdapter; lAdapter = lAdapter->Next)
    {
      if (StrStrI(pIFCName, lAdapter->AdapterName))
	     {
        if (&lAdapter->IpAddressList != NULL && &lAdapter->IpAddressList.IpAddress != NULL && lAdapter->IpAddressList.IpAddress.String != NULL)
        {
          pNetCfg->LocalIPAddr = inet_addr(lAdapter->IpAddressList.IpAddress.String);
          pNetCfg->SubnetAddr = inet_addr(lAdapter->IpAddressList.IpMask.String);
 
 
          lIPAddressPtr = (unsigned char *) &pNetCfg->LocalIPAddr;
          lNetworkAddressPtr = (unsigned char *) &pNetCfg->NetworkAddr;
          lBroadcastAddressPtr = (unsigned char *) &pNetCfg->BroadcastAddr;
          lSubnetAddressPtr = (unsigned char *) &pNetCfg->SubnetAddr;
 
          for (i = 0; i < 4; i++)
          {
            // Calculatin broadcast address
            lBroadcastAddressPtr[i] = (lIPAddressPtr[i] | (lSubnetAddressPtr[i] ^ 255));
 
            // Calculating network address
            lNetworkAddressPtr[i] = (lIPAddressPtr[i] & (lSubnetAddressPtr[i]));
          }
 
          lRetVal = 0;
          break;
        } // if (&lAdapter...
      } // if (StrStrI(pIFCN...
    } // for (lAdapt...
 
 
  }
  else
  {
    lRetVal = 1;
  } // if ((lFunc...
 
END:
  if (lAdapterInfoPtr)
    HeapFree(GetProcessHeap(), 0, lAdapterInfoPtr);
 
 
  return(lRetVal);
}