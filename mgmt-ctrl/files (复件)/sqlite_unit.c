/*
 * sqlite_unit.c
 *
 *  Created on: Apr 2, 2024
 *      Author: slb
 */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sqlite3.h"
#include "mgmt_types.h"
#include "sqlite_unit.h"
#include "mgmt_netlink.h"
#include "Lock.h"
#include "socketUDP.h"
#include "mgmt_transmit.h"
#include "wg_config.h"
sqlite3 *g_psqlitedb;
stSurveyInfo g_stSurveyInfo;
bcMeshInfo meshinfo;
pthread_mutex_t sqlite3_mutex1;

int SOCKET_BCAST_SEND; 
uint8_t  rate_auto=0;   //0:fix  1:auto
struct sockaddr_in S_GROUND_BCAST;
#define BCAST_SEND_PORT  7901   //组播端口

stInData g_stSystemInfo[SYSTEMINFO_PARAM_NUM] = {
		{"ipaddr","192.168.2.1","0","0"},
		{"runtimeid","2023.03.21","0","0"},
		{"device","1001","0","0"},
		{"g_ver","1001","0","0"},
		{"rf_freq","1001","0","0"},
		{"m_chanbw","1001","0","0"},
		{"m_txpower","1001","0","0"},
		{"m_rate","1001","0","0"},
		{"m_type","1001","0","0"},
		{"deviceid","1001","0","0"},
		{"signallevel","1001","0","0"},
		{"noise","1001","0","0"},
		{"deviceid2","1001","0","0"},
		{"signallevel2","1001","0","0"},
		{"noise2","1001","0","0"},
		{"deviceid3","1001","0","0"},
		{"signallevel3","1001","0","0"},
		{"noise3","1001","0","0"},
		{"deviceid4","1001","0","0"},
		{"signallevel4","1001","0","0"},
		{"noise4","1001","0","0"},
		{"deviceid5","1001","0","0"},
		{"signallevel5","1001","0","0"},
		{"noise5","1001","0","0"},
		{"deviceid6","1001","0","0"},
		{"signallevel6","1001","0","0"},
		{"noise6","1001","0","0"},
		{"deviceid7","1001","0","0"},
		{"signallevel7","1001","0","0"},
		{"noise7","1001","0","0"},
		{"deviceid8","1001","0","0"},
		{"signallevel8","1001","0","0"},
		{"noise8","1001","0","0"},
		{"deviceid9","1001","0","0"},
		{"signallevel9","1001","0","0"},
		{"noise9","1001","0","0"},
		{"deviceid10","1001","0","0"},
		{"signallevel10","1001","0","0"},
		{"noise10","1001","0","0"},
		{"deviceid11","1001","0","0"},
		{"signallevel11","1001","0","0"},
		{"noise11","1001","0","0"},
		{"deviceid12","1001","0","0"},
		{"signallevel12","1001","0","0"},
		{"noise12","1001","0","0"},
		{"deviceid13","1001","0","0"},
		{"signallevel13","1001","0","0"},
		{"noise13","1001","0","0"},
		{"deviceid14","1001","0","0"},
		{"signallevel14","1001","0","0"},
		{"noise14","1001","0","0"},
		{"deviceid15","1001","0","0"},
		{"signallevel15","1001","0","0"},
		{"noise15","1001","0","0"},
		{"deviceid16","1001","0","0"},
		{"signallevel16","1001","0","0"},
		{"noise16","1001","0","0"},
		{"deviceid17","1001","0","0"},
		{"signallevel17","1001","0","0"},
		{"noise17","1001","0","0"},
		{"deviceid18","1001","0","0"},
		{"signallevel18","1001","0","0"},
		{"noise18","1001","0","0"},
		{"deviceid19","1001","0","0"},
		{"signallevel19","1001","0","0"},
		{"noise19","1001","0","0"},
		{"deviceid20","1001","0","0"},
		{"signallevel20","1001","0","0"},
		{"noise20","1001","0","0"},
		{"deviceid21","1001","0","0"},
		{"signallevel21","1001","0","0"},
		{"noise21","1001","0","0"},
		{"deviceid22","1001","0","0"},
		{"signallevel22","1001","0","0"},
		{"noise22","1001","0","0"},
		{"deviceid23","1001","0","0"},
		{"signallevel23","1001","0","0"},
		{"noise23","1001","0","0"},
		{"deviceid24","1001","0","0"},
		{"signallevel24","1001","0","0"},
		{"noise24","1001","0","0"},
		{"deviceid25","1001","0","0"},
		{"signallevel25","1001","0","0"},
		{"noise25","1001","0","0"},
		{"deviceid26","1001","0","0"},
		{"signallevel26","1001","0","0"},
		{"noise26","1001","0","0"},
		{"deviceid27","1001","0","0"},
		{"signallevel27","1001","0","0"},
		{"noise27","1001","0","0"},
		{"deviceid28","1001","0","0"},
		{"signallevel28","1001","0","0"},
		{"noise28","1001","0","0"},
		{"deviceid29","1001","0","0"},
		{"signallevel29","1001","0","0"},
		{"noise29","1001","0","0"},
		{"deviceid30","1001","0","0"},
		{"signallevel30","1001","0","0"},
		{"noise30","1001","0","0"},
		{"deviceid31","1001","0","0"},
		{"signallevel31","1001","0","0"},
		{"noise31","1001","0","0"},
		{"deviceid32","1001","0","0"},
		{"signallevel31","1001","0","0"},
		{"noise31","1001","0","0"},
		};

