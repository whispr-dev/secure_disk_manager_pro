/* is ip address in the same LAN segment */

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
int IPAddrBelongsToLocalNet(unsigned long pLocalIP, unsigned long pNetmask, unsigned long pTestAddr);
 
 
/*
 * Program entry point
 *
 */
int main(int argc, char *argv[])
{
  NETCONFIG lNetCfg;
  unsigned long lTestIP = 0;
 
  if (argc > 2)
  {
    ZeroMemory(&lNetCfg, sizeof(lNetCfg));
    if (searchInterface(argv[1], &lNetCfg) == 0)
    {
      lTestIP = inet_addr(argv[2]);
 
      if (IPAddrBelongsToLocalNet(lNetCfg.LocalIPAddr, lNetCfg.SubnetAddr, lTestIP) == 0)
        printf("System with IP %s is in the subnet\n", argv[2]);
      else
        printf("System with IP %s is not in the subnet\n", argv[2]);
    }
    else
      printf("Usage: %s ifc-name ip-address\n", argv[0]);
  }
 
  return(0);
}
 
 
 
 
 
/*
 *
 *
 */
int IPAddrBelongsToLocalNet(unsigned long pLocalIP, unsigned long pNetmask, unsigned long pTestAddr)
{
	unsigned long lLocalNet;
	unsigned long lOtherNet;
 
	lLocalNet = pLocalIP & pNetmask;
	lOtherNet = pTestAddr & pNetmask;
 
	return(lLocalNet==lOtherNet ? 0 : 1);
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