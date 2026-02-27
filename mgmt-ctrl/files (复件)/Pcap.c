#include "Pcap.h"
#include <string.h>


pcap_t* GetPcapDevice(char* adaptername,INT8* packet_filter)
{
	struct bpf_program fcode;
	UINT32 netmask;
	pcap_if_t * allAdapters;
	pcap_if_t * adapter;
	pcap_t           * adapterHandle;
	INT32 crtAdapter = 0;
	char errorBuffer[ PCAP_ERRBUF_SIZE ];
//	printf( "\nGetPcapDevice start!!!!!!!!!!!!!!!\n" );
//	if( pcap_findalldevs( PCAP_SRC_IF_STRING, NULL,
//		&allAdapters, errorBuffer ) == -1 )
//	{
//		fprintf( stderr, "Error in pcap_findalldevs_ex function: %s\n", errorBuffer );
//		return NULL;
//	}
//	if( allAdapters == NULL )
//	{
//		printf( "\nNo adapters found! Make sure WinPcap is installed.\n" );
//		return NULL;
//	}
//
//	for(adapter = allAdapters;adapter != NULL;adapter = adapter->next)
//	{
//		//++crtAdapter;
//		printf( "\n%d.%s ", ++crtAdapter, adapter->name );
//		printf( "-- %s\n", adapter->description );
//	}
//
//	if(adapterNumber < 1 || adapterNumber > crtAdapter)
//	{
//		return NULL;
//	}

//	adapter = allAdapters;
//	for( crtAdapter = 0; crtAdapter < adapterNumber - 1; crtAdapter++ )
//		adapter = adapter->next;

	adapterHandle = pcap_open_live( adaptername, // name of the adapter
		1520,         // portion of the packet to capture
		// 65536 guarantees that the whole 
		// packet will be captured
		/*PCAP_OPENFLAG_PROMISCUOUS*/0, // promiscuous mode
		-1,             // read timeout - 1 millisecond
//		NULL,          // authentication on the remote machine
		errorBuffer    // error buffer
		);
	if( adapterHandle == NULL )
	{
		fprintf( stderr, "\nUnable to open the adapter\n", adapter->name );

//		pcap_freealldevs( allAdapters );
		return NULL;
	}

//	if(adapter->addresses != NULL)
//	{
//		netmask=((struct sockaddr_in *)(adapter->addresses->netmask))->sin_addr.S_un.S_addr;
//	}
//	else
//		netmask=0xffffff;



	/////
	if (pcap_compile(adapterHandle, &fcode, packet_filter, 1, netmask) <0 )
	{
		fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");

//		pcap_freealldevs(allAdapters);
		return NULL;
	}


	if (pcap_setfilter(adapterHandle, &fcode)<0)
	{
		fprintf(stderr,"\nError setting the filter.\n");

//		pcap_freealldevs(allAdapters);
		return NULL;
	}

//	printf( "\nCapture session started on  adapter %s\n", adapterHandle-> );
//	pcap_freealldevs( allAdapters );

	return adapterHandle;
}


INT32 GetIPPacket(pcap_t *adapterHandle,INT8* data)
{

	return pcap_dispatch(adapterHandle,0,NULL,NULL);
	//printf("1\n");
	//struct pcap_pkthdr* packetHeader;
	//int retValue = -1;
	//INT8* buffer;
	//int i = 0;

	//if(adapterHandle == NULL)
	//{
	//	return RETURN_FAILED;
	//}

	//retValue = pcap_next_ex(adapterHandle,&packetHeader,&buffer);

	////printf( "\npacketHeader->len = %d\n" ,packetHeader->len);

	//if(retValue <= 0)
	//{
	//	return RETURN_FAILED;
	//}
	//else
	//{
	//	//system("PAUSE");
	//	memcpy(data,buffer,packetHeader->len);
	//	return packetHeader->len;
	//}
}


INT32 GetPacketIP(INT8* packet,INT32 packetLen,INT8* ipaddr,INT8* data,INT32 dataLen)
{
	ip_header* ih;
	if(packetLen< (sizeof(ip_header)))
	{
		return RETURN_FAILED;
	}
	ih = (ip_header*)packet;
//	sprintf(ipaddr,"%d.%d.%d.%d",ih->daddr.byte1,ih->daddr.byte2,ih->daddr.byte3,ih->daddr.byte4);
	//printf("ip = %s\n",ipaddr);
	dataLen = packetLen;
	memcpy(data,packet,dataLen);
	return dataLen;
}

INT32 GetPacketDstIP(INT8* packet,INT32 packetLen,INT8* dstip)
{
	ip_header *ih;
	if(packetLen<sizeof(ip_header))
	{
		return RETURN_FAILED;
	}
	ih = (ip_header*)packet;
//	sprintf(dstip,"%d.%d.%d.%d",ih->daddr.byte1,ih->daddr.byte2,ih->daddr.byte3,ih->daddr.byte4);
	return RETURN_OK;
}
