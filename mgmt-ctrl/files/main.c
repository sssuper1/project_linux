/*
 * @Author: SunDG311 sdg18252543282@163.com
 * @Date: 2025-06-23 18:21:23
 * @LastEditors: SunDG311 sdg18252543282@163.com
 * @LastEditTime: 2025-11-13 09:35:04
 * @FilePath: \files\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
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
#include "ui_get.h"
#include "audio_uart.h"
#pragma pack(1)


int ui_fd;

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
	printf("compile time 2025-11-15-11:50\r\n");
	//printf("no ui uart version\r\n ");
	// int fd;
	SetupSignal();
	mgmt_mysql_init();
	ui_fd=uart_init();
	if(ui_fd==-1)
	{
		printf("init ui uart error\r\n");
	}
	sqliteinit();
	Create_Thread(mgmt_get_msg,NULL);//状态上报
	Create_Thread(mgmt_recv_web,NULL);
	Create_Thread(mgmt_recv_msg,NULL);
	Create_Thread(sqlite_set_param,NULL);//参数设置
	Create_Thread(gps_Thread,NULL);//gps数据获取

#ifdef	 Radio_QK
	Create_Thread(thread_report_test,NULL);
	Create_Thread(mgmt_recv_from_qkwg,NULL);	 //业务模拟系统
	Create_Thread(mgmt_recv_from_qkcj,NULL);     //场景系统
	Create_Thread(get_ui_Thread,(void*)ui_fd);
	Create_Thread(write_ui_Thread,(void*)ui_fd);
	Create_Thread(audio_thread,NULL);
	Create_Thread(play_audio_thread,NULL);

	
#endif
//#ifdef Radio_CEC
//    Create_Thread(Beam_infor_update_cyclic,NULL);
//#endif
   
	while(1){
		sleep(10);
	
		}

	return 0;
}
