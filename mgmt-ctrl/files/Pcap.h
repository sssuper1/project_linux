#ifndef   _PCAP_H
#define   _PCAP_H

#include "mgmt_types.h"
#include <pcap.h>

/* 4 bytes IP address */
typedef struct ip_address
{
	UINT8 byte1;
	UINT8 byte2;
	UINT8 byte3;
	UINT8 byte4;
}ip_address;


///* IPv4 header */
//typedef struct ip_header
//{
//	UINT8 ver_ihl;   /* Version (4 bits) + Internet header length (4 bits)*/
//	UINT8 tos;       /* Type of service */
//	UINT16 tlen;     /* Total length */
//	UINT16 identification; /* Identification */
//	UINT16 flags_fo;        /* Flags (3 bits) + Fragment offset (13 bits)*/
//	UINT8 ttl;       /* Time to live */
//	UINT8 proto;     /* Protocol */
//	UINT16 crc;      /* Header checksum */
//	ip_address saddr;/* Source address */
//	ip_address daddr;/* Destination address */
//	UINT8 op_pad;     /* Option + Padding */
//}ip_header;


pcap_t* GetPcapDevice(char* adaptername,INT8* packet_filter);
INT32 GetIPPacket(pcap_t *adapterHandle,INT8* data);
INT32 GetPacketIP(INT8* packet,INT32 packetLen,INT8* ipaddr,INT8* data,INT32 dataLen);
INT32 GetPacketDstIP(INT8* packet,INT32 packetLen,INT8* dstip);
;

#endif