stInData* getSystemInfo(void)
{
	return g_stSystemInfo;
}

int busyHandle(void* ptr,int retry_times)
{
	printf("retry_times %d\n",retry_times);
	sqlite3_sleep(10);
	return 1;
}


int sqliteinit(void)
{
	sqlite3_mutex1 = CreateLock();
    int rc = sqlite3_open("/www/cgi-bin/test.db", &g_psqlitedb);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(g_psqlitedb));
        sqlite3_close(g_psqlitedb);
        return 1;
    }

	updateData_init();
	// else
	// {
	// 	sqlite3_busy_timeout(g_psqlitedb,3000);
	// }
	
}

static int select_meshinfo_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	char *zErrMsg = 0;
	char updateSql[300];
	int countTmp = 0;
	int txpower=39; 
	int freq=1470;   
	int rate=1;  
	int chanbw=0;
	int m_chanbw;
	int bcastmode;

	int rc;
	if(0 == strcmp(argv[0],"m_txpower"))
	{
		sscanf(argv[1], "%d", &txpower);
		txpower-=39;
		//printf("get txpower=%d  ",txpower);
	}
	if(0 == strcmp(argv[0],"rf_freq"))
	{
		sscanf(argv[1], "%d", &freq);
		//printf("get freq=%d  ",freq);
	}
	if(0 == strcmp(argv[0],"m_chanbw"))
	{
		sscanf(argv[1], "%d", &chanbw);
		if(chanbw==0)
		{
			m_chanbw=20;
		}
		else if(chanbw==1)
		{
			m_chanbw=10;
		}
		else if(chanbw==2)
		{
			m_chanbw=5;
		}
		else;
		//printf("get bw=%d  ",m_chanbw);

	}
	if(0 == strcmp(argv[0],"m_rate"))
	{
		sscanf(argv[1], "%d", &rate);
		printf("get rate=%d\r\n",rate);
	}
	if(0 == strcmp(argv[0],"m_distance"))
	{
	}
	if(0 == strcmp(argv[0],"m_ssid"))
	{
	}
	if(0 == strcmp(argv[0],"m_bcastmode"))
	{
		sscanf(argv[1], "%d", &bcastmode);  //组播传输模式 0：开启 1：关闭
	}

/* meshinfo表内容改变，更新到systeminfo中 */
		snprintf(updateSql, sizeof(updateSql), "UPDATE systemInfo SET value = %d WHERE name = '%s';\
				UPDATE systemInfo SET value = %d WHERE name = '%s'; \
				UPDATE systemInfo SET value = %d WHERE name = '%s'; \
				UPDATE systemInfo SET value = %d WHERE name = '%s'; " \
			,freq,"rf_freq",m_chanbw,"m_chanbw",txpower,"m_txpower",rate,"m_rate");
		sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
		Lock(&sqlite3_mutex1,0);

		
		while (SQLITE_OK != sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &zErrMsg)) {
			//fprintf(stderr, "SQL error1: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			countTmp ++;
			if(countTmp > 10)
				break;
			sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);

		}

	

		Unlock(&sqlite3_mutex1);
    return 0;
	
}

void select_meshinfo_toprint(void)
{
   char *sql="select * from meshInfo";
   char *errmsg;

   sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
   int rc = sqlite3_exec(g_psqlitedb,sql,select_meshinfo_callback , 0, &errmsg);
   if (rc != SQLITE_OK)
   {
       printf(stderr, "无法更新数据: %s\n", errmsg);
   }
}


