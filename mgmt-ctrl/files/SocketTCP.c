#include "SocketTCP.h"
#include <net/if.h>
//#include <linux/if.h>
//#include <linux/types.h>

INT32 CreateTCPClient(/*INT8* srcIp,*/INT16 srcPort/*,struct sockaddr_in* serAddr,INT8* ppp*/)
{
	INT32 workSockfd;
	struct sockaddr_in cliAddr;
	//struct sockaddr_in serAddr;
	struct ifreq ifppp;
	INT8 buffer[1000];
	socklen_t len;
//	strncpy(ifppp.ifr_name,ppp,IFNAMSIZ);

	cliAddr.sin_family      = AF_INET;
//	cliAddr.sin_addr.s_addr = inet_addr(srcIp);
	cliAddr.sin_addr.s_addr = INADDR_ANY;
	cliAddr.sin_port        = htons(srcPort);

	//serAddr.sin_family      = AF_INET;
	//serAddr.sin_addr.s_addr = inet_addr(destIp);
	//serAddr.sin_port        = htons(destPort);

	workSockfd = socket (AF_INET , SOCK_STREAM , 0 );
	if(workSockfd <= 0)
	{
		return RETURN_FAILED;
	}

	//if(/*RETURN_FAILED*/-1 == bind(workSockfd,(struct sockaddr *)&cliAddr,sizeof(cliAddr)))
	//{
	//	CloseTCPSocket(workSockfd);
	//	return RETURN_FAILED;
	//}

	//if(SOCKET_ERROR == listen(workSockfd,1))
	//{
	//	return RETURN_FAILED;
	//}
//	if(setsockopt(workSockfd,SOL_SOCKET,SO_BINDTODEVICE,(INT8*)&ifppp,sizeof(ifppp))<0)
//	{
//		printf("CreateTCPClient error  ppp = %s\n",ppp);
//		CloseTCPSocket(workSockfd);
//		return RETURN_FAILED;
//	}

//	if(getsockopt(workSockfd,SOL_SOCKET,SO_BINDTODEVICE,(void*)&buffer,&len)<0)
//	{
//		printf("CreateTCPClient error2  ppp = %s\n",ppp);
//		CloseTCPSocket(workSockfd);
//		return RETURN_FAILED;
//	}

//	printf("buffer = %s\n",buffer);

	return workSockfd;
}

INT32 ConnectServer(INT32 workSockfd,struct sockaddr_in* serAddr)
{
	int i = 5;
		printf("4 %d\n",workSockfd);
	if(workSockfd <= 0)
		return RETURN_FAILED;
	while(/*RETURN_FAILED*/-1 == connect (workSockfd, serAddr,sizeof(struct sockaddr_in)))
	{
		i --;
		ThreadSleep(500);
		if(i == 0)
		{
					printf("3 %d\n",workSockfd);
			return RETURN_FAILED;
		}
		printf("1 %d\n",workSockfd);
	}
	printf("2 %d\n",workSockfd);
}

INT32 CreateTCPServer(INT16 port)
{
	INT32 workSockfd;
	struct sockaddr_in svrAddr;

	svrAddr.sin_family      = AF_INET;
	svrAddr.sin_addr.s_addr = INADDR_ANY;
	svrAddr.sin_port        = htons(port);

	workSockfd = socket (AF_INET , SOCK_STREAM , 0 );
	if(workSockfd <= 0)
	{
		return RETURN_FAILED;
	}

	//struct ifreq if_ppp0;
	//struct ifreq if_ppp1;
	//strncpy(if_ppp0.ifr_name, "ppp0", IFNAMSIZ);
	//strncpy(if_ppp1.ifr_name, "ppp1", IFNAMSIZ);

	//if (setsockopt(sock1, SOL_SOCKET, SO_BINDTODEVICE,
	//	(char *)&if_ppp0, sizeof(if_ppp0)) 
	//{
	//	/*error handling*/
	//}

	if(/*RETURN_FAILED*/-1 == bind(workSockfd,(struct sockaddr *)&svrAddr,sizeof(svrAddr)))
	{
		CloseTCPSocket(workSockfd);
		return RETURN_FAILED;
	}

	if(/*RETURN_FAILED*/-1 == listen(workSockfd,10))
	{
		CloseTCPSocket(workSockfd);
		return RETURN_FAILED;
	}
	return workSockfd;
}

INT32 SendTCPClient(INT32 workSockfd,INT8 *pSendBuf,INT32 sendDataLen)
{
	INT32 allsendlen = 0;
	INT32 thissendlen;

	while(allsendlen<sendDataLen)
	{
		thissendlen = send (workSockfd, pSendBuf+ allsendlen, sendDataLen-allsendlen, 0 );
		if(thissendlen<0)
		{
			return RETURN_FAILED;
		}
		allsendlen += thissendlen;
	}
	//printf("allsendlen = %d\n",allsendlen);
	return RETURN_OK;
}

INT32 RecvTCPClient(INT32 workSockfd,INT8 *pRecvBuf,INT32 recvBufLen)
{
	INT32 allrecvlen = 0;
	INT32 thisrecvlen;

	while (allrecvlen<recvBufLen)
	{
		thisrecvlen = recv (workSockfd , pRecvBuf+allrecvlen , recvBufLen-allrecvlen , 0 );
		if(thisrecvlen<0)
		{
			return RETURN_FAILED;
		}
		allrecvlen+=thisrecvlen;
	}
	return RETURN_OK;
}

INT32 SendTCPServer(INT32 workSockfd,INT8 *pSendBuf,INT32 sendDataLen)
{
	int allsendlen = 0;
	int thissendlen;

	while(allsendlen<sendDataLen)
	{
		thissendlen = send (workSockfd, pSendBuf+ allsendlen, sendDataLen-allsendlen, 0 );
		if(thissendlen<0)
		{
			return RETURN_FAILED;
		}
		allsendlen += thissendlen;
	}
	return RETURN_OK;
}

INT32 RecvTCPServer(INT32 workSockfd,INT8 *pRecvBuf,INT32 recvBufLen)
{
	INT32 allrecvlen = 0;
	INT32 thisrecvlen;

	while (allrecvlen<recvBufLen)
	{
		thisrecvlen = recv (workSockfd , pRecvBuf+allrecvlen , recvBufLen-allrecvlen , 0 );
		if(thisrecvlen<0)
		{
			return RETURN_FAILED;
		}
		allrecvlen+=thisrecvlen;
	}
	return RETURN_OK;
}

void CloseTCPSocket(INT32 workSockfd)
{
	if(workSockfd > 0)
	{
		shutdown(workSockfd,SHUT_RDWR);
		close(workSockfd);
	}
}

INT32 OnlineMonitor(INT32 listenSockd,struct sockaddr_in* clientAddr)
{
	INT32 newSockfd;
	socklen_t clientAddrSize = sizeof(struct sockaddr_in);
	newSockfd = accept(listenSockd,(struct sockaddr*)clientAddr,&clientAddrSize);
	if(newSockfd > 0)
	{
		return newSockfd;
	}
	else{
		return 0;
	}

}
