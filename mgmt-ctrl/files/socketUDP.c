/*
 * socketUDP.c
 *
 *  Created on: Jan 26, 2021
 *      Author: slb
 */

#include "mgmt_types.h"
#include "socketUDP.h"
#include "errno.h"
#include <string.h>
int CreateUDPServer(int port)
{
	int server_sockfd;
	struct sockaddr_in my_addr;   //服务器网络地址结构体
	memset(&my_addr, 0, sizeof(my_addr)); //数据初始化--清零
	my_addr.sin_family = AF_INET; //设置为IP通信
	my_addr.sin_addr.s_addr = INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上
	my_addr.sin_port = htons(port); //服务器端口号
	if ((server_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}

	int reuse = 1;
    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
    }	
	/*将套接字绑定到服务器的网络地址上*/
	if (bind(server_sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind");
		return -1;
	}

	return server_sockfd;
}

int CreateUDPServerToDevice(char* eth, int len, int port)
{
	int workSockfd;
	struct sockaddr_in svrAddr;
	int flag;

	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = INADDR_ANY;
	svrAddr.sin_port = htons(port);

	workSockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (workSockfd <= 0)
	{
		return RETURN_FAILED;
	}

	if (/*RETURN_FAILED*/-1 == bind(workSockfd, (struct sockaddr*)&svrAddr, sizeof(svrAddr)))
	{
		CloseUDPSocket(workSockfd);
		return RETURN_FAILED;
	}
	flag = 1;

	if (setsockopt(workSockfd, SOL_SOCKET, SO_BINDTODEVICE, eth, len) == -1)
	{
		//		printf("setsockopt SO_BINDTODEVICE error! 2\n");
		CloseUDPSocket(workSockfd);
		return RETURN_FAILED;
	}

	return workSockfd;
}

int RecvUDPClient(int socket, char* buf, int bufsize, struct sockaddr_in* from, int* from_len)
{
	int len;
	//	printf("waiting for a packet...\n");
	if ((len = recvfrom(socket, buf, bufsize, 0, from, from_len)) < 0)
	{
		//printf("receive error");
		return -1;
	}
	//	printf("received packet from %d %d\n",from->sin_addr,from->sin_addr.s_addr);
	buf[len] = '\0';
	//	printf("contents: %s\n",buf);
	return len;
}

int SendUDPClient(int socket, char* msg, int len, struct sockaddr_in* to)
{
	int s_len;
	if ((s_len = sendto(socket, msg, len, 0, to, sizeof(struct sockaddr))) != len)
	{
		fprintf(stderr, "send failed: %s,ErrNo:%d\n", strerror(errno),errno);
		return -1;
	}
		
	return s_len;
}


//add by yang
int SendUDPBrocast(int socket, char* msg, int len, struct sockaddr_in* to)
{
	int ret;
	int broadcast = 1;

	// Send the message
	ret = sendto(socket, msg, len, 0, to, sizeof(struct sockaddr_in));
	if (ret < 0) {
		perror("sendto() failed");
		return -1;
	}

	return 0;
}


void CloseUDPSocket(int workSockfd)
{
	if (workSockfd > 0)
	{
		shutdown(workSockfd, SHUT_RDWR);
		close(workSockfd);
	}
}

//add by sdg 
int createUdpClient(struct sockaddr_in *addr,const char* ip,const int port)
{
	int s=socket(AF_INET, SOCK_DGRAM, 0);

	addr->sin_family = AF_INET;
	addr->sin_port =htons(port);

	if(inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
	{
		perror("invalid address \r\n");	
		return -1;	
	}

	return s;
}

/* 加入到组播 */
void add_multiaddr_group(int s,char* group_ip)
{
	struct ip_mreq ldyw_group;
	ldyw_group.imr_multiaddr.s_addr = inet_addr(group_ip);
	ldyw_group.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ldyw_group, sizeof(ldyw_group)) < 0) {
        printf("[CJ DEBUG] :fail to Join multicast group \n");
        //join_success = 1;
    } 

}