static int sqlite_set_userinfo_callback(void *NotUsed, int argc, char **argv, char **azColName) {
	char *zErrMsg = 0;
	char updateSql[100];
	int rc;
	int counttmp = 0;
	uint8_t cmd[200];
	bool isset = FALSE;
	if(0 == strcmp(argv[2],"1"))
	{
//		printf("%s = %s  %s %s\n", azColName[2], argv[2],azColName[0],argv[0]);
		if(0 == strcmp(argv[0],"m_ip"))
		{
			sscanf(argv[1], "%d.%d.%d.%d", &SELFIP_s[0],&SELFIP_s[1],&SELFIP_s[2],&SELFIP_s[3]);
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,
					"ifconfig br0 %d.%d.%d.%d",
					SELFIP_s[0],SELFIP_s[1],SELFIP_s[2],SELFIP_s[3]);
			system(cmd);
				
			printf("set ------- br0 ip address = %d.%d.%d.%d\n", SELFIP_s[0],SELFIP_s[1],SELFIP_s[2],SELFIP_s[3]);
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,
					"sed -i \"s/ip_addr .*/ip_addr %s/g\" /mnt/node_xwg",
					argv[1]);
			system(cmd);
			system("sync");
			isset = TRUE;	
		}
		if(0 == strcmp(argv[0],"m_dhcpStart"))
		{
		}
		if(0 == strcmp(argv[0],"m_dhcpGateway"))
		{
		}
		if(0 == strcmp(argv[0],"m_dhcpDns"))
		{
		}

		snprintf(updateSql, sizeof(updateSql), "UPDATE userInfo SET state = '0' WHERE name = '%s';" \
					,argv[0]);
		
		sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
		Lock(&sqlite3_mutex1,0);
	    while (SQLITE_OK != sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &zErrMsg)) {
	        //fprintf(stderr, "callback error1: %s\n", zErrMsg);
	        counttmp ++;
	        if(counttmp > 10)
	        	break;
	        sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
	    }
	    Unlock(&sqlite3_mutex1);
		
		if(isset)
		{
			isset=FALSE;
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,
				"cp /www/cgi-bin/test.db /www/cgi");
			system(cmd);
		}
	}
    return 0;
}

