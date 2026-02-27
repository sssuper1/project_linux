#ifndef   _SOCKETTCP_H
#define   _SOCKETTCP_H


#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
//#include <error.h>
#include <arpa/inet.h>
#include "mgmt_types.h"

typedef struct STcpClient {
	INT32 sockfd;
	BOOL useful;
	INT32 srcip;
	struct timeval time;
} TcpClient;

INT32 CreateTCPClient(/*INT8* srcIp,*/INT16 srcPort/*,struct sockaddr_in* serAddr,INT8* ppp*/);
INT32 CreateTCPServer(INT16 port);
INT32 ConnectServer(INT32 workSockfd,struct sockaddr_in* serAddr);
INT32 SendTCPClient(INT32 workSockfd,INT8 *pSendBuf,INT32 sendDataLen);
INT32 RecvTCPClient(INT32 workSockfd,INT8 *pRecvBuf,INT32 recvBufLen);
INT32 SendTCPServer(INT32 workSockfd,INT8 *pSendBuf,INT32 sendDataLen);
INT32 RecvTCPServer(INT32 workSockfd,INT8 *pRecvBuf,INT32 recvBufLen);
void CloseTCPSocket(INT32 workSockfd);
INT32 OnlineMonitor(INT32 listenSockd,struct sockaddr_in* clientAddr);
int createUdpClient(struct sockaddr_in *addr,const char* ip,const int port);
#endif
