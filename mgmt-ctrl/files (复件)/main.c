/*
 * main.c
 *
 *  Created on: Jul 23, 2020
 *      Author: slb
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include "mgmt_types.h"
#include "mgmt_netlink.h"
#include "mgmt_transmit.h"
#include "Thread.h"
#include "sqlite_unit.h"
#include"gpsget.h"

#pragma pack(1)

#define Radio_QK 

void SetupSignal()
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigemptyset(&sa.sa_mask) == -1 ||
			sigaction(SIGPIPE,&sa,0) == -1)
	{
		exit(-1);
	}
}



int main(){

    printf("tianhui:hello world\n");

	SetupSignal();
	mgmt_mysql_init();
	sqliteinit();
	Create_Thread(mgmt_get_msg,NULL);//状态上报
//	Create_Thread(mgmt_recv_web,NULL);
	Create_Thread(mgmt_recv_msg,NULL);
	Create_Thread(sqlite_set_param,NULL);//参数设置
	//Create_Thread(gps_Thread,NULL);//gps数据获取

#ifdef	 Radio_QK
	Create_Thread(thread_report_test,NULL);
	Create_Thread(mgmt_recv_from_qkwg,NULL);	 //业务模拟系统
	Create_Thread(mgmt_recv_from_qkcj,NULL);     //场景系统
	
#endif
//#ifdef Radio_CEC
//    Create_Thread(Beam_infor_update_cyclic,NULL);
//#endif
   
	while(1){
		sleep(10);
	
		}

	return 0;
}