static int sqlite_set_meshinfo_callback(void *NotUsed, int argc, char **argv, char **azColName) {
	char *zErrMsg = 0;
	bool isset = FALSE;
	char updateSql[100];
	int countTmp = 0;
	int m_chanbw;
	int m_rate;
	int m_freq;
	int m_power;
	uint8_t bcastmode;
	int workmode;//0:开启 1：关闭
	int rc;
	static int SOCKET_BCAST; 
	//INT8 bcast_buf[BUFLEN];
	static struct sockaddr_in S_GROUND_BCAST;

	INT8 buffer[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];
	INT32 buflen = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);
	Smgmt_header* mhead = (Smgmt_header*)buffer;
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;
	uint8_t cmd[200]; 
	bzero(buffer, buflen);
	memset(cmd,0,sizeof(cmd));
	mhead->mgmt_head = htons(HEAD);
	mhead->mgmt_len = sizeof(Smgmt_set_param);
	mhead->mgmt_type = 0;

	stInData stsysteminfodata;
	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));


	if(0 == strcmp(argv[4],"1"))
	{
		if(0 == strcmp(argv[0],"m_txpower"))
		{
			//mhead->mgmt_type |= MGMT_SET_POWER;
			sscanf(argv[1], "%d", &(mparam->mgmt_mac_txpower));			
			mparam->mgmt_mac_txpower = htons(mparam->mgmt_mac_txpower);		
			meshinfo.m_txpower=mparam->mgmt_mac_txpower;
/*  更新宽带参数 */
			sscanf(argv[1], "%d", &(m_power));
			meshinfo.sys_power=m_power;
			uint16_t txpower_channels[POWER_CHANNEL_NUM];
			txpower_lookup_channels(m_power, txpower_channels);
			for (int i = 0; i < POWER_CHANNEL_NUM; ++i) {
				meshinfo.m_txpower_ch[i] = htons(txpower_channels[i]);
			}
//			printf("get txpower:%d  \r\n",m_power-39);
			 memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
			 sprintf(stsysteminfodata.name,"%s","m_txpower");
			 sprintf(stsysteminfodata.value,"%d",39-m_power);
			 stsysteminfodata.state[0] = '1';
			 updateData_systeminfo(stsysteminfodata);
		}
		if(0 == strcmp(argv[0],"rf_freq"))
		{
			//mhead->mgmt_type |= MGMT_SET_FREQUENCY;
			 sscanf(argv[1], "%d", &(mparam->mgmt_mac_freq));			
			mparam->mgmt_mac_freq = htonl(mparam->mgmt_mac_freq);
			meshinfo.rf_freq=mparam->mgmt_mac_freq;		
			//isset = TRUE;
			meshinfo.freq_isset=1;

			sscanf(argv[1], "%d", &(m_freq));
			meshinfo.sys_freq=m_freq;
//			printf("get freq:%d  \r\n",m_freq);
			memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
			sprintf(stsysteminfodata.name,"%s","rf_freq");
			sprintf(stsysteminfodata.value,"%d",m_freq);
			stsysteminfodata.state[0] = '1';
			updateData_systeminfo(stsysteminfodata);
		}
		if(0 == strcmp(argv[0],"m_chanbw"))
		{
			//mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
			sscanf(argv[1], "%d", &(mparam->mgmt_mac_bw));
			meshinfo.m_chanbw=mparam->mgmt_mac_bw;
			//isset = TRUE;
			
			meshinfo.chanbw_isset=1;
			
			if(mparam->mgmt_mac_bw==0)
			{
			m_chanbw=20;
			}
			else if(mparam->mgmt_mac_bw==1)
			{
			m_chanbw=10;
			}
			else if(mparam->mgmt_mac_bw==2)
			{
			m_chanbw=5;
			}
			else;

			meshinfo.sys_bw=mparam->mgmt_mac_bw;
//			printf("get bw=%d  \r\n",m_chanbw);
			memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
			sprintf(stsysteminfodata.name,"%s","m_chanbw");
			sprintf(stsysteminfodata.value,"%d",m_chanbw);
			stsysteminfodata.state[0] = '1';
			updateData_systeminfo(stsysteminfodata);
		}
		if(0 == strcmp(argv[0],"m_rate"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(mparam->mgmt_virt_unicast_mcs));
			meshinfo.m_rate=mparam->mgmt_virt_unicast_mcs;
			//isset = TRUE;
			
			meshinfo.rate_isset=1;
			
			sscanf(argv[1], "%d", &(m_rate));
			meshinfo.sys_rate=m_rate;
			if(m_rate<0)
			{
				rate_auto=1;
				meshinfo.rate_isset=0;
			}
			
//			printf("get rate=%d  \r\n",mparam->mgmt_virt_unicast_mcs);
			memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
			sprintf(stsysteminfodata.name,"%s","m_rate");
			sprintf(stsysteminfodata.value,"%d",mparam->mgmt_virt_unicast_mcs);
			stsysteminfodata.state[0] = '1';
			updateData_systeminfo(stsysteminfodata);
		}
		if(0 == strcmp(argv[0],"m_distance"))
		{
		}
		if(0 == strcmp(argv[0],"m_ssid"))
		{
		}
		if(0 == strcmp(argv[0],"m_bcastmode"))
		{
			sscanf(argv[1], "%d", &(meshinfo.m_bcastmode));
			printf("bcast mode=%d\r\n",meshinfo.m_bcastmode);
			//meshinfo.bcastmode_isset=1;
			//isset = TRUE;			
		}
		if(0 == strcmp(argv[0],"workmode"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(mparam->mgmt_net_work_mode.NET_work_mode));
			meshinfo.workmode=mparam->mgmt_net_work_mode.NET_work_mode;
			//isset = TRUE;
			
			meshinfo.workmode_isset=1;
			
			sscanf(argv[1], "%d", &(workmode));
			meshinfo.sys_workmode=workmode;
			
//			printf("get workmode=%d  \r\n",mparam->mgmt_net_work_mode);
		}
// add  by sdg 20250625
		if(0 == strcmp(argv[0],"m_route"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_route));
			
			meshinfo.route_isset=1;
						
			//printf("get route=%d  \r\n",meshinfo.m_route);
		}

		if(0 == strcmp(argv[0],"m_slot_len"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_slot_len));
			
			meshinfo.slot_isset=1;
			
			//printf("get %d, slot info=%lf  \r\n",meshinfo.m_slot_len,slot_info[meshinfo.m_slot_len]);
		}


		if(0 == strcmp(argv[0],"m_trans_mode"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_trans_mode));
			meshinfo.trans_mode_isset=1;
						
			//printf("get transmode=%d  \r\n",meshinfo.m_trans_mode);
		}
		if(0 == strcmp(argv[0],"m_select_freq1"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_select_freq_1));
			meshinfo.select_freq_isset=1;
						
			//printf("get m_select_freq1=%d  \r\n",meshinfo.m_select_freq_1);
		}
		if(0 == strcmp(argv[0],"m_select_freq2"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_select_freq_2));
			meshinfo.select_freq_isset=1;
						
			//printf("get m_select_freq2=%d  \r\n",meshinfo.m_select_freq_2);
		}		
		if(0 == strcmp(argv[0],"m_select_freq3"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_select_freq_3));
			meshinfo.select_freq_isset=1;
						
			//printf("get m_select_freq3=%d  \r\n",meshinfo.m_select_freq_3);
		}
		if(0 == strcmp(argv[0],"m_select_freq4"))
		{
			//mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
			sscanf(argv[1], "%d", &(meshinfo.m_select_freq_4));
			meshinfo.select_freq_isset=1;
						
			//printf("get m_select_freq3=%d  \r\n",meshinfo.m_select_freq_3);
		}
		if(0 == strcmp(argv[0],"power_level"))
		{
			int level = 0;
			sscanf(argv[1], "%d", &level);
			meshinfo.power_level = (uint8_t)level;
			meshinfo.sys_power_level = level;
			meshinfo.power_level_isset = 1;
		}
		if(0 == strcmp(argv[0],"power_attenuation"))
		{
			int attenuation = 0;
			sscanf(argv[1], "%d", &attenuation);
			meshinfo.power_attenuation = (uint8_t)attenuation;
			meshinfo.sys_power_attenuation = attenuation;
			meshinfo.power_attenuation_isset = 1;
		}				
		// add by sdg

			// mhead->mgmt_type = htons(mhead->mgmt_type);
			// mgmt_netlink_set_param(buffer, buflen,NULL);
			// sprintf(cmd,
			// 	"cp /www/cgi-bin/test.db /www/cgi");
			// system(cmd);


