/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2025-06-18 14:56:06
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025-06-23 10:11:41
 * @FilePath: \files\socketUDP.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * socketUDP.h
 *
 *  Created on: Jan 26, 2021
 *      Author: slb
 */

#ifndef SOCKETUDP_H_
#define SOCKETUDP_H_

#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <math.h>
#include <arpa/inet.h>

int CreateUDPServer(int port);
int RecvUDPClient(int socket, char *buf, int bufsize, struct sockaddr_in *from,
		int *from_len);
int SendUDPClient(int socket, char *msg, int len, struct sockaddr_in * to);
int MakeCMD(char *buf1, char *buf2);
int CreateUDPServerToDevice(char* eth, int len, int port);
void CloseUDPSocket(int workSockfd);

int crateUdpClient(struct sockaddr_in *addr,const char* ip,const int port);
void add_multiaddr_group(int s,char* group_ip);

#endif /* SOCKETUDP_H_ */
