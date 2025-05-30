#ifndef __REASSEMBLER__
#define __REASSEMBLER__

#include <windows.h>

#define snprintf _snprintf
#define MAX_BUF_SIZE 1024
#define MAX_ID_LEN 128
#define MAX_PAYLOAD 1460
#define BIN_MAC_LEN 6
#define BIN_IP_LEN 4
#define TCP_MAX_INACTIVITY 30
#define MAX_CONNECTION_VOLUME 4096
#define MAX_CONNECTION_COUNT 1024





/*
 * Type definitions
 *
 */

typedef struct ethern_hdr
{
  unsigned char ether_dhost[6];  // dest Ethernet address
  unsigned char ether_shost[6];  // source Ethernet address
  unsigned short ether_type;     // protocol (16-bit)
} ETHDR, *PETHDR;


typedef struct ipaddress
{
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
} IPADDRESS, *PIPADDRESS;

typedef struct iphdr
{
  unsigned char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
  unsigned char  tos;            // Type of service 
  unsigned short tlen;           // Total length 
  unsigned short identification; // Identification
  unsigned short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
  unsigned char  ttl;            // Time to live
  unsigned char  proto;          // Protocol
  unsigned short crc;            // Header checksum
  IPADDRESS      saddr;      // Source address
  IPADDRESS      daddr;      // Destination address
  unsigned int   opt;        // Option + padding
} IPHDR, *PIPHDR;



typedef struct tcphdr 
{
  unsigned short sport;  
  unsigned short dport;
  unsigned int   seq; 
  unsigned int   ack_seq; 
  unsigned short res1:4, 
                 doff:4,
                 fin:1,
                 syn:1,  
                 rst:1,  
                 psh:1,  
                 ack:1,  
                 urg:1, 
                 res2:2; 
  unsigned short window;
  unsigned short check;  
  unsigned short urg_ptr;
} TCPHDR, *PTCPHDR;



typedef struct SCANPARAMS
{
  unsigned char IFCName[MAX_BUF_SIZE + 1];
  unsigned char GWIP[BIN_IP_LEN];
  unsigned char GWMAC[BIN_MAC_LEN];
  unsigned char StartIP[BIN_IP_LEN];
  unsigned char StopIP[BIN_IP_LEN];
  unsigned char LocalIP[BIN_IP_LEN];
  unsigned char LocalMAC[BIN_MAC_LEN];
  unsigned char *PCAPPattern;
  HANDLE PipeHandle;
} SCANPARAMS, *PSCANPARAMS;



/*
 * Function forward declaration
 *
 */
void POISONSnifferCallback(unsigned char *pScanParams, struct pcap_pkthdr *header, unsigned char *pkt_data);
int StartSniffer(PSCANPARAMS pScanParams);
void POISONSnifferCallback(unsigned char *pScanParams, struct pcap_pkthdr *header, unsigned char *pkt_data);
int GetIFCDetails(char *pIFCName, PSCANPARAMS pScanParams);
int GetIFCName(char *pIFCName, char *pRealIFCName, int pBufLen);

#endif