//		printf("%s = %s  %s %s\n", azColName[4], argv[4],azColName[0],argv[0]);
/*  meshInfo表中各参数的state值置0  */
		snprintf(updateSql, sizeof(updateSql), "UPDATE meshInfo SET state = '0' WHERE name = '%s';" \
					,argv[0]);
		
		// snprintf(updateSql, sizeof(updateSql), "UPDATE meshInfo SET state = '0' WHERE state ='1';" \
		// 			);		
        	sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
		Lock(&sqlite3_mutex1,0);

		
		while (SQLITE_OK != sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &zErrMsg)) {
			fprintf(stderr, "SQL error1: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			countTmp ++;
			if(countTmp > 5)
				break;
			sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);

		}
		Unlock(&sqlite3_mutex1);

	}
    return 0;
}


int sqlite_set_param(void) {
    char *zErrMsg = 0;
    int rc;
	int opt=1;
    bool isset=FALSE;
	int ret;
//    // 创建一个触发器，在表的UPDATE操作后发送socket通知
//    const char *sql = "DROP TRIGGER if exists after_update_send_socket;";
//    				  "create table IF NOT EXISTS socket_queue_table(message varchar(20) NOT null primary key);"
//    				  "CREATE TRIGGER after_update_send_socket AFTER UPDATE ON userInfo FOR EACH ROW BEGIN "
//                      "INSERT INTO socket_queue_table(message) VALUES('Table updated'); END;";
//    rc = sqlite3_exec(g_psqlitedb, sql, callback, 0, &zErrMsg);
//    if (rc != SQLITE_OK) {
//        fprintf(stderr, "SQL error: %s\n", zErrMsg);
//        sqlite3_free(zErrMsg);
//    }
	
	double slot_info[]={0.5,1,1.25,2};     //时隙长度


	char bcast_buf[1000];
	uint8_t cmd[200];
	INT8 buffer[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];
	INT32 buflen = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);
	Smgmt_header* mhead = (Smgmt_header*)buffer;
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;
	bzero(buffer, buflen);
	memset(cmd,0,sizeof(cmd));
	mhead->mgmt_head = htons(HEAD);
	mhead->mgmt_len = sizeof(Smgmt_set_param);
	mhead->mgmt_type = 0;

	

	stInData stsysteminfodata;
	
	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	memset((char*)&meshinfo,0,sizeof(meshinfo));
	

	meshinfo.m_bcastmode=1; //初始化shi非组播模式

	S_GROUND_BCAST.sin_family=AF_INET;
	S_GROUND_BCAST.sin_addr.s_addr = inet_addr("192.168.2.255"); //设置成广播
	S_GROUND_BCAST.sin_port = htons(BCAST_SEND_PORT);
	SOCKET_BCAST_SEND=socket (AF_INET,SOCK_DGRAM, 0 );
	if(SOCKET_BCAST_SEND <= 0)
	{
		printf("create bcast socket failed\r\n");
		//return -1;
	}
	setsockopt(SOCKET_BCAST_SEND,SOL_SOCKET,SO_BROADCAST,&opt,sizeof(opt));//

    while (1) {

    	sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);

    	sqlite3_exec(g_psqlitedb,"SELECT * FROM userInfo;",sqlite_set_userinfo_callback, 0, &zErrMsg);
    	sqlite3_exec(g_psqlitedb,"SELECT * FROM meshInfo;",sqlite_set_meshinfo_callback, 0, &zErrMsg);

//add by sdg				

		if(meshinfo.m_bcastmode==1)
		{
			
			bzero(buffer, buflen);
		       	mhead->mgmt_head = htons(HEAD);
			mhead->mgmt_len = sizeof(Smgmt_set_param);
			mhead->mgmt_type = 0;
			/* 非组播模式*/
			//printf("非组播模式下设置参数\r\n");
			if(meshinfo.txpower_isset)
			{
				meshinfo.txpower_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_POWER;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_mac_txpower=meshinfo.m_txpower;
				memcpy(mparam->mgmt_mac_txpower_ch, meshinfo.m_txpower_ch, sizeof(meshinfo.m_txpower_ch));
				//printf("非组播模式 set txpower %d\r\n",mparam->mgmt_mac_txpower);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				
				printf("set power:%d \r\n",mparam->mgmt_mac_txpower);
			
			}
			if(meshinfo.power_level_isset)
			{
				meshinfo.power_level_isset=0;
				isset=TRUE;
				mhead->mgmt_keep |= MGMT_SET_POWER_LEVEL;
				mparam->mgmt_mac_power_level=meshinfo.power_level;
			}
			if(meshinfo.power_attenuation_isset)
			{
				meshinfo.power_attenuation_isset=0;
				isset=TRUE;
				mhead->mgmt_keep |= MGMT_SET_POWER_ATTENUATION;
				mparam->mgmt_mac_power_attenuation=meshinfo.power_attenuation;
			}
			if(meshinfo.freq_isset)
			{
				meshinfo.freq_isset=0;

				if(meshinfo.workmode==1)
				{
					/* 定频模式下配置中心频率 */
					isset=TRUE;
					mhead->mgmt_type |= MGMT_SET_FREQUENCY;
					mparam->mgmt_mac_freq=meshinfo.rf_freq;

					printf("set freq:%d \r\n",mparam->mgmt_mac_freq);

				}

				//mhead->mgmt_type = htons(mhead->mgmt_type);
				//printf("set freq %d\r\n",mparam->mgmt_mac_freq);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);

			}
			if(meshinfo.chanbw_isset)
			{
				meshinfo.chanbw_isset=0;
				isset=TRUE;				
				mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_mac_bw=meshinfo.m_chanbw;
				printf("set bw %d\r\n",mparam->mgmt_mac_bw);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);

			}
			if(meshinfo.rate_isset)
			{
				meshinfo.rate_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_virt_unicast_mcs=meshinfo.m_rate;
				//printf("set rate %d\r\n",mparam->mgmt_virt_unicast_mcs);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);

			}
			if(meshinfo.workmode_isset)
			{
				meshinfo.workmode_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_WORKMODE;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_net_work_mode.NET_work_mode=meshinfo.workmode;

				printf("set workmode:%d \r\n",mparam->mgmt_net_work_mode.NET_work_mode);
				if(meshinfo.workmode==4)
				{
					/* 自适应选频 */
					if(meshinfo.select_freq_isset==1)
					{
						meshinfo.select_freq_isset=0;
						printf("set select freq,freq1:%d,freq2:%d,freq3:%d，freq4:%d \r\n",
							meshinfo.m_select_freq_1,meshinfo.m_select_freq_2,meshinfo.m_select_freq_3,meshinfo.m_select_freq_4);
						mparam->mgmt_net_work_mode.fh_len=4;
						mparam->mgmt_net_work_mode.hop_freq_tb[0]=meshinfo.m_select_freq_1;
						mparam->mgmt_net_work_mode.hop_freq_tb[1]=meshinfo.m_select_freq_2;
						mparam->mgmt_net_work_mode.hop_freq_tb[2]=meshinfo.m_select_freq_3;
						mparam->mgmt_net_work_mode.hop_freq_tb[3]=meshinfo.m_select_freq_4;

					}
					 
				}

			}
			if(meshinfo.route_isset)
			{
				meshinfo.route_isset=0;
				//sprintf(cmd,"./home/root/cs.sh");
				switch(meshinfo.m_route) {
					case KD_ROUTING_OLSR:
						printf("set route olsr \r\n");
						 ret = system("/home/root/cs.sh");
						if(ret == -1) printf("change olsr failed\r\n");
						break;
						
					case KD_ROUTING_AODV:
						// aodv
						printf("set route aodv \r\n");
						break;
						
					case KD_ROUTING_CROSS_LAYER:  //batman
						printf("set route batman \r\n");
						 ret = system("/home/root/cs_batman.sh");
						if(ret == -1) printf("change batman failed\r\n");

						break;
						
					default:
						break;
				}				
				
			}
			if(meshinfo.slot_isset)
			{	
				meshinfo.slot_isset=0;
				isset=TRUE;
				mhead->mgmt_keep |= MGMT_SET_SLOTLEN;
				mparam->u8Slotlen=meshinfo.m_slot_len;
				printf("set slot len %lf \r\n",slot_info[mparam->u8Slotlen]);

			}

			if(meshinfo.trans_mode_isset)
			{
				meshinfo.trans_mode_isset=0;
				//zmhead->mgmt_type |= MGMT_SET_WORKMODE;

				printf("set trans_mode %d \r\n");
			}
			// if(meshinfo.select_freq_isset)
			// {
			// 	meshinfo.select_freq_isset=0;
			// 	printf("set select freq: %d %d %d \r\n",meshinfo.m_select_freq_1,meshinfo.m_select_freq_2,meshinfo.m_select_freq_3);
			// }

			if(isset)
			{
				isset=FALSE;
				mhead->mgmt_type = htons(mhead->mgmt_type);
				mhead->mgmt_keep = htons(mhead->mgmt_keep);
				mgmt_netlink_set_param(buffer, buflen,NULL);	
				sprintf(cmd,
					"cp /www/cgi-bin/test.db /www/cgi");
				system(cmd);
			}

		}	

		//组播模式
		else 
		{
			//printf("组播模式,发送组播包\r\n");
			bzero(buffer, buflen);
		       	mhead->mgmt_head = htons(HEAD);
			mhead->mgmt_len = sizeof(Smgmt_set_param);
			mhead->mgmt_type = 0;
			memcpy(bcast_buf,&meshinfo,sizeof(meshinfo));
			if(SendUDPClient(SOCKET_BCAST_SEND,bcast_buf,sizeof(meshinfo),&S_GROUND_BCAST)<0)
			{
				printf("send broadcast packet fail\r\n");
			}
			sleep(4); //延时4秒后修改自身参数
			if(meshinfo.txpower_isset)
			{
				meshinfo.txpower_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_POWER;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_mac_txpower=meshinfo.m_txpower;
				memcpy(mparam->mgmt_mac_txpower_ch, meshinfo.m_txpower_ch, sizeof(meshinfo.m_txpower_ch));
				//printf("set txpower %d \r\n",mparam->mgmt_mac_txpower);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				
			
			}
			if(meshinfo.power_level_isset)
			{
				meshinfo.power_level_isset=0;
				isset=TRUE;
				mhead->mgmt_keep |= MGMT_SET_POWER_LEVEL;
				mparam->mgmt_mac_power_level=meshinfo.power_level;
			}
			if(meshinfo.power_attenuation_isset)
			{
				meshinfo.power_attenuation_isset=0;
				isset=TRUE;
				mhead->mgmt_keep |= MGMT_SET_POWER_ATTENUATION;
				mparam->mgmt_mac_power_attenuation=meshinfo.power_attenuation;
			}
			if(meshinfo.freq_isset)
			{
				meshinfo.freq_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_FREQUENCY;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_mac_freq=meshinfo.rf_freq;
				//printf("set freq %d \r\n",mparam->mgmt_mac_freq);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);
				
			}
			if(meshinfo.chanbw_isset)
			{
				meshinfo.chanbw_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_mac_bw=meshinfo.m_chanbw;
				//printf("set bw %d \r\n",mparam->mgmt_mac_bw);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);
				
			}
			if(meshinfo.rate_isset)
			{
				meshinfo.rate_isset=0;
				mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
				isset=TRUE;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_virt_unicast_mcs=meshinfo.m_rate;
				//printf("set rate %d \r\n",mparam->mgmt_virt_unicast_mcs);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);
				
			}
			if(meshinfo.workmode_isset)
			{
				meshinfo.workmode_isset=0;
				isset=TRUE;
				mhead->mgmt_type |= MGMT_SET_WORKMODE;
				//mhead->mgmt_type = htons(mhead->mgmt_type);
				mparam->mgmt_net_work_mode.NET_work_mode=meshinfo.workmode;
				//printf("set rate %d\r\n",mparam->mgmt_virt_unicast_mcs);
				//mgmt_netlink_set_param(buffer, buflen,NULL);
				// sprintf(cmd,
				// 	"cp /www/cgi-bin/test.db /www/cgi");
				// system(cmd);

			}
			if(isset)
			{
				isset=FALSE;
				mhead->mgmt_type = htons(mhead->mgmt_type);
				mhead->mgmt_keep = htons(mhead->mgmt_keep);
				mgmt_netlink_set_param(buffer, buflen,NULL);
				sprintf(cmd,
					"cp /www/cgi-bin/test.db /www/cgi");
				system(cmd);
			}


		}	//CloseUDPSocket(SOCKET_BCAST);			
	
			    
		sleep(1);
    }

    sqlite3_close(g_psqlitedb);
    return 0;
}

void updateData_systeminfo(stInData data)
{
    char updateSql[SQLDATALEN];
	//sqlite3 *g_psqlitedb;
    int rc ;

    snprintf(updateSql, sizeof(updateSql), "UPDATE systemInfo SET value = '%s', state = '%s', lib = '%s' WHERE name = '%s';" \
    		, data.value, data.state,data.lib,data.name);

    char* errMsg;
    sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
    Lock(&sqlite3_mutex1,0);
   
	rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf(stderr, "无法更新数据: %s\n", errMsg);
    }

    Unlock(&sqlite3_mutex1);
}

void updateData_meshinfo(stInData data)
{
    char updateSql[SQLDATALEN];
	//sqlite3 *g_psqlitedb;
    int rc ;

    snprintf(updateSql, sizeof(updateSql), "UPDATE meshInfo SET value = '%s', state = '%s', lib = '%s' WHERE name = '%s';" \
    		, data.value, data.state,data.lib,data.name);

    char* errMsg;
    sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
    Lock(&sqlite3_mutex1,0);
   
	rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf(stderr, "无法更新数据: %s\n", errMsg);
    }

    Unlock(&sqlite3_mutex1);
}

/*
void updateData_surveyInfo(stSurveyInfo data)
{
    char updateSql[SQLDATALEN];
    int i = 0;
    sprintf(updateSql, "UPDATE surveyInfo SET id = '%d',",data.id);
    for(; i < 301; i ++)
    {

    }


    snprintf(updateSql, sizeof(updateSql), "UPDATE surveyInfo SET value = '%s', state = '%s', lib = '%s' WHERE name = '%s';" \
    		, data.value, data.state,data.lib,data.name);

    char* errMsg;
    int rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf(stderr, "无法更新数据: %s\n", errMsg);
    }
}*/


/*
void updateData_nodeinfo(stNode data)
{
    char updateSql[SQLDATALEN];
    snprintf(updateSql, sizeof(updateSql), "UPDATE node SET value = '%s', state = '%s', lib = '%s' WHERE name = '%s';" \
    		, value, state,lib,param);

    char* errMsg;
    int rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf(stderr, "无法更新数据: %s\n", errMsg);
    }
}
*/

void updateData_linkinfo(stLink *data,int cnt,int selfid)
{
    char updateSql[SQLDATALEN];
	char snr[20];
	char getlv[20];
	char flowrate[20];
	
	int rc;
	
	snprintf(snr,sizeof(snr),"snr%d",data->m_stNbInfo[cnt].nbid1);
	snprintf(getlv,sizeof(getlv),"getlv%d",data->m_stNbInfo[cnt].nbid1);
	snprintf(flowrate,sizeof(flowrate),"flowrate%d",data->m_stNbInfo[cnt].nbid1);

    //printf("link neigh_%d: snr:%d getlv:%d  flowrate:%d  \r\n",data->m_stNbInfo[cnt].nbid1,data->m_stNbInfo[cnt].snr1,);
    snprintf(updateSql, sizeof(updateSql), "UPDATE link SET  %s = %d, %s = %d,%s = %d WHERE id = %d;",\
    		snr,data->m_stNbInfo[cnt].snr1,getlv,data->m_stNbInfo[cnt].getlv1,flowrate,data->m_stNbInfo[cnt].flowrate1,selfid);
	
    char* errMsg;

	sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
    Lock(&sqlite3_mutex1,0);

    rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf(stderr, "无法更新数据: %s\n", errMsg);
    }

	Unlock(&sqlite3_mutex1);

}

void updateData_timeslotinfo(unsigned char           data, int selfid)
{
    char updateSql[SQLDATALEN]; 
	int rc;
    //printf("link neigh_%d: snr:%d getlv:%d  flowrate:%d  \r\n",data->m_stNbInfo[cnt].nbid1,data->m_stNbInfo[cnt].snr1,);
    snprintf(updateSql, sizeof(updateSql), "UPDATE timeslot SET color = %d WHERE id = %d;",data,selfid);
	
    char* errMsg;

	sqlite3_busy_handler(g_psqlitedb,busyHandle,NULL);
    Lock(&sqlite3_mutex1,0);

    rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf(stderr, "无法更新数据: %s\n", errMsg);
    }

	Unlock(&sqlite3_mutex1);

}


//void updateData_userinfo(stInData data)
//{
//    char updateSql[SQLDATALEN];
//    snprintf(updateSql, sizeof(updateSql), "UPDATE userInfo SET value = '%s', state = '%s', lib = '%s' WHERE name = '%s';" \
//    		, data.value, data.state,data.lib,data.name);
//
//    char* errMsg;
//    int rc = sqlite3_exec(g_psqlitedb, updateSql, NULL, 0, &errMsg);
//    if (rc != SQLITE_OK)
//    {
//        printf(stderr, "无法更新数据: %s\n", errMsg);
//    }
//}
//




/*
int mgmt_sqlite_report_systemInfo(void)
{
   // sqlite3 *db = NULL;
    char *err_msg = NULL;
	char            sqlname[2048 * 10];
	int             sqlnamelen;
	char            sqld[2048 * 10];
	int             sqldlen;

	sqlnamelen = sprintf(sqlname,
		"create table IF NOT EXISTS systemInfo("
		"name varchar NOT null primary key,"
		"value varchar NOT null, "
		"state varchar,"
		"lib varchar);");

    int rc = sqlite3_exec(g_psqlitedb, sqlname, NULL, NULL, &err_msg);

    updateData(g_psqlitedb,"systemInfo","ipaddr","192.168.2.10","1","0");
    updateData(g_psqlitedb,"systemInfo","device","1002","1","0");
    updateData(g_psqlitedb,"systemInfo","g_ver","001","1","0");
    updateData(g_psqlitedb,"systemInfo","rf_freq","2450","1","0");
    updateData(g_psqlitedb,"systemInfo","m_chanbw","10","1","0");
    updateData(g_psqlitedb,"systemInfo","m_txpower","1","1","0");
    updateData(g_psqlitedb,"systemInfo","m_rate","3","1","0");
//    updateData(db,"systemInfo","ipaddr","192.168.2.10","1","0");
//    updateData(db,"systemInfo","ipaddr","192.168.2.10","1","0");

    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(g_psqlitedb);

        return 1;
    }

    sleep(2);
    updateData(g_psqlitedb,"userInfo","m_ip","192.168.2.99","1","0");
    return 0;
}*/

int systemexit(void){
	if(g_psqlitedb)
		sqlite3_close(g_psqlitedb);
}







