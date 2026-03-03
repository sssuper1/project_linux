/*
 * mgmt_transmit.c
 *
 *  Created on: Jan 19, 2021
 *      Author: slb
 */

#include "mgmt_types.h"
#include "mgmt_netlink.h"
#include "mgmt_transmit.h"
#include "socketUDP.h"
#include "SocketTCP.h"
#include "Lock.h"
#include "Pcap.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "sqlite_unit.h"
#include <stdbool.h>
#include "wg_config.h"
#include "audio_uart.h"
#include "ui_get.h"
#define BUFLEN 1500
#define CONNECTNUM 10
#define CTRL_PORT   16000
#define MGMT_PORT   16001
#define GROUND_PORT 16001
#define WG_RX_UDP_PORT  7600
#define WG_TX_UDP_PORT  7700
#define WG_RX_TCP_PORT  7601
#define WG_TX_TCP_PORT  7701
#define BCAST_RECV_PORT   7901
uint8_t SELFID;
uint32_t SELFIP;
uint8_t SELFIP_s[4];
static int SOCKET_MGMT;
static int SOCKET_GROUND;
static int SOCKET_UDP_WG;
static int SOCKET_TCP_WG;
static int SOCKET_BCAST_RECV;//add by sdg
static int SOCKET_MEM_REQUEST_RECV;  //add by sdg
//static int SOCKET_AUDIO_RECV; //add by sdg 20250915
extern int serial_port;
extern int audio_read;
//extern int SOCKET_BCAST;

static pthread_cond_t TCPCLIENTCOND_WG;
static pthread_mutex_t TCPCLIENTMUTEX_WG;
static TcpClient TCPCLIENT_WG[CONNECTNUM];
static struct sockaddr_in S_GROUND_STD;
static struct sockaddr_in S_GROUND_PC;
static struct sockaddr_in S_GROUND;
static struct sockaddr_in S_GROUND_WG;
static struct sockaddr_in S_OTHER_NODE;
static struct sockaddr_in S_GROUND_BCAST;   //组播 add by sdg

#define GROUND_STA 1
uint32_t FREQ_INIT;
uint16_t POWER_INIT;
uint8_t BW_INIT;
uint8_t MCS_INIT;
uint16_t MACMODE_INIT;
uint8_t NET_WORKMOD_INIT;
uint8_t POWER_LEVEL_INIT;
uint8_t POWER_ATTENUATION_INIT;
uint8_t RX_CHANNEL_MODE_INIT;
uint8_t DEVICETYPE_INIT;
uint32_t HOP_FREQ_TB_INIT[HOP_FREQ_NUM];
uint8_t g_u8SLOTLEN;

#define TXPOWER_TABLE_FILE "/etc/9361_table"

uint8_t g_cj_state=1;
char version[20];

static uint16_t g_txpower_table[POWER_TABLE_SIZE][POWER_CHANNEL_NUM];
static bool g_txpower_table_initialized;

static const uint16_t g_default_txpower_table[POWER_TABLE_SIZE][POWER_CHANNEL_NUM] = {
	{0, 0, 0, 0},
	{1, 1, 1, 1},
	{2, 2, 2, 2},
	{3, 3, 3, 3},
	{4, 4, 4, 4},
	{5, 5, 5, 5},
	{6, 6, 6, 6},
	{7, 7, 7, 7},
	{8, 8, 8, 8},
	{9, 9, 9, 9},
	{10, 10, 10, 10},
	{11, 11, 11, 11},
	{12, 12, 12, 12},
	{13, 13, 13, 13},
	{14, 14, 14, 14},
	{15, 15, 15, 15},
	{16, 16, 16, 16},
	{17, 17, 17, 17},
	{18, 18, 18, 18},
	{19, 19, 19, 19},
	{20, 20, 20, 20},
	{21, 21, 21, 21},
	{22, 22, 22, 22},
	{23, 23, 23, 23},
	{24, 24, 24, 24},
	{25, 25, 25, 25},
	{26, 26, 26, 26},
	{27, 27, 27, 27},
	{28, 28, 28, 28},
	{29, 29, 29, 29},
	{30, 30, 30, 30},
	{31, 31, 31, 31},
	{32, 32, 32, 32},
	{33, 33, 33, 33},
	{34, 34, 34, 34},
	{35, 35, 35, 35},
	{36, 36, 36, 36},
	{37, 37, 37, 37},
	{38, 38, 38, 38},
	{39, 39, 39, 39},
	{40, 40, 40, 40},
	{41, 41, 41, 41},
	{42, 42, 42, 42},
	{43, 43, 43, 43},
	{44, 44, 44, 44},
	{45, 45, 45, 45},
	{46, 46, 46, 46},
	{47, 47, 47, 47},
	{48, 48, 48, 48},
	{49, 49, 49, 49},
	{50, 50, 50, 50},
	{51, 51, 51, 51},
	{52, 52, 52, 52},
	{53, 53, 53, 53},
	{54, 54, 54, 54},
	{55, 55, 55, 55},
	{56, 56, 56, 56},
	{57, 57, 57, 57},
	{58, 58, 58, 58},
	{59, 59, 59, 59},
	{60, 60, 60, 60},
	{61, 61, 61, 61},
	{62, 62, 62, 62},
	{63, 63, 63, 63},
	{64, 64, 64, 64},
	{65, 65, 65, 65},
	{66, 66, 66, 66},
	{67, 67, 67, 67},
	{68, 68, 68, 68},
	{69, 69, 69, 69},
	{70, 70, 70, 70},
};

static void txpower_table_use_defaults(void)
{
	memcpy(g_txpower_table, g_default_txpower_table, sizeof(g_default_txpower_table));
}

static void txpower_table_init(void)
{
	if (g_txpower_table_initialized)
		return;

	txpower_table_use_defaults();

	FILE* fp = fopen(TXPOWER_TABLE_FILE, "r");
	if (!fp) {
		printf("txpower: unable to open %s, using default table\n", TXPOWER_TABLE_FILE);
		g_txpower_table_initialized = true;
		return;
	}

	char line[256];
	uint16_t row = 0;
	while (row < POWER_TABLE_SIZE && fgets(line, sizeof(line), fp)) {
		/* skip comments or empty lines */
		char* ptr = line;
		while (*ptr == ' ' || *ptr == '\t')
			ptr++;
		if (*ptr == '\0' || *ptr == '\n' || *ptr == '#')
			continue;

		unsigned int values[POWER_CHANNEL_NUM] = {0};
		int count = sscanf(ptr, "%u %u %u %u",
			&values[0], &values[1], &values[2], &values[3]);
		if (count <= 0)
			continue;

		if (count == 1) {
			values[1] = values[0];
			values[2] = values[0];
			values[3] = values[0];
		} else {
			unsigned int last = values[count - 1];
			for (int i = count; i < POWER_CHANNEL_NUM; ++i)
				values[i] = last;
		}

		for (int i = 0; i < POWER_CHANNEL_NUM; ++i) {
			if (values[i] > UINT16_MAX)
				values[i] = UINT16_MAX;
			g_txpower_table[row][i] = (uint16_t)values[i];
		}
		row++;
	}

	if (row < POWER_TABLE_SIZE) {
		printf("txpower: %s provides %u rows (<%u), remaining rows use defaults\n",
			TXPOWER_TABLE_FILE, row, POWER_TABLE_SIZE);
	}

	fclose(fp);
	g_txpower_table_initialized = true;
}

void txpower_lookup_channels(uint16_t single, uint16_t channel_power[POWER_CHANNEL_NUM])
{
	txpower_table_init();
	uint16_t index = single;
	if (index >= POWER_TABLE_SIZE) {
		index = POWER_TABLE_SIZE - 1;
	}
	memcpy(channel_power, g_txpower_table[index], sizeof(uint16_t) * POWER_CHANNEL_NUM);
}

void txpower_lookup_channels_be(uint16_t single, uint16_t channel_power[POWER_CHANNEL_NUM])
{
	uint16_t host_values[POWER_CHANNEL_NUM];
	txpower_lookup_channels(single, host_values);
	for (int i = 0; i < POWER_CHANNEL_NUM; ++i) {
		channel_power[i] = htons(host_values[i]);
	}
}

/* 用于上报的全局参数 */
Global_Radio_Param g_radio_param;
extern int audio_cnt_recv;
extern Info_0x06_Statistics stat_info;
 

#ifdef Radio_CEC

static int SOCKET_UDP_BEAM;
#define BEAM_TX_UDP_PORT  7801
#define BEAM_RX_UDP_PORT  5005

#define BEAM_STATE_NUM 128
#define BEAM_DATA_LEN 1420

#define START_TIME 10
#define BEAM_HEADER_LEN 12
#define BEAM_STATE_LEN  11


#define SLOT_CYCLE 8
#define Sub_SCAN_Slots 1
#define Sub_CONTROL_Slots 1
#define Sub_TRAFFIC_Slots 15
#define TRAFFIC_SLOT_REPEAT_CYCLE 5

#define Sub_N_Slots  (Sub_CONTROL_Slots + Sub_TRAFFIC_Slots)
#define TOTAL_SLOT_NUM (Sub_N_Slots * SLOT_CYCLE)
#define CONTROL_SLOT_NUM (Sub_CONTROL_Slots * SLOT_CYCLE)
#define TRAFFIC_SLOT_NUM (Sub_TRAFFIC_Slots * SLOT_CYCLE)
#define NEIGBOR_NUM 4

uint8_t NET_WORKMOD_INIT;
uint8_t NCU_NODE_ID_INIT;

uint8_t Send_Beam_num;
uint8_t Rec_Beam_num;
uint8_t Neigbor_direction[4];
uint32_t frame_cnt;
 uint32_t pps_cnt;
static struct sockaddr_in BEAM_RX_add;

#endif

Smgmt_phy PHY_MSG;
static uint32_t mysql_num;
static BOOL ISLOGIN;
//add by yang
int is_conned = 0;//网关节点判断是否和网管连接
int gotRequest = 0;//邻居节点判断是否收到拓扑请求，收到后持续向网关发送拓扑信息
struct sockaddr_in wg_addr;//用来存放网管的地址信息
struct sockaddr_in gate_addr;//用于邻居节点存放网关的地址信息
double longitude;
double latitude;


/*extern param*/
extern uint8_t  rate_auto;
//msg
static int MSG_MGMT;
void SendNodeMsg(void* data, int datalen);

void mgmt_param_init() {
	INT8 buffer[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];
	INT32 buflen = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);
	Smgmt_header* mhead = (Smgmt_header*)buffer;
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;
	bzero(buffer, buflen);
	mhead->mgmt_head = htons(HEAD);
	mhead->mgmt_len = sizeof(Smgmt_set_param);
	mhead->mgmt_type = 0;
	

	mhead->mgmt_type |= MGMT_SET_POWER;
	mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
	mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
	mhead->mgmt_type |= MGMT_SET_TEST_MODE;
	mhead->mgmt_type |= MGMT_SET_PHY;
	mhead->mgmt_keep |= MGMT_SET_SLOTLEN;
	mhead->mgmt_keep |= MGMT_SET_POWER_LEVEL;
	mhead->mgmt_keep |= MGMT_SET_POWER_ATTENUATION;
	mhead->mgmt_keep |= MGMT_SET_RX_CHANNEL_MODE;
	if(NET_WORKMOD_INIT == FIX_FREQ_MODE)
	{
		mhead->mgmt_type |= MGMT_SET_FREQUENCY;
	}
	mhead->mgmt_type |= MGMT_SET_WORKMODE;
	
	mhead->mgmt_type = htons(mhead->mgmt_type);
	mhead->mgmt_keep = htons(mhead->mgmt_keep);
	mparam->mgmt_mac_freq = htonl(FREQ_INIT);
	mparam->mgmt_mac_txpower = htons(POWER_INIT);
	txpower_lookup_channels_be(POWER_INIT, mparam->mgmt_mac_txpower_ch);
	mparam->mgmt_mac_power_level = POWER_LEVEL_INIT;
	mparam->mgmt_mac_power_attenuation = POWER_ATTENUATION_INIT;
	mparam->mgmt_rx_channel_mode = RX_CHANNEL_MODE_INIT;
	mparam->mgmt_mac_bw = BW_INIT;
	mparam->mgmt_virt_unicast_mcs = MCS_INIT;
	mparam->mgmt_mac_work_mode = htons(MACMODE_INIT);
	mparam->mgmt_phy = PHY_MSG;
	mparam->mgmt_phy.phy_pre_STS_thresh = htons(mparam->mgmt_phy.phy_pre_STS_thresh);
	mparam->mgmt_phy.phy_pre_LTS_thresh = htons(mparam->mgmt_phy.phy_pre_LTS_thresh);
	mparam->mgmt_phy.phy_tx_iq0_scale = htons(mparam->mgmt_phy.phy_tx_iq0_scale);
	mparam->mgmt_phy.phy_tx_iq1_scale = htons(mparam->mgmt_phy.phy_tx_iq1_scale);
	mparam->mgmt_net_work_mode.NET_work_mode=NET_WORKMOD_INIT;
	mparam->u8Slotlen = g_u8SLOTLEN;

	mgmt_netlink_set_param(buffer, buflen, NULL);

	memset(&g_radio_param,0,sizeof(Global_Radio_Param));
	g_radio_param.g_chanbw=BW_INIT;
	g_radio_param.g_rate=MCS_INIT;
	g_radio_param.g_rf_freq=FREQ_INIT;
	g_radio_param.g_route=3;
	g_radio_param.g_slot_len=g_u8SLOTLEN;
	g_radio_param.g_txpower=POWER_INIT;
	g_radio_param.g_workmode=NET_WORKMOD_INIT;


}

void mgmt_mysql_init(void) {
	FILE* file;
	FILE* file_node;
	FILE* file_hop;
	char nodemessage[100];
	char messagename[100];
	char data[10];
	int id;
	int param[9];
	char ifname[] = "br0";
	int on = 1;

	int ret;
	int broadcast = 1;
	uint8_t cmd[200];
	int row_cnt;
	int i;
	char data_hop[100];
	char hop_freq_msg[100];

	bzero(param, sizeof(param));
	bzero(version,sizeof(version));
	POWER_LEVEL_INIT = 0;
	POWER_ATTENUATION_INIT = 0;
	RX_CHANNEL_MODE_INIT = 0;
	mysql_num = 0;
	file = popen("cat /etc/node_id", "r");
	fread(data, sizeof(char), sizeof(data), file);
	sscanf(data, "%d", &id);
	pclose(file);

	if ((file_node = fopen("/etc/node_xwg", "r")) == NULL) {
		FREQ_INIT = 1478;
	}
	else {
		while (fgets(nodemessage, sizeof(nodemessage), file_node) != NULL) {
			bzero(messagename, sizeof(messagename));
			sscanf(nodemessage, "%s ", messagename);
			if (strcmp(messagename, "channel") == 0) {
				sscanf(nodemessage, "channel %d", &FREQ_INIT);
				printf("set ------- channel = %d\n", FREQ_INIT);
			}
			if (strcmp(messagename, "networkmode") == 0) {
				sscanf(nodemessage, "networkmode %d", &param[0]);
				NET_WORKMOD_INIT = param[0];
				printf("set ------- networkmode = %d\n", NET_WORKMOD_INIT);
			}
			if (strcmp(messagename, "devicetype") == 0) {
				sscanf(nodemessage, "devicetype %d", &param[0]);
				DEVICETYPE_INIT = param[0];
				printf("set ------- devicetype = %d\n", DEVICETYPE_INIT);
			}
			if (strcmp(messagename, "power") == 0) {
				sscanf(nodemessage, "power %d", &param[0]);
				POWER_INIT = param[0];
				printf("set ------- power = %d\n", POWER_INIT);
			}
			if (strcmp(messagename, "power_level") == 0) {
				sscanf(nodemessage, "power_level %d", &param[0]);
				POWER_LEVEL_INIT = param[0];
				printf("set ------- power_level = %d\n", POWER_LEVEL_INIT);
			}
			if (strcmp(messagename, "power_attenuation") == 0) {
				sscanf(nodemessage, "power_attenuation %d", &param[0]);
				POWER_ATTENUATION_INIT = param[0];
				printf("set ------- power_attenuation = %d\n", POWER_ATTENUATION_INIT);
			}
			if (strcmp(messagename, "rx_channel_mode") == 0) {
				sscanf(nodemessage, "rx_channel_mode %d", &param[0]);
				if (param[0] < 0)
					param[0] = 0;
				else if (param[0] > 4)
					param[0] = 4;
				RX_CHANNEL_MODE_INIT = param[0];
				printf("set ------- rx_channel_mode = %d\n", RX_CHANNEL_MODE_INIT);
			}
			if (strcmp(messagename, "bw") == 0) {
				sscanf(nodemessage, "bw %d", &param[0]);
				BW_INIT = param[0];
				printf("set ------- bw = %d\n", BW_INIT);
			}
			if (strcmp(messagename, "version") == 0) {
				sscanf(nodemessage, "version %s", &version);
				printf("set ------- version = %s\n", version);
			}
			if (strcmp(messagename, "mcs") == 0) {
				sscanf(nodemessage, "mcs %d", &param[0]);
#ifdef Radio_SWARM_WNW
                if(param[0] == 7)
                {
                    param[0] = 6;
				}
#endif
				MCS_INIT = param[0];
				printf("set ------- mcs = %d\n", MCS_INIT);
			}
			if (strcmp(messagename, "macmode") == 0) {
				sscanf(nodemessage, "macmode %d", &param[0]);
				MACMODE_INIT = param[0];
				printf("set ------- macmode = %d\n", MACMODE_INIT);
			}
			if (strcmp(messagename, "phymsg") == 0) {
#ifdef Radio_SWARM_S2
				sscanf(nodemessage, "phymsg %d %d %d %d %d %d %d %d %d", &param[0],
					&param[1], &param[2],
					&param[3], &param[4],
					&param[5],&param[6],
					&param[7],&param[8]);
				PHY_MSG.rf_agc_framelock_en = param[0];
				PHY_MSG.phy_cfo_bypass_en = param[1];
				PHY_MSG.phy_pre_STS_thresh = param[2];
				PHY_MSG.phy_pre_LTS_thresh = param[3];
				PHY_MSG.phy_tx_iq0_scale = param[4];
				PHY_MSG.phy_tx_iq1_scale = param[5];
				PHY_MSG.phy_msc_length_mode = param[6];
				PHY_MSG.phy_sfbc_en         = param[7];
				PHY_MSG.phy_cdd_num         = param[8];
				
				//				sscanf(nodemessage, "phymsg %d %d %d %d %d %d", &PHY_MSG.rf_agc_framelock_en,
				//						&PHY_MSG.phy_cfo_bypass_en,&PHY_MSG.phy_pre_STS_thresh,
				//						&PHY_MSG.phy_pre_LTS_thresh,&PHY_MSG.phy_tx_iq0_scale,
				//						&PHY_MSG.phy_tx_iq1_scale);
				printf("set ------- phymsg = %d %d %d %d %d %d %d %d %d\n", PHY_MSG.rf_agc_framelock_en, PHY_MSG.phy_cfo_bypass_en,
					PHY_MSG.phy_pre_STS_thresh, PHY_MSG.phy_pre_LTS_thresh,
					PHY_MSG.phy_tx_iq0_scale, PHY_MSG.phy_tx_iq1_scale,
					PHY_MSG.phy_msc_length_mode,PHY_MSG.phy_sfbc_en,
					PHY_MSG.phy_cdd_num);


#else
				sscanf(nodemessage, "phymsg %d %d %d %d %d %d", &param[0],
					&param[1], &param[2],
					&param[3], &param[4],
					&param[5]);
			    PHY_MSG.rf_agc_framelock_en = param[0];
			    PHY_MSG.phy_cfo_bypass_en = param[1];
			    PHY_MSG.phy_pre_STS_thresh = param[2];
			    PHY_MSG.phy_pre_LTS_thresh = param[3];
			    PHY_MSG.phy_tx_iq0_scale = param[4];
			    PHY_MSG.phy_tx_iq1_scale = param[5];
			//				sscanf(nodemessage, "phymsg %d %d %d %d %d %d", &PHY_MSG.rf_agc_framelock_en,
			//						&PHY_MSG.phy_cfo_bypass_en,&PHY_MSG.phy_pre_STS_thresh,
			//						&PHY_MSG.phy_pre_LTS_thresh,&PHY_MSG.phy_tx_iq0_scale,
			//						&PHY_MSG.phy_tx_iq1_scale);
			    printf("set ------- phymsg = %d %d %d %d %d %d\n", PHY_MSG.rf_agc_framelock_en, PHY_MSG.phy_cfo_bypass_en,
				      PHY_MSG.phy_pre_STS_thresh, PHY_MSG.phy_pre_LTS_thresh,
				      PHY_MSG.phy_tx_iq0_scale, PHY_MSG.phy_tx_iq1_scale);

#endif

			}
#ifdef Radio_CEC
			
			if (strcmp(messagename, "ncunodeid") == 0) {
				sscanf(nodemessage, "ncunodeid %d", &param[0]);
				NCU_NODE_ID_INIT = param[0];
				printf("set ------- ncu_node_id = %d\n", NCU_NODE_ID_INIT);
			}


#endif
          	if (strcmp(messagename, "ip_addr") == 0) {
				sscanf(nodemessage, "ip_addr %d.%d.%d.%d", &SELFIP_s[0],&SELFIP_s[1],&SELFIP_s[2],&SELFIP_s[3]);
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,
						"ifconfig br0 %d.%d.%d.%d",
						SELFIP_s[0],SELFIP_s[1],SELFIP_s[2],SELFIP_s[3]);
				system(cmd);
				
				printf("set ------- br0 ip address = %d.%d.%d.%d\n", SELFIP_s[0],SELFIP_s[1],SELFIP_s[2],SELFIP_s[3]);

				
			}
			//add by yang
			/*if (strcmp(messagename, "longitude") == 0) {
				sscanf(nodemessage, "longitude %lf", &longitude);
				printf("set ------- longitude = %lf\n", longitude);
			}
			if (strcmp(messagename, "latitude") == 0) {
				sscanf(nodemessage, "latitude %lf", &latitude);
				printf("set ------- latitude = %lf\n", latitude);
			}*/
          	if (strcmp(messagename, "slotlen") == 0) {
				sscanf(nodemessage, "slotlen %d", &g_u8SLOTLEN);
				printf("set ------- slotlen = %d\n", g_u8SLOTLEN);	
			}
			//add by yang
			/*if (strcmp(messagename, "longitude") == 0) {
				sscanf(nodemessage, "longitude %lf", &longitude);
				printf("set ------- longitude = %lf\n", longitude);
			}
			if (strcmp(messagename, "latitude") == 0) {
				sscanf(nodemessage, "latitude %lf", &latitude);
				printf("set ------- latitude = %lf\n", latitude);
			}*/
		}

		fclose(file);
	}
	row_cnt = 0;

	if ((file_hop = fopen("/etc/node_hop", "r")) != NULL)
	 {
		while (fgets(data_hop, sizeof(data_hop), file_hop) != NULL) {
				
				sscanf(data_hop, "%d %d %d %d %d %d %d %d", &param[0],
					&param[1], &param[2],
					&param[3], &param[4],
					&param[5], &param[6],
					&param[7]);
				for(i=0;i<8;i++)
				{
					HOP_FREQ_TB_INIT[row_cnt*8+i] = param[i];
				}
				row_cnt++;		
				bzero(data_hop, sizeof(data_hop));
				// printf("set ------- hop = %d %d %d %d %d %d %d %d\n", param[0], param[1],
				// 													  param[2], param[3],
				// 													  param[4], param[5],
				// 												      param[6], param[7]);
				
			
		}
		fclose(file_hop);
	}
	printf("set ------- hop freq table= ");
	for(i=0;i<HOP_FREQ_NUM;i++)
	{
		printf(" %d", HOP_FREQ_TB_INIT[i]);
	}
	printf("\n");
	SELFID = id;
	g_radio_param.device_id=SELFID;
	SOCKET_MGMT = CreateUDPServer(MGMT_PORT);
	if (SOCKET_MGMT <= 0)
	{
		printf("SOCKET_MGMT create error\n");
		exit(1);
	}

	//ret = setsockopt(SOCKET_MGMT, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
	//if (ret < 0) {
	//	perror("setsockopt(SO_BROADCAST) failed");
	//	return -1;
	//}

	S_GROUND_STD.sin_family = AF_INET;
	S_GROUND_STD.sin_addr.s_addr = inet_addr("192.168.2.1");
	S_GROUND_STD.sin_port = htons(MGMT_PORT);

	S_GROUND_WG.sin_family = AF_INET;
	S_GROUND_WG.sin_addr.s_addr = inet_addr("255.255.255.255");
	S_GROUND_WG.sin_port = htons(WG_TX_UDP_PORT);

	S_OTHER_NODE.sin_family = AF_INET;
	S_OTHER_NODE.sin_addr.s_addr = inet_addr("192.168.255.255");
	S_OTHER_NODE.sin_port = htons(WG_RX_UDP_PORT);

	SOCKET_GROUND = CreateUDPServer(CTRL_PORT);
	if (SOCKET_GROUND <= 0) {
		printf("SOCKET_GROUND create error\n");
		exit(1);
	}

	SOCKET_BCAST_RECV=CreateUDPServer(BCAST_RECV_PORT);
	if (SOCKET_BCAST_RECV <= 0)
	{
		printf("SOCKET_BCAST_RECV create error\n");
		exit(1);
	}
	if (setsockopt(SOCKET_BCAST_RECV, SOL_SOCKET, SO_BROADCAST, ifname, 4) < 0) {
		printf("SOCKET_BCAST_RECV bindtodevice error\n");
		exit(1);
	}

	SOCKET_MEM_REQUEST_RECV=CreateUDPServer(MEM_REQUEST_PORT);
	if (SOCKET_MEM_REQUEST_RECV <= 0)
	{
		printf("SOCKET_MEM_REQUEST_RECV create error\n");
		exit(1);
	}	
	// SOCKET_AUDIO_RECV=CreateUDPServer(AUDIO_BROADCAST_PORT);
	// if (SOCKET_AUDIO_RECV <= 0)
	// {
	// 	printf("SOCKET_AUDIO_RECV create error\n");
	// 	exit(1);
	// }
	// if (setsockopt(SOCKET_AUDIO_RECV, SOL_SOCKET, SO_BROADCAST, ifname, 4) < 0) {
	// 	printf("SOCKET_AUDIO_RECV bindtodevice error\n");
	// 	exit(1);
	// }

	//ret = sendto(SOCKET_MGMT, messagename, 100, 0, &S_GROUND_WG, sizeof(struct sockaddr_in));
	//if (ret < 0) {
	//	perror("sendto() failed");
	//	return -1;
	//}


	SOCKET_UDP_WG = CreateUDPServer(WG_RX_UDP_PORT);
	//测试打印：
	printf("创建SOCKET_UDP_WG的socket，端口为WG_RX_UDP_PORT=%d\n", WG_RX_UDP_PORT);

	if (setsockopt(SOCKET_UDP_WG, SOL_SOCKET, SO_BINDTODEVICE, ifname, 4) < 0) {
		printf("SOCKET_UDP_WG bindtodevice error\n");
		exit(1);
	}
	if (setsockopt(SOCKET_UDP_WG, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
		printf("SOCKET_UDP_WG BROADCAST error\n");
		exit(1);
	}

	SOCKET_TCP_WG = CreateTCPClient(WG_RX_TCP_PORT);
	printf("SOCKET_TCP_WG = %d\n", SOCKET_TCP_WG);
	for (int i = 0; i < CONNECTNUM; i++) {
		TCPCLIENT_WG[i].useful = FALSE;
		TCPCLIENT_WG[i].sockfd = 0;
		TCPCLIENT_WG[i].srcip = 0;
		TCPCLIENT_WG[i].time.tv_sec = 0;
		TCPCLIENT_WG[i].time.tv_usec = 0;
	}
	TCPCLIENTCOND_WG = CreateEvent();
	TCPCLIENTMUTEX_WG = CreateLock();


	S_GROUND_PC.sin_family = AF_INET;
	S_GROUND_PC.sin_addr.s_addr = inet_addr("192.168.2.100");
	S_GROUND_PC.sin_port = htons(GROUND_PORT);
	S_GROUND = S_GROUND_PC;
	ISLOGIN = FALSE;
	/*
	//modify by yang 注释掉下面一段，因为无需使用web
		MSG_MGMT = msgget(MSGKEY,IPC_CREAT);
		if(MSG_MGMT < 0)
		{
			printf("MSG_MGMT create error\n");
			exit(1);
		}
	//
	*/

#ifdef Radio_CEC

	SOCKET_UDP_BEAM = CreateUDPServer(BEAM_TX_UDP_PORT);
	if (SOCKET_UDP_BEAM <= 0)
	{
		printf("SOCKET_UDP_BEAM create error\n");
		exit(1);
	}
	BEAM_RX_add.sin_family = AF_INET;
	BEAM_RX_add.sin_addr.s_addr = inet_addr("192.168.2.202");
	BEAM_RX_add.sin_port = htons(BEAM_RX_UDP_PORT);
	
	printf("创建波位预报的socket =%d ，接收端口为=%d\n", SOCKET_UDP_BEAM,BEAM_RX_UDP_PORT);
	
	frame_cnt = 0;
	pps_cnt   = 0;
	Neigbor_direction[0] = 1;
	Neigbor_direction[1] = 16;
	Neigbor_direction[2] = 31;
	Neigbor_direction[3] = 46;


#endif
	
	mgmt_param_init();
	printf("init 1\n", SOCKET_TCP_WG);
}



//static int mgmt_mysql_executesql(const char * sql,MYSQL* g_conn)
//{
//	if(mysql_real_query(g_conn,sql,strlen(sql))){
//		printf("executesql(): %s\n", mysql_error(g_conn));
//		return -1;
//	}
//	return 0;
//}
//
//static int mgmt_mysql_create_table(void)
//{
//	MYSQL           mysql;
//	MYSQL_RES       *res = NULL;
//	char            sql[1000];
//	int iNum_rows;
//	int ret;
//	if (NULL == mysql_init(&mysql)) {
//		printf("mysql_init(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
//	if (NULL == mysql_real_connect(&mysql,
//				"192.168.232.128",
//				"root",
//				"123456",
//				"mysql",
//				0,
//				NULL,
//				0)) {
//		printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
//	printf("1. Connected MySQL successful! \n");
//	sprintf(sql,"show tables;");
//	ret = mgmt_mysql_executesql(sql,&mysql);
//	res = mysql_store_result(&mysql);
//	iNum_rows = mysql_num_rows(res);
//	ret = mgmt_mysql_executesql("create table nodemsg(id int(32) unsigned primary key,nodeid int(4) unsigned not null,"
//				"ip int(32) unsigned not null,mac char(20) not null,txrate int(32) unsigned not null,"
//				"rxrate int(32) unsigned not null,freq int(32) unsigned not null,bw int(16) unsigned not null,"
//				"txpower int(16) not null);",&mysql);
//	mysql_free_result(res);//释放结果集
//	return iNum_rows;
//}
//
//static int mgmt_mysql_con(Smgmt_transmit_info* mgmt_info)
//{
//	MYSQL           mysql;
//	MYSQL_RES       *res = NULL;
//	char            sqldata[1024];
//	int             sqldatalen;
//	int             rc, i, fields;
//	if (NULL == mysql_init(&mysql)) {
//		printf("mysql_init(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
//	if (NULL == mysql_real_connect(&mysql,
//				"192.168.2.200",
//				"root",
//				"123456",
//				"mysql",
//				0,
//				NULL,
//				0)) {
//		printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
//	printf("1. Connected MySQL successful! \n");
//
//	memset(mgmt_info->macaddr,0xff,6);
//
////        mgmt_mysql_create_table(sql,&mysql,res);
////执行插入请求
////	query_str = "insert into nodemsg values (0,1,0xc0a80001,'b88edf000001',1000,500,2450,10,-10)";
//	sqldatalen = sprintf(sqldata,"insert into nodemsg values (%d,%d,%d,'%s',%d,%d,%d,%d,%d);",
//			mysql_num,(mgmt_info->id),(mgmt_info->ip),mgmt_info->macaddr,(mgmt_info->txrate),
//			(mgmt_info->rxrate),(mgmt_info->freq),(mgmt_info->bw),(mgmt_info->txpower));
////	rc = mysql_real_query(&mysql, query_str, strlen(query_str));
//
//	rc = mysql_real_query(&mysql, sqldata, sqldatalen);
//	if (0 != rc) {
//		printf("mysql_real_query(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
////    //执行删除请求
////        query_str = "delete from tb_users where userid=10006";
////        rc = mysql_real_query(&mysql, query_str, strlen(query_str));
////        if (0 != rc) {
////            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
////            return -1;
////        }
////    //然后查询插入删除之后的数据
////        query_str = "select * from tb_users";
////        rc = mysql_real_query(&mysql, query_str, strlen(query_str));
////        if (0 != rc) {
////            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
////            return -1;
////        }
////        res = mysql_store_result(&mysql);
////        if (NULL == res) {
////             printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
////             return -1;
////        }
////        rows = mysql_num_rows(res);
////        printf("The total rows is: %d\n", rows);
////        fields = mysql_num_fields(res);
////        printf("The total fields is: %d\n", fields);
////        while ((row = mysql_fetch_row(res))) {
////            for (i = 0; i < fields; i++) {
////                printf("%s\t", row[i]);
////            }
////            printf("\n");
////        }
//	mysql_free_result(res);
//	mysql_close(&mysql);
//	mysql_num ++;
//	return 0;
//}
//
//static int mgmt_mysql_write(int nodeid,char* sqldata,int sqldatalen){
//	MYSQL           mysql;
//	MYSQL_RES       *res = NULL;
//	MYSQL_ROW       row;
//	char            sqlname[1024];
//	int             sqlnamelen;
//	char            sqld[2048*10];
//	int             sqldlen;
//	char            *query_str = NULL;
//	int             rc, i, fields;
//	int             rows;
//	char*           sql;
//	struct mgmt_msg temp_msg;
//	struct mgmt_header* hmgmt = (struct mgmt_header*)sqldata;
//	struct mgmt_msg* mmsg;
//	char ip[20] = "192.168.2.2";
//	char mac[20] = "b8:8e:df:00:01:03";
//	//printf("mgmt_mysql_write\n");
//
//	bzero(&temp_msg,sizeof(struct mgmt_msg));
//	if (NULL == mysql_init(&mysql)) {
//		printf("mysql_init(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
//	if (NULL == mysql_real_connect(&mysql,
//				"192.168.2.200",
//				"root",
//				"123456",
//				"mysql",
//				0,
//				NULL,
//				0)) {
//		printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
//		return -1;
//	}
////	printf("1. Connected MySQL successful! \n");
//
//	sqlnamelen = sprintf(sqlname,"create table IF NOT EXISTS node_%d(id int(32) unsigned NOT NULL AUTO_INCREMENT primary key,nodeid int(4) unsigned not null,"
//				"ip char(20) not null,mac char(20) not null,"
//				"freq int(4) unsigned not null,bw int(4) unsigned not null,"
//				"txpower int(4) not null,seqno int(4) unsigned not null,neigh_num int(4) unsigned not null,"
//				"neigh_id int(4) unsigned not null,mcs int(4) unsigned not null,snr int(4) unsigned not null,"
//				"ucds int(1) unsigned not null,enqueue_bytes int(4) unsigned not null,outqueue_bytes int(4) unsigned not null);",nodeid);
//
////	printf("sqlname %s %d\n",sqlname,sqlnamelen);
//	if(mysql_real_query(&mysql,sqlname,sqlnamelen)){
//		printf("sqlname: %s\n", mysql_error(&mysql));
//	}
//
//
////	hmgmt->node_id = 1;
////	hmgmt->freq = 1495000000;
////	hmgmt->bw = 10;
////	hmgmt->txpower = 20;
////	hmgmt->neigh_num = 5;
////	hmgmt->seqno = 1;
////	mmsg->node_id = 2;
////	mmsg->mcs = 4;
////	mmsg->snr = 34;
////	mmsg->ucds = 1;
////	mmsg->enqueue_bytes = 2044444;
////	mmsg->outqueue_bytes = 2434333;
//
//	if(hmgmt->neigh_num == 0)
//		mmsg = &temp_msg;
//	else{
//		mmsg = (struct mgmt_msg*)(sqldata+sizeof(struct mgmt_header));
//	}
//	sqldlen = sprintf(sqld,"insert into node_%d(nodeid,ip,mac,freq,bw,txpower,seqno,neigh_num,neigh_id,mcs,snr,ucds,enqueue_bytes,outqueue_bytes)"
//			"values (%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",nodeid,(int)hmgmt->node_id,
//			ip,mac,hmgmt->freq,hmgmt->bw,hmgmt->txpower,hmgmt->seqno,hmgmt->neigh_num,mmsg->node_id,mmsg->mcs,mmsg->snr
//			,mmsg->ucds,mmsg->enqueue_bytes,mmsg->outqueue_bytes);
//
//	for(i = 1; i < hmgmt->neigh_num ; i ++){
//		mmsg = mmsg+1;
//		sqldlen = sprintf(sqld,"%s,(%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",sqld,(int)hmgmt->node_id,
//				ip,mac,hmgmt->freq,hmgmt->bw,hmgmt->txpower,hmgmt->seqno,hmgmt->neigh_num,mmsg->node_id,mmsg->mcs,mmsg->snr
//				,mmsg->ucds,mmsg->enqueue_bytes,mmsg->outqueue_bytes);
//	}
//
//	sqldlen = sprintf(sqld,"%s;",sqld);
//
////	printf("sqldata %s %d\n",sqld,sqldlen);
//	if(mysql_real_query(&mysql,sqld,sqldlen)){
//		printf("sqldata: %s\n", mysql_error(&mysql));
//	}
//
//	//mysql_real_query(&mysql,sqldata,sqldatalen);
//	mysql_free_result(res);
//	mysql_close(&mysql);
//	return 0;
//}
//add by yang
double htond(double val) {
	double dval = val;
	uint64_t tmp = 0;
	memcpy(&tmp, &dval, sizeof(dval));
	tmp = htobe64(tmp);
	memcpy(&dval, &tmp, sizeof(tmp));
	return dval;
}
void mgmt_recv_web(void)
{
	INT8 buffer[BUFLEN];
	INT32 buflen = 0;
	Smgmt_header* hmsg;
	struct mgmt_header* hmgmt;
	Smgmt_set_param* sparam;
	while (TRUE)
	{
		buflen = msgrcv(MSG_MGMT, buffer, BUFLEN, MSG_TYPE, 0);

		//		printf("MSG_MGMT getdata\n");

		if (buflen < sizeof(Smgmt_header))
			continue;
		hmsg = (Smgmt_header*)(buffer + sizeof(long));

		if (ntohs(hmsg->mgmt_head) != HEAD) {
			//			printf("1\n");
			continue;
		}
		switch (ntohs(hmsg->mgmt_type)) {
		case MGMT_LOGIN: {
			//			S_GROUND = from;
			ISLOGIN = TRUE;
			//			printf("login\n");
			break;
		}
		default: {
			//			printf("2\n");
			sparam = (Smgmt_set_param*)hmsg->mgmt_data;
			//			printf("%d %d %d %d\n",ntohl(sparam->mgmt_mac_freq),ntohs(sparam->mgmt_mac_txpower),sparam->mgmt_virt_unicast_mcs,sparam->mgmt_mac_bw);
			mgmt_netlink_set_param(buffer + sizeof(long), buflen - sizeof(long), NULL);
			break;
		}
		}
	}
}

void mgmt_status_reply(struct sockaddr_in ipaddr)
{
	INT8 buf[BUFLEN];
	INT32 len = 0;
	Smgmt_header* hmsg;
	Snodefind* snodefind;
	hmsg = (Smgmt_header*)buf;
	snodefind = (Snodefind*)hmsg->mgmt_data;
	hmsg->mgmt_head = htons(HEAD);
	hmsg->mgmt_type = htons(MGMT_NODEFIND);
	hmsg->mgmt_keep = 0;
	////
	SendUDPClient(SOCKET_GROUND, buf, len, &ipaddr);
}

void mgmt_set_msg(struct sockaddr_in ipaddr)
{
	INT8 buf[BUFLEN];
	INT32 len = 0;
	Smgmt_header* hmsg;
	Snodefind* snodefind;
	hmsg = (Smgmt_header*)buf;
	snodefind = (Snodefind*)hmsg->mgmt_data;
	hmsg->mgmt_head = htons(HEAD);
	hmsg->mgmt_type = htons(MGMT_NODEFIND);
	hmsg->mgmt_keep = 0;
	////
	SendUDPClient(SOCKET_GROUND, buf, len, &ipaddr);
}

void mgmt_set_reply() {
	Smgmt_header msgreply;
	msgreply.mgmt_head = htons(HEAD);
	msgreply.mgmt_type = htons(MGMT_NODEFIND); /*字节序转换*/
//	msgreply.mgmt_keep = htons(type);
//	msgreply.mgmt_len = htons(sizeof(msg));
//	SendTCPServer(SOCKET_GROUND, (INT8*) &msgreply, ntohs(msgreply.len));
}

void mgmt_recv_msg(void) {
	INT32 GRet = -1, i;
	INT32 maxfdp = 0;
	fd_set fds;
bool isset=FALSE;
	struct timeval TimeOut = { 1, 0 };  // 减少超时时间从20秒到1秒
	struct timeval timenow;
	INT8 buffer[BUFLEN];
	INT32 buflen = 0;
	struct sockaddr_in from;
	Smgmt_header* hmsg;
	struct mgmt_header* hmgmt;
	INT32 socklen = sizeof(struct sockaddr_in);
	int param = 0;
	int* ipdaddr;
	INT32 sock = 0;
	int ret = -1;
	Smgmt_header tcpmsg;
	INT8 filename[100];
	FILE* filefd = NULL;
	INT8 cmd[200];
	char selfAddr[4] = {0xc0,0xa8,0x02,0x01};
	uint32_t selfip;
	int sysinfo_bw;

	INT8 set_buf[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];
	INT32 set_len = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);
	Smgmt_header* mhead = (Smgmt_header*)set_buf;
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;
	bzero(set_buf, set_len);
	memset(cmd,0,sizeof(cmd));
	mhead->mgmt_head = htons(HEAD);
	mhead->mgmt_len = sizeof(Smgmt_set_param);
	mhead->mgmt_type = 0;

	bcMeshInfo *meshinfo_recv=NULL;
	MEM_REQUEST_FRAME *info=NULL;

	//测试打印：
	printf("调用mgmt_recv_msg接收函数\n");

	selfAddr[3]=SELFID;
	printf("seifid:%d\r\n",selfAddr[3]);
	memcpy(&selfip,selfAddr,sizeof(uint32_t));
	struct in_addr selfIP;
	selfIP.s_addr = selfip;
	
	stInData bc_systeminfoupdate;
	memset((char*)&bc_systeminfoupdate,0,sizeof(bc_systeminfoupdate));

	// 添加性能优化变量
	static int select_count = 0;
	static struct timeval last_select_time = {0, 0};
	struct timeval current_time;

	while (TRUE) {
		sleep(1);
		GRet = -1;
		bzero(buffer, sizeof(buffer));
		
		// 优化：减少select调用频率，添加微秒级休眠
		while (GRet <= 0) {
			FD_ZERO(&fds);
			maxfdp = 0;
			FD_SET(SOCKET_MGMT, &fds);
			maxfdp = SOCKET_MGMT;
			FD_SET(SOCKET_GROUND, &fds);
			if (maxfdp < SOCKET_GROUND)
				maxfdp = SOCKET_GROUND;
			FD_SET(SOCKET_UDP_WG, &fds);
			if (maxfdp < SOCKET_UDP_WG)
				maxfdp = SOCKET_UDP_WG;
			FD_SET(SOCKET_TCP_WG, &fds);
			if (maxfdp < SOCKET_TCP_WG)
				maxfdp = SOCKET_TCP_WG;
			FD_SET(SOCKET_BCAST_RECV,&fds);   //add by sdg
			if (maxfdp < SOCKET_BCAST_RECV)
				maxfdp = SOCKET_BCAST_RECV;
			FD_SET(SOCKET_MEM_REQUEST_RECV,&fds);   //add by sdg
			if (maxfdp < SOCKET_MEM_REQUEST_RECV)
				maxfdp = SOCKET_MEM_REQUEST_RECV;
			for (i = 0; i < CONNECTNUM; i++) {
				if (TCPCLIENT_WG[i].useful == TRUE) {
					FD_SET(TCPCLIENT_WG[i].sockfd, &fds);
					if (maxfdp < TCPCLIENT_WG[i].sockfd)
						maxfdp = TCPCLIENT_WG[i].sockfd;
				}
			}

			TimeOut.tv_sec = 1;  // 减少超时时间
			TimeOut.tv_usec = 500000;  // 增加500ms微秒级超时，减少CPU轮询
			GRet = select(maxfdp + 1, &fds, NULL, NULL, &TimeOut);
			
			// 如果没有事件，添加短暂休眠减少CPU占用
			if (GRet <= 0) {
				usleep(5000);  // 增加到5ms休眠，进一步降低CPU占用
			}
		}
		//测试打印：
		//printf("监听到%d个socket变化\n",GRet);
		// if(FD_ISSET(SOCKET_AUDIO_RECV, &fds))
		// {
		// 	buflen=	RecvUDPClient(SOCKET_AUDIO_RECV, buffer, BUFLEN, &from, &socklen);
		// 	if(selfip!=inet_addr(inet_ntoa(from.sin_addr))&&(buflen>0))
		// 	{
		// 		//printf("[AUDIO DEBUG] recv audio socket from %s  \r\n",inet_ntoa(from.sin_addr));
				
		// 		//
		// 		write_audio_info((char*)buffer,buflen);
		// 		stat_info.audio_rx_packets++;
		// 	}

		// }

		if(FD_ISSET(SOCKET_MEM_REQUEST_RECV, &fds))
		{
			buflen=	RecvUDPClient(SOCKET_MEM_REQUEST_RECV, buffer, BUFLEN, &from, &socklen);
			if(buflen>0)
			{
				if(selfip==inet_addr(inet_ntoa(from.sin_addr)))
				{
					continue;
				}
				process_member_info(buffer,buflen);
				

			}
		}

		if(FD_ISSET(SOCKET_BCAST_RECV, &fds))
		{
			//printf("SOCKET_BCAST_RECV SET\r\n");
			buflen=	RecvUDPClient(SOCKET_BCAST_RECV, buffer, BUFLEN, &from, &socklen);
			
			//printf("selfip %s fromIp:%s compare result %d\r\n",inet_ntoa(selfIP),inet_ntoa(from.sin_addr),strcmp((char*)inet_ntoa(selfIP),(char*)inet_ntoa(from.sin_addr)));
			if(buflen>0)
			{
				//if(strcmp(selfAddr,inet_addr(inet_ntoa(from.sin_addr)))!=0)
				if(selfip!=inet_addr(inet_ntoa(from.sin_addr)))
				{
					// printf("收到组播包 接收源IP:%s  \r\n", inet_ntoa(from.sin_addr));
					meshinfo_recv=(bcMeshInfo*)buffer;
					bzero(set_buf, set_len);
					memset(cmd,0,sizeof(cmd));
					mhead->mgmt_head = htons(HEAD);
					mhead->mgmt_len = sizeof(Smgmt_set_param);
					mhead->mgmt_type = 0;
					if(meshinfo_recv->txpower_isset)
					{
						// printf("receive bcast packet,set txpower\r\n");
						isset=TRUE;
						meshinfo_recv->txpower_isset=0;

						bool has_per_channel = false;
						for (int idx = 0; idx < POWER_CHANNEL_NUM; ++idx) {
							if (meshinfo_recv->m_txpower_ch[idx] != 0) {
								has_per_channel = true;
								break;
							}
						}
						if (!has_per_channel) {
							txpower_lookup_channels_be(ntohs(meshinfo_recv->m_txpower), meshinfo_recv->m_txpower_ch);
						}

						mhead->mgmt_type |= MGMT_SET_POWER;
						//mhead->mgmt_type = htons(mhead->mgmt_type);
						mparam->mgmt_mac_txpower=meshinfo_recv->m_txpower;
						memcpy(mparam->mgmt_mac_txpower_ch, meshinfo_recv->m_txpower_ch, sizeof(meshinfo_recv->m_txpower_ch));
						//mgmt_netlink_set_param(set_buf, set_len,NULL);
//接收到组播包后，更新systeminfo库
						memset((char*)&bc_systeminfoupdate,0,sizeof(bc_systeminfoupdate));
						sprintf(bc_systeminfoupdate.name,"%s","m_txpower");
						sprintf(bc_systeminfoupdate.value,"%d",39-meshinfo_recv->sys_power);
						bc_systeminfoupdate.state[0] = '1';
						updateData_systeminfo(bc_systeminfoupdate);

						
					}
					if(meshinfo_recv->freq_isset)
					{
						// printf("receive bcast packet,set freq\r\n");
						isset=TRUE;
						meshinfo_recv->freq_isset=0;
						mhead->mgmt_type |= MGMT_SET_FREQUENCY;
						//mhead->mgmt_type = htons(mhead->mgmt_type);
						mparam->mgmt_mac_freq=meshinfo_recv->rf_freq;
						//printf("set freq %d \r\n",mparam->mgmt_mac_freq);
						//mgmt_netlink_set_param(set_buf, set_len,NULL);

						memset((char*)&bc_systeminfoupdate,0,sizeof(bc_systeminfoupdate));
						sprintf(bc_systeminfoupdate.name,"%s","rf_freq");
						sprintf(bc_systeminfoupdate.value,"%d",meshinfo_recv->sys_freq);
						bc_systeminfoupdate.state[0] = '1';
						updateData_systeminfo(bc_systeminfoupdate);


					}
					if(meshinfo_recv->chanbw_isset)
					{
						printf("receive bcast packet,set chanbw\r\n");
						isset=TRUE;
						meshinfo_recv->chanbw_isset=0;
						mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
						//mhead->mgmt_type = htons(mhead->mgmt_type);
						mparam->mgmt_mac_bw=meshinfo_recv->m_chanbw;
						//printf("set bw %d \r\n",mparam->mgmt_mac_bw);
						//mgmt_netlink_set_param(set_buf, set_len,NULL);

						if(meshinfo_recv->sys_bw==0)
						{
							sysinfo_bw=20;
						}
						else if(meshinfo_recv->sys_bw==1)
						{
							sysinfo_bw=10;
						}
						else if(meshinfo_recv->sys_bw==2)
						{
							sysinfo_bw=5;
						}
						else;
						memset((char*)&bc_systeminfoupdate,0,sizeof(bc_systeminfoupdate));
						sprintf(bc_systeminfoupdate.name,"%s","m_chanbw");
						sprintf(bc_systeminfoupdate.value,"%d",sysinfo_bw);
						bc_systeminfoupdate.state[0] = '1';
						updateData_systeminfo(bc_systeminfoupdate);						
					}
					if(meshinfo_recv->rate_isset)
					{
						printf("receive bcast packet,set rate\r\n");
						isset=TRUE;
						meshinfo_recv->rate_isset=0;
						mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
						//mhead->mgmt_type = htons(mhead->mgmt_type);
						mparam->mgmt_virt_unicast_mcs=meshinfo_recv->m_rate;
						//printf("set rate %d \r\n",mparam->mgmt_virt_unicast_mcs);
						//mgmt_netlink_set_param(set_buf, set_len,NULL);
						memset((char*)&bc_systeminfoupdate,0,sizeof(bc_systeminfoupdate));
						sprintf(bc_systeminfoupdate.name,"%s","m_rate");
						sprintf(bc_systeminfoupdate.value,"%d",meshinfo_recv->sys_rate);
						bc_systeminfoupdate.state[0] = '1';
						updateData_systeminfo(bc_systeminfoupdate);						
						

					}
					if(isset)
					{
						isset=FALSE;
						mhead->mgmt_type = htons(mhead->mgmt_type);
						mgmt_netlink_set_param(set_buf, set_len,NULL);
						sleep(1);
						if (!persist_test_db()) {
							printf("[mgmt_transmit] persist test.db failed after broadcast packet\n");
						}
					}

				}		

			}
			else;


		}

		if (FD_ISSET(SOCKET_TCP_WG, &fds)) {
			//测试打印：
			//printf("监听到SOCKET_TCP_WG变化\n");
//			printf("SOCKETTCP connect!\n");
			sock = OnlineMonitor(SOCKET_TCP_WG, &from);
			if (sock > 0) {
				gettimeofday(&timenow, NULL);
				Lock(&TCPCLIENTMUTEX_WG, 0);

				for (i = 0; i < CONNECTNUM; i++) {
					if (TCPCLIENT_WG[i].useful == FALSE) {
						TCPCLIENT_WG[i].sockfd = sock;
						TCPCLIENT_WG[i].useful = TRUE;
						TCPCLIENT_WG[i].srcip = from.sin_addr.s_addr;
						TCPCLIENT_WG[i].time.tv_sec = timenow.tv_sec;
						TCPCLIENT_WG[i].time.tv_usec = timenow.tv_usec;
						break;
					}
				}
				Unlock(&TCPCLIENTMUTEX_WG);
			}
		}
		if (FD_ISSET(SOCKET_GROUND, &fds)) {
			//测试打印：
			//printf("监听到SOCKET_GROUND变化\n");
			buflen = RecvUDPClient(SOCKET_GROUND, buffer, BUFLEN, &from, &socklen);//from为什么没绑定也能用
			if (buflen < sizeof(Smgmt_header))
				continue;
			hmsg = (Smgmt_header*)buffer;

			if (ntohs(hmsg->mgmt_head) != HEAD) {
				continue;
			}
			switch (ntohs(hmsg->mgmt_type)) {
			case MGMT_LOGIN: {
				S_GROUND = from;
				ISLOGIN = TRUE;
				printf("login\n");
				break;
			}
			default: {
				mgmt_netlink_set_param(buffer, buflen, NULL);
				break;
			}
			}
			//			printf("recv msg\n");
		}
		if (FD_ISSET(SOCKET_UDP_WG, &fds)) {
			//测试打印： 
			//printf("监听到SOCKET_UDP_WG变化，端口是7600\n");
			buflen = RecvUDPClient(SOCKET_UDP_WG, buffer, BUFLEN, &from, &socklen);
			//测试打印：
			//printf("接收源IP:%s  接收源端口:%d\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));
			if (buflen < sizeof(Smgmt_header))
				continue;
			hmsg = (Smgmt_header*)buffer;

			if (ntohs(hmsg->mgmt_head) != HEAD) {
				continue;
			}
			//测试打印：
			//printf("监听到SOCKET_UDP_WG变化，端口是7600\n");
			switch (ntohs(hmsg->mgmt_type)) {
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：%hu\n", hmsg->mgmt_type);
			case MGMT_DEVINFO:
			{
				//测试打印：------------------------
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_DEVINFO %x\n", MGMT_DEVINFO);
				ipdaddr = (int*)(hmsg->mgmt_data);
				//打印selfip
				struct in_addr selfaddr;
				selfaddr.s_addr = (uint32_t)SELFIP;
				//printf("selfip:%s\n", inet_ntoa(selfaddr));
				//打印ipdaddr
				//printf("hmsg->mgmt_data接收到的数据内容中IP为：");
				struct in_addr recvaddr;
				//recvaddr.s_addr = htonl((uint32_t)*ipdaddr);
				recvaddr.s_addr = (uint32_t)*ipdaddr;
				//printf("%s\n", inet_ntoa(recvaddr));
				//测试打印--------------------------
				if (SELFIP == *ipdaddr)
				{
					//测试打印
					//printf("调用mgmt_status_report(from)函数\n");
					//printf("收到状态查询包hmsg->mgmt_keep ：%u\n", hmsg->mgmt_keep);
					//keep == 0为网关节点，1为其他邻居
					if (hmsg->mgmt_keep == 0) {
						is_conned = 1;
					}
					//网关节点接收到邻居状态包，发往网管(没用，不是在转发)
					/*if (is_conned == 1 && hmsg->mgmt_keep != 0) {
						printf("-------------------------网关转发邻居状态包\n-------------------------------------------");
						mgmt_status_report(wg_addr);
						break;
					}*/
					wg_addr = from;
					//printf("接收到状态请求时的端口：%u", wg_addr.sin_port);
					//memcpy(&wg_addr, &from, sizeof(from));					
					mgmt_status_report(from);
				}
				else {
					//测试打印
					//printf("调用SendUDPClient发送至S_OTHER_NODE\n");
					hmsg->mgmt_keep = 1;
					//printf("是否将hmsg->mgmt_keep置1？ ：%u\n", hmsg->mgmt_keep);
					S_OTHER_NODE.sin_addr.s_addr = *ipdaddr;
					SendUDPClient(SOCKET_UDP_WG, buffer, buflen, &S_OTHER_NODE);
				}
				break;
			}
			case MGMT_DEVINFO_REPORT:
			{
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_DEVINFO_REPORT %x\n", MGMT_DEVINFO_REPORT);
				//ground
				SendUDPClient(SOCKET_UDP_WG, buffer, buflen, &wg_addr);
				break;
			}
			case MGMT_SET_PARAM:
			{
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_SET_PARAM %x\n", MGMT_SET_PARAM);
				ipdaddr = (int*)(hmsg->mgmt_data);
				if (SELFIP == *ipdaddr)
				{
					mgmt_netlink_set_param_wg(hmsg->mgmt_data + sizeof(int), ntohs(hmsg->mgmt_len) - sizeof(int), NULL,MGMT_SET_PARAM);
				}
				else {
					S_OTHER_NODE.sin_addr.s_addr = *ipdaddr;
					SendUDPClient(SOCKET_UDP_WG, buffer, buflen, &S_OTHER_NODE);
				}
				break;
			}
			case MGMT_SPECTRUM_QUERY:
			{
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_SPECTRUM_QUERY %x\n", MGMT_SPECTRUM_QUERY);
				//频谱查询
				break;
			}
			case MGMT_POWEROFF:
			{
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_POWEROFF %x\n", MGMT_POWEROFF);
				system("poweroff");
				break;
			}
			case MGMT_RESTART:
			{
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：%x\n", MGMT_RESTART);
				system("reboot");
				break;
			}
			case MGMT_FACTORY_RESET:
			{
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_FACTORY_RESET %x\n", MGMT_FACTORY_RESET);
				system("sh /etc/init.sh");
				break;
			}
			case MGMT_MULTIPOINT_SET:
			{
				//测试打印：配置消息
				//printf("接收到的消息类型hmsg->mgmt_type为：MGMT_MULTIPOINT_SET %x\n", MGMT_MULTIPOINT_SET);
				uint16_t nodenum = ntohs(hmsg->mgmt_keep);
				for (i = 0; i < nodenum; i++)
				{
					ipdaddr = (int*)(hmsg->mgmt_data);
					//测试打印--------------------------------
					struct in_addr prtaddr;
					prtaddr.s_addr = (uint32_t)*ipdaddr;
					//printf("接收到的ipdaddr地址是:%s\n", inet_ntoa(prtaddr));
					prtaddr.s_addr = SELFIP;
					//printf("SELFIP地址是:%s\n", inet_ntoa(prtaddr));
					//测试打印-------------------------------
					if (SELFIP == *ipdaddr)
					{
						//printf("进入SELFIP == *ipdaddr\n");
						//printf("nodenum:%hd\n", nodenum);
						//printf("(hmsg->mgmt_len:%hd\n", ntohs(hmsg->mgmt_len));
						mgmt_netlink_set_param_wg(hmsg->mgmt_data + sizeof(int) * nodenum, ntohs(hmsg->mgmt_len) - sizeof(int) * nodenum, NULL,MGMT_MULTIPOINT_SET);
					}
					else {
						//printf("进入else的S_OTHER_NODE\n");
						S_OTHER_NODE.sin_addr.s_addr = *ipdaddr;
						SendUDPClient(SOCKET_UDP_WG, buffer, buflen, &S_OTHER_NODE);
					}
					ipdaddr++;
				}
				break;
			}
			//add by yang
			case MGMT_TOPOLOGY_REQUEST:
			{
				//测试打印：
				//printf("接收到拓扑请求：MGMT_TOPOLOGY_REQUEST %x\n", MGMT_TOPOLOGY_REQUEST);
				//邻居节点接收到拓扑请求，将收到请求置为1
				topology_request* trptr = (topology_request*)&hmsg->mgmt_data;
				//测试打印----------
				struct in_addr broaddr;
				broaddr.s_addr = htonl(trptr->srcIp);
				//printf("广播包携带IP：%s\n", inet_ntoa(broaddr));
				//测试打印----------
				if (htonl(trptr->srcIp) != SELFIP) {
					//printf("gotRequest 置为 1\n");
					gotRequest = 1;
				}

				memcpy(&gate_addr, &from, sizeof(from));
				//将拓扑信息目的IP替换成广播包中携带的网关IP
				gate_addr.sin_addr.s_addr = htonl(trptr->srcIp);
				gate_addr.sin_port = htons(WG_RX_UDP_PORT);//7600端口为什么用htons？而不是htonl
				break;
			}
			case MGMT_TOPOLOGY_INFO:
			{
				//测试打印：------------------------
				//printf("接收到邻居拓扑信息：MGMT_TOPOLOGY_INFO %x\n", MGMT_TOPOLOGY_INFO);
				ipdaddr = (int*)(hmsg->mgmt_data);
				//打印selfip
				struct in_addr selfaddr;
				selfaddr.s_addr = (uint32_t)SELFIP;
				//printf("selfip:%s\n", inet_ntoa(selfaddr));
				//打印ipdaddr
				//printf("hmsg->mgmt_data接收到的数据内容中IP为：\n");
				struct in_addr recvaddr;
				//recvaddr.s_addr = htonl((uint32_t)*ipdaddr);
				recvaddr.s_addr = (uint32_t)*ipdaddr;
				//printf("%s\n", inet_ntoa(recvaddr));
				//测试打印--------------------------
				if (SELFIP != *ipdaddr)
				{
					//测试打印
					//printf("转发拓扑信息包\n");
					//网关节点接收到邻居节点的拓扑信息，则转发到网管
					SendUDPClient(SOCKET_UDP_WG, buffer, buflen, &wg_addr);
				}
				break;
			}

			default: {
				//测试打印：
				//printf("接收到的消息类型hmsg->mgmt_type跳转到了default\n");
				break;
			}
			}
			//			printf("recv msg\n");
		}
		if (FD_ISSET(SOCKET_MGMT, &fds)) {
			//测试打印：
			//printf("监听到SOCKET_MGMT变化\n");
			buflen = RecvUDPClient(SOCKET_MGMT, buffer, BUFLEN, &from, &socklen);
			//printf("recv SOCKET_MGMT buflen %d\n",buflen);
			if (buflen <= 12)
				continue;
			hmgmt = (struct mgmt_header*)buffer;
			//			mgmt_mysql_write((int)hmgmt->node_id,buffer,buflen);
						//SendUDPClient(SOCKET_GROUND,buffer,buflen,&S_GROUND_PC);
			SendNodeMsg(buffer, buflen);
		}
		for (i = 0; i < CONNECTNUM; i++) {
			if (TCPCLIENT_WG[i].useful != TRUE) {
				continue;
			}
			else {
				if (FD_ISSET(TCPCLIENT_WG[i].sockfd, &fds)) {
					ret = RecvTCPServer(TCPCLIENT_WG[i].sockfd,
						(INT8*)&tcpmsg, sizeof(tcpmsg));
					if (ret != RETURN_OK) {
						Lock(&TCPCLIENTMUTEX_WG, 0);
						CloseTCPSocket(TCPCLIENT_WG[i].sockfd);
						TCPCLIENT_WG[i].useful = FALSE;
						TCPCLIENT_WG[i].sockfd = 0;
						TCPCLIENT_WG[i].srcip = 0;
						TCPCLIENT_WG[i].time.tv_sec = 0;
						TCPCLIENT_WG[i].time.tv_usec = 0;
						Unlock(&TCPCLIENTMUTEX_WG);
						continue;
					}
					if (ntohs(tcpmsg.mgmt_head) != HEAD) {
						//printf("tcp tcpmsg.head = %d\n", tcpmsg.mgmt_head);
						continue;
					}
					//printf("Recv_order tcprecv len %d\n", ntohs(tcpmsg.mgmt_len));
					bzero(buffer, sizeof(buffer));
					ret = RecvTCPServer(TCPCLIENT_WG[i].sockfd, buffer,
						ntohs(tcpmsg.mgmt_len));
					switch (ntohs(tcpmsg.mgmt_type)) {
					case MGMT_FIRMWARE_UPDATE: {
						switch (ntohs(tcpmsg.mgmt_keep)) {
						case MGMT_NAME: {
							sprintf(filename, "/root/%s", buffer);
							while ((filefd = fopen(filename, "wb")) == NULL) {
								usleep(10000);
							}
							break;
						}
						case MGMT_CONTENT: {
							if (filefd != NULL)
								fwrite(buffer, tcpmsg.mgmt_len, 1,
									filefd);
							break;
						}
						case MGMT_END: {
							if (filefd == NULL)
								break;
							fwrite(buffer, tcpmsg.mgmt_len, 1, filefd);
							fclose(filefd);
							bzero(cmd, sizeof(cmd));
							sprintf(cmd, "opkg install %s", filename);
							system(cmd);
							bzero(cmd, sizeof(cmd));
							sprintf(cmd, "rm %s", filename);
							system(cmd);
							bzero(cmd, sizeof(cmd));
							bzero(filename, sizeof(filename));

							break;
						}
						case MGMT_UPDATE_FIRMWARE: {
							ret = system(buffer);
							if (ret != -1) {
								bzero(cmd, sizeof(cmd));
								sprintf(cmd, "opkg remove %s", buffer);
								system(cmd);
							}
							break;
						}
						default:
							break;
						}
						break;
					}
					case MGMT_FILE_UPDATE: {
						switch (ntohs(tcpmsg.mgmt_keep)) {
						case MGMT_FILENAME: {
							sprintf(filename, "/root/%s", buffer);
							while ((filefd = fopen(filename, "wb")) == NULL) {
								usleep(10000);
							}
							break;
						}
						case MGMT_FILECONTENT: {
							if (filefd != NULL)
								fwrite(buffer, tcpmsg.mgmt_len, 1,
									filefd);
							break;
						}
						case MGMT_FILEEND: {
							if (filefd == NULL)
								break;
							fwrite(buffer, tcpmsg.mgmt_len, 1, filefd);
							fclose(filefd);
							bzero(cmd, sizeof(cmd));
							sprintf(cmd, "opkg install %s", filename);
							system(cmd);
							bzero(cmd, sizeof(cmd));
							sprintf(cmd, "rm %s", filename);
							system(cmd);
							bzero(cmd, sizeof(cmd));
							bzero(filename, sizeof(filename));

							break;
						}
						case MGMT_UPDATE_FILE: {
							ret = system(buffer);
							if (ret != -1) {
								bzero(cmd, sizeof(cmd));
								sprintf(cmd, "opkg remove %s", buffer);
								system(cmd);
							}
							break;
						}
						default:
							break;
						}
						break;
					}
					default:
						break;
					}
				}
			}
		}
		usleep(1000);
	}
}

void SendNodeMsg(void* data, int datalen) {
	int len = sizeof(Smgmt_header) + datalen;
	char buf[len];
	Smgmt_header* hmgmt = (Smgmt_header*)buf;
	memset(buf, 0, len);
	hmgmt->mgmt_head = htons(HEAD);
	hmgmt->mgmt_type = htons(NODEPARAMTYPE);
	hmgmt->mgmt_len = htons(datalen);
	memcpy(hmgmt->mgmt_data, data, datalen);

	//SendUDPClient(SOCKET_GROUND,buf,len,&S_GROUND);
//	printf("SendNodeMsg len %d\n",len);
}

uint16_t ipCksum(void* ip, int len) {
	uint16_t* buf = (uint16_t*)ip;
	uint32_t cksum = 0;

	while (len > 1)
	{
		cksum += *buf++;
		len -= sizeof(uint16_t);
	}

	if (len)
		cksum += *(uint16_t*)buf;

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (uint16_t)(~cksum);
}

//add by yang

void send_topo_request() {
	//printf("网关节点，发送拓扑请求\n");
	Smgmt_header* hmsg;
	topology_request* request;
	char buffer[1024];
	int broadcast = 1;
	int ret;

	hmsg = (Smgmt_header*)buffer;
	hmsg->mgmt_head = htons(HEAD);
	hmsg->mgmt_type = htons(MGMT_TOPOLOGY_REQUEST);
	hmsg->mgmt_keep = 0;
	hmsg->mgmt_len = sizeof(topology_request);
	char myIp[4] = { 0xc0,0xa8,0x02,0x01 };
	myIp[3] = SELFID;

	request = (topology_request*)hmsg->mgmt_data;
	request->srcIp = htonl(SELFIP);

	struct sockaddr_in toNeigh;
	toNeigh.sin_family = AF_INET;
	toNeigh.sin_addr.s_addr = INADDR_BROADCAST;
	//toNeigh.sin_addr.s_addr = inet_addr("192.168.255.255");
	toNeigh.sin_port = htons(WG_RX_UDP_PORT);//7600端口
	int len = sizeof(Smgmt_header) + sizeof(topology_request);
	// Enable broadcasting on socket
	ret = setsockopt(SOCKET_MGMT, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
	if (ret < 0) {
		perror("setsockopt(SO_BROADCAST) failed");
		return -1;
	}
	int sret = SendUDPBrocast(SOCKET_MGMT, buffer, len, &toNeigh);

	/*int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		perror("socket");
		return 1;
	}
	int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes)) < 0) {
		perror("setsockopt");
		return 1;
	}

	if (sendto(sock, buffer, len, 0, (struct sockaddr*)&toNeigh, sizeof(toNeigh)) < 0) {
		perror("sendto");
		return 1;
	}*/

	//printf("SendUDPBrocast发送返回值:%d\n", ret);//-1
	//printf("socket缓冲区长度:%d\n", len);//12

}

//add by yang
void send_topo_msg(struct sockaddr_in from, struct mgmt_send self_msg) {

	struct sockaddr_in towg = from;
	printf("send_topo_msg往网管：%s 发送拓扑数据包\n", inet_ntoa(towg.sin_addr));
	Smgmt_header* topo_header_ptr;
	struct topo_data* topomsgptr;
	struct neighbor_data* neighbormsgptr;
	char topobuff[2048];
	// 拓扑数据包包头->网关节点信息包->网关节点邻居的数据包
	topo_header_ptr = (Smgmt_header*)topobuff;


	//bzero(topo_header_ptr,sizeof(struct Smgmt_header));
	//bzero(topomsgptr, sizeof(struct topo_data));
	//bzero(topobuff, sizeof(topobuff));
	//1、拓扑数据包包头
	topo_header_ptr->mgmt_head = htons(HEAD);
	topo_header_ptr->mgmt_type = htons(MGMT_TOPOLOGY_INFO);
	topo_header_ptr->mgmt_keep = 0;

	//topomsgptr = topo_header_ptr->mgmt_data;//这个写法是不是有问题？
	topomsgptr = (topo_data*)topo_header_ptr->mgmt_data;

	//读取配置文件中的经纬度
	FILE* file_node;
	char nodemessage[100];
	char messagename[100];

	if ((file_node = fopen("/etc/node_xwg", "r")) == NULL) {
		longitude = 118.76;
		latitude = 32.04;

	}
	else {

		while (fgets(nodemessage, sizeof(nodemessage), file_node) != NULL) {
			bzero(messagename, sizeof(messagename));
			sscanf(nodemessage, "%s ", messagename);
			//add by yang
			if (strcmp(messagename, "longitude") == 0) {
				sscanf(nodemessage, "longitude %lf", &longitude);
				//printf("set ------- longitude = %lf\n", longitude);
			}
			if (strcmp(messagename, "latitude") == 0) {
				sscanf(nodemessage, "latitude %lf", &latitude);
				//printf("set ------- latitude = %lf\n", latitude);
			}
		}

	}


	fclose(file_node);
	//邻居数据包
	//printf("节点邻居数:%d ", self_msg.neigh_num);
	neighbormsgptr = (struct neighbor_data*)(topobuff + sizeof(Smgmt_header) + sizeof(topo_data));
	int topo_len = 0;

	//2、拓扑数据包数据内容1：网关节点信息包
	topomsgptr->selfip = SELFIP;
	topomsgptr->longitude = htond(longitude);
	topomsgptr->latitude = htond(latitude);
	topomsgptr->noise = 0;
	topomsgptr->tx_traffic = htonl(self_msg.tx);
	topomsgptr->rx_traffic = htonl(self_msg.rx);
	topomsgptr->neighbors_num = htonl(self_msg.neigh_num);
	if (self_msg.neigh_num == 0) {
		topo_len = sizeof(Smgmt_header) + sizeof(topo_data);
		topo_header_ptr->mgmt_len = htons(topo_len - sizeof(Smgmt_header));
	}
	else {
		//3、拓扑数据包数据内容2：邻居的数据包
		topo_len = sizeof(Smgmt_header) + sizeof(topo_data) + sizeof(neighbor_data) * self_msg.neigh_num;
		char neigh_ip[4] = { 0xc0,0xa8,0x02,0x01 };
		for (int i = 0; i < self_msg.neigh_num; i++) {
			neigh_ip[3] = self_msg.msg[i].node_id;
			//printf("节点id:%d\n", self_msg.msg[i].node_id);//
			//printf("-----------------------------------------拷贝前邻居IP：%08x---------------------------------------\n", neigh_ip);
			memcpy((char*)&neighbormsgptr->neighbor_ip, neigh_ip, 4);
			//neighbormsgptr->neighbor_ip = htonl(neighbormsgptr->neighbor_ip);
			//printf("-----------------------------------------拷贝后邻居IP：%08x---------------------------------------\n", neighbormsgptr->neighbor_ip);
			neighbormsgptr->neighbor_rssi = htonl(self_msg.msg[i].rssi);
			//
			neighbormsgptr->neighbor_tx = 0;
			if ((i + 1) < self_msg.neigh_num) {
				neighbormsgptr++;
			}

		}
		topo_header_ptr->mgmt_len = htons(topo_len - sizeof(Smgmt_header));
	}


	int ret = SendUDPClient(SOCKET_UDP_WG, topobuff, topo_len, &towg);


}

//判断节点是否为邻居节点，并返回下标
int neighid_isexit(int *buf,int size,int target)
{
	int i=0;
	int index=-1;
	//printf("size:%d,compare %d is exit\r\n",size,target);
	for(i=0;i<32;i++)
	{
		
		if(*(buf+i)==target)
		{
			index=i;
//			printf("find no.%d neigh,id=%d\r\n",i,*(buf+i));
		}
		else
		{	
			//printf("%d is not neigh\r\n",i);
			continue;
		}
	}
	return index;
}

uint8_t find_minMcs(uint8_t *arr,int size)
{
	uint8_t min=0x0f;   
	int i=0;
	for(i=0;i<size;i++)
	{
		if(arr[i]<min)
		{
			min=arr[i];
		}
	}
	return min;
}

uint32_t find_max(uint32_t *arr,int size)
{
	uint32_t max=0;
	int i=0;
	for(i=0;i<size;i++)
	{
		if(arr[i]>max)
		{
			max=arr[i];
		}
	}
	return max;

}
void update_time_slot_table(ob_state_part1 * part1_data, uint8_t * time_slot_tb_infor)
{
	uint8_t  ts_n_used_l0[NET_SIZE*2];
	uint8_t  ts_n_free_hx[NET_SIZE*2];
	uint8_t  ts_n_ol0_hx[NET_SIZE*2];
	uint8_t  ts_n_free_h1[NET_SIZE*2];
	uint8_t  ts_n_ol0_h1[NET_SIZE*2];
	uint8_t  ts_n_free_h2[NET_SIZE*2];
	uint8_t  ts_n_ol0_h2[NET_SIZE*2];
	uint8_t  ts_n_used_l1[NET_SIZE*2];

	int offset = 0;
	int i,j;

	memset(ts_n_used_l0,0,NET_SIZE*2);
	memset(ts_n_free_hx,0,NET_SIZE*2);
	memset(ts_n_ol0_hx,0,NET_SIZE*2);
	memset(ts_n_free_h1,0,NET_SIZE*2);
	memset(ts_n_ol0_h1,0,NET_SIZE*2);
	memset(ts_n_free_h2,0,NET_SIZE*2);
	memset(ts_n_ol0_h2,0,NET_SIZE*2);
	memset(ts_n_used_l1,0,NET_SIZE*2);

//	printf("n_used_l0 = %d,n_ol0_hx=%d,n_free_hx=%d,n_free_h1=%d,n_ol0_h1=%d,n_free_h2=%d,n_ol0_h2=%d,n_used_l1=%d\n",
//	part1_data->n_used_l0,
//	part1_data->n_ol0_hx,
//	part1_data->n_free_hx,
//	part1_data->n_free_h1,
//	part1_data->n_ol0_h1,		
//	part1_data->n_free_h2,
//	part1_data->n_ol0_h2,			
//	part1_data->n_used_l1);

	//2
	memcpy((void *)ts_n_used_l0,(void *)&part1_data->slot_list[offset],part1_data->n_used_l0);

	//3
	offset = part1_data->n_used_l0;
	memcpy((void *)ts_n_free_hx,(void *)&part1_data->slot_list[offset],part1_data->n_free_hx);

	//4
	offset = part1_data->n_used_l0+part1_data->n_free_hx;
	memcpy((void *)ts_n_ol0_hx,(void *)&part1_data->slot_list[offset],part1_data->n_ol0_hx);

	//5
	offset = part1_data->n_used_l0+part1_data->n_free_hx+part1_data->n_ol0_hx;
	memcpy((void *)ts_n_free_h1,(void *)&part1_data->slot_list[offset],part1_data->n_free_h1);

	//6
	offset = part1_data->n_used_l0+part1_data->n_free_hx+part1_data->n_ol0_hx+part1_data->n_free_h1;
	memcpy((void *)ts_n_ol0_h1,(void *)&part1_data->slot_list[offset],part1_data->n_ol0_h1);

	//7
	offset = part1_data->n_used_l0+part1_data->n_free_hx+part1_data->n_ol0_hx+part1_data->n_free_h1+part1_data->n_ol0_h1;
	memcpy((void *)ts_n_free_h2,(void *)&part1_data->slot_list[offset],part1_data->n_free_h2);

	//8
	offset = part1_data->n_used_l0+part1_data->n_free_hx+part1_data->n_ol0_hx+part1_data->n_free_h1
		+part1_data->n_ol0_h1+part1_data->n_free_h2;
	memcpy((void *)ts_n_ol0_h2,(void *)&part1_data->slot_list[offset],part1_data->n_ol0_h2);

	//9
	offset = part1_data->n_used_l0+part1_data->n_free_hx+part1_data->n_ol0_hx+part1_data->n_free_h1
		+part1_data->n_ol0_h1+part1_data->n_free_h2+part1_data->n_ol0_h2;
	memcpy((void *)ts_n_used_l1,(void *)&part1_data->slot_list[offset],part1_data->n_used_l1);

	for(i=0;i<part1_data->n_used_l0;i++)
	{
		j = ts_n_used_l0[i];
		time_slot_tb_infor[j] = 2;
	}

	for(i=0;i<part1_data->n_free_hx;i++)
	{
		j = ts_n_free_hx[i];
		time_slot_tb_infor[j] = 3;
	}
	
	for(i=0;i<part1_data->n_ol0_hx;i++)
	{
		j = ts_n_ol0_hx[i];
		time_slot_tb_infor[j] = 4;
	}

	for(i=0;i<part1_data->n_free_h1;i++)
	{
		j = ts_n_free_h1[i];
		time_slot_tb_infor[j] = 5;
	}

	for(i=0;i<part1_data->n_ol0_h1;i++)
	{
		j = ts_n_ol0_h1[i];
		time_slot_tb_infor[j] = 6;
	}
	for(i=0;i<part1_data->n_free_h2;i++)
	{
		j = ts_n_free_h2[i];
		time_slot_tb_infor[j] = 7;
	}
	for(i=0;i<part1_data->n_ol0_h2;i++)
	{
		j = ts_n_ol0_h2[i];
		time_slot_tb_infor[j] = 8;
	}
	for(i=0;i<part1_data->n_used_l1;i++)
	{
		j = ts_n_used_l1[i];
		time_slot_tb_infor[j] = 9;
	}

}

char *id[]={"id1","id2","id3","id4","id5","id6","id7","id8","id9","id10","id11","id12","id13","id14","id15","id16",
"id17","id18","id19","id20","id21","id22","id23","id24","id25","id26","id27","id28","id29","id30","id31","id32"};

char *id2hop[]={"2id1","2id2","2id3","2id4","2id5","2id6","2id7","2id8","2id9","2id10","2id11","2id12","2id13","2id14","2id15","2id16",
"2id17","2id18","2id19","2id20","2id21","2id22","2id23","2id24","2id25","2id26","2id27","2id28","2id29","2id30","2id31","2id32"};

char *ip[]={"ip1","ip2","ip3","ip4","ip5","ip6","ip7","ip8","ip9","ip10","ip11","ip12","ip13","ip14","ip15","ip16",
"ip17","ip18","ip19","ip20","ip21","ip22","ip23","ip24","ip25","ip26","ip27","ip28","ip29","ip30","ip31","ip32"};

char *ip2hop[]={"2ip1","2ip2","2ip3","2ip4","2ip5","2ip6","2ip7","2ip8","2ip9","2ip10","2ip11","2ip12","2ip13","2ip14","2ip15","2ip16",
"2ip17","2ip18","2ip19","2ip20","2ip21","2ip22","2ip23","2ip24","2ip25","2ip26","2ip27","2ip28","2ip29","2ip30","2ip31","2ip32"};

char *rssi[]={"rssi1","rssi2","rssi3","rssi4","rssi5","rssi6","rssi7","rssi8","rssi9","rssi10","rssi11","rssi12","rssi13","rssi14","rssi15","rssi16",
"rssi17","rssi18","rssi19","rssi20","rssi21","rssi22","rssi23","rssi24","rssi25","rssi26","rssi27","rssi28","rssi29","rssi30","rssi31","rssi32"};

char *snr[]={"snr1","snr2","snr3","snr4","snr5","snr6","snr7","snr8","snr9","snr10","snr11","snr12","snr13","snr14","snr15","snr16",
"snr17","snr18","snr19","snr20","snr21","snr22","snr23","snr24","snr25","snr26","snr27","snr28","snr29","snr30","snr31","snr32"};

char *timejitter[]={"timejitter1","timejitter2","timejitter3","timejitter4","timejitter5","timejitter6","timejitter7","timejitter8",
"timejitter9","timejitter10","timejitter11","timejitter12","timejitter13","timejitter14","timejitter15","timejitter16",
"timejitter17","timejitter18","timejitter19","timejitter20","timejitter21","timejitter22","timejitter23","timejitter24",
"timejitter25","timejitter26","timejitter27","timejitter28","timejitter29","timejitter30","timejitter31","timejitter32"};

char *linkquality[]={"linkquality1","linkquality2","linkquality3","linkquality4","linkquality5","linkquality6","linkquality7","linkquality8",
"linkquality9","linkquality10","linkquality11","linkquality12","linkquality13","linkquality14","linkquality15","linkquality16",
"linkquality17","linkquality18","linkquality19","linkquality20","linkquality21","linkquality22","linkquality23","linkquality24",
"linkquality25","linkquality26","linkquality27","linkquality28","linkquality29","linkquality30","linkquality31","linkquality32"};

char *bad[]={"bad1","bad2","bad3","bad4","bad5","bad6","bad7","bad8","bad9","bad10","bad11","bad12","bad13","bad14","bad15","bad16",
"bad17","bad18","bad19","bad20","bad21","bad22","bad23","bad24","bad25","bad26","bad27","bad28","bad29","bad30","bad31","bad32"};

char *good[]={"good1","good2","good3","good4","good5","good6","good7","good8","good9","good10","good11","good12","good13","good14",
	"good15","good16","good17","good18","good19","good20","good21","good22","good23","good24","good25","good26","good27","good28",
	"good29","good30","good31","good32"};

char *ucds[]={"ucds1","ucds2","ucds3","ucds4","ucds5","ucds6","ucds7","ucds8","ucds9","ucds10","ucds11","ucds12","ucds13","ucds14",
	"ucds15","ucds16","ucds17","ucds18","ucds19","ucds20","ucds21","ucds22","ucds23","ucds24","ucds25","ucds26","ucds27","ucds28",
	"ucds29","ucds30","ucds31","ucds32"};


char *signalevel[]={"signallevel","signallevel2","signallevel3","signallevel4","signallevel5","signallevel6","signallevel7","signallevel8","signallevel9",
"signallevel10","signallevel11","signallevel12","signallevel13","signallevel14","signallevel15","signallevel16","signallevel17","signallevel18","signallevel19",
"signallevel20","signallevel21","signallevel22","signallevel23","signallevel24","signallevel25","signallevel26","signallevel27","signallevel28","signallevel29",
"signallevel30","signallevel31","signallevel32"};

char *noise[]={"noise1","noise2","noise3","noise4","noise5","noise6","noise7","noise8","noise9",
"noise10","noise11","noise12","noise13","noise14","noise15","noise16","noise17","noise18","noise19",
"noise20","noise21","noise22","noise23","noise24","noise25","noise26","noise27","noise28","noise29",
"noise30","noise31","noise32"};


void reset_systeminfo_table(int j)
{
	stInData stsysteminfodata;

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",id[j-1]);  //name : idX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",ip[j-1]);  //name : ipX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",timejitter[j-1]);  //name : timejitterX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",snr[j-1]);  //name : snrX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",rssi[j-1]);  //name : rssiX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",linkquality[j-1]);  //name : linkqualityX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);	

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",good[j-1]);  //name : goodX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",bad[j-1]);  //name : ucdsX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);	

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",ucds[j-1]);  //name : deviceX
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);

	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",id2hop[j-1]);  //name : id2X
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);


	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
	sprintf(stsysteminfodata.name,"%s",ip2hop[j-1]);  //name : ip2X
	sprintf(stsysteminfodata.value,"%d",0);
	stsysteminfodata.state[0] = '0';
	updateData_systeminfo(stsysteminfodata);
}
/**
 * @brief mgmt_get_msg - 管理消息获取线程函数
 * @details 该函数是一个周期性循环线程，用于：
 *          1. 从内核获取节点的路由表和虚拟网卡信息
 *          2. 构造包含网络拓扑信息的报文（含以太网头、IP头、UDP头、管理头等）
 *          3. 统计邻居节点信息（邻居ID、MCS编码方式等）
 *          4. 更新SQLite数据库中的系统信息表、链路信息表
 *          5. 根据节点角色（网关/邻居）发送拓扑请求或拓扑数据
 *          6. 验证版本一致性（veth、agent、ctrl模块版本）
 * @param   void
 * @return  void (无返回值，周期循环运行)
 * @note    - 该函数在main中被create_Thread启动为独立线程
 *          - 涉及数据库操作需要加锁保护（sqlite3_mutex1）
 *          - 与内核通过netlink接口通信获取实时网络状态
 */
void mgmt_get_msg(void) {
	// ===== 管理消息结构体 =====
	struct mgmt_send self_msg;        // 自身节点消息结构体
	struct routetable route_msg;      // 路由表信息
	
	// ===== 拓扑相关结构体（网关和邻居间的拓扑交互）=====
	Smgmt_header topo_header;         // 拓扑消息头
	Smgmt_header* topo_header_ptr;    // 拓扑消息头指针
	struct topo_data topomsg;         // 拓扑数据
	struct topo_data* topomsgptr;     // 拓扑数据指针
	char topobuff[2048];              // 拓扑数据缓冲区
	
	// ===== 邻居信息统计数组 =====
	int neighid_info[32];             // 存放邻居ID信息
	uint8_t mcs_all[NET_SIZE];        // 存放邻居MCS编码方式信息
	
	// ===== 报文缓冲区和报文长度 =====
	char buf[2048];                   // 完整报文缓冲区（包含所有协议层）
	int len = sizeof(buf);            // 报文长度
	int ret = 0, i = 0, j = 0, k=0;   // 循环计数器
	int id_index=0;                   // 节点索引
	uint32_t seqno = 0;               // 报文序号（递增）
	uint8_t dstmac[ETH_ADDR_SIZE] = { 0xff,0xff,0xff,0xff,0xff,0xff };
	uint8_t srcmac[ETH_ADDR_SIZE] = { 0x00,0x0a,0x35,0x00,0x1e,0x54 };
	char dstip[4] = { 0xc0,0xa8,0xff,0xff };
	char srcip[4] = { 0xc0,0xa8,0x02,0x01 };
	srcmac[5] = SELFID;
	ethernet_header_t* ehdr = (ethernet_header_t*)buf;
	ip_header* iphdr = (ip_header*)(buf + sizeof(ethernet_header_t));
	udp_header* udphdr = (udp_header*)(buf + sizeof(ethernet_header_t) + sizeof(ip_header));

	Smgmt_header* hmsg;
	Snodefind* snodefind;
	int node_num = 0;
	int offset = 0;
	int* ipaddr = 0;

	uint8_t  ts_time_slot_color[NET_SIZE*2];
	

	static uint8_t mcs_stat;          // MCS统计状态
	char bcrecv_buf[BUFLEN];          // 广播接收缓冲区
	int bc_len=0;                     // 广播数据长度
	int socklen;                      // socket长度
	struct sockaddr_in from;          // 发送者地址
	bcMeshInfo *meshinfo_recv;        // 接收的mesh信息指针
	printf("调用 mgmt_get_msg函数\r\n");
	
	// ===== 设置参数相关 =====
	uint8_t cmd[200];                 // 系统命令缓冲区
	INT8 buffer[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];  // 参数设置报文缓冲区
	INT32 buflen = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);  // 缓冲区长度
	Smgmt_header* mhead = (Smgmt_header*)buffer;    // 管理头指针
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;  // 参数设置指针
	uint8_t version_compare[20];      // 版本比较字符串
	
	// ===== 初始化报文缓冲区 =====
	bzero(buffer, buflen);            // 清空参数设置报文缓冲区
	memset(cmd,0,sizeof(cmd));        // 清空命令缓冲区
	mhead->mgmt_head = htons(HEAD);   // 设置管理报文头标识
	mhead->mgmt_len = sizeof(Smgmt_set_param);    // 设置报文长度
	mhead->mgmt_type = 0;             // 初始化报文类型
	
	// ===== 数据库更新数据结构初始化 =====
	stInData stsysteminfodata;        // 系统信息数据结构
	stSurveyInfo stsurveyinfodata;    // 调查信息数据结构
	stLink   stlinkdata;              // 链路信息数据结构
	stNode   stnode;                  // 节点信息数据结构
	memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));  // 清空系统信息
	memset((char*)&stsurveyinfodata,0,sizeof(stsurveyinfodata));  // 清空调查信息
	memset((char*)&stlinkdata,0,sizeof(stLink));  // 清空链路信息
	memset((char*)&stnode,0,sizeof(stNode));      // 清空节点信息
	
	// ===== 初始化邻居信息数组 =====
	memset(neighid_info,0,sizeof(neighid_info));  // 清空邻居ID信息



	// ===== 构造管理报文 =====
	hmsg = (Smgmt_header*)(buf + sizeof(ethernet_header_t) + sizeof(ip_header) + sizeof(udp_header));  // 指向管理头位置
	snodefind = (Snodefind*)hmsg->mgmt_data;   // 指向节点发现数据位置
	hmsg->mgmt_head = htons(HEAD);    // 设置管理报文标识
	hmsg->mgmt_type = htons(MGMT_NODEFIND);  // 设置报文类型为节点发现
	hmsg->mgmt_keep = 0;              // 初始化保留字段

	srcip[3] = SELFID;                // 设置源IP最后一字节为本节点ID


	memcpy(&SELFIP, SELFIP_s, sizeof(uint32_t));  // 复制本节点IP地址
	
	// ===== 测试打印本节点IP =====
	struct in_addr selfaddr;
	selfaddr.s_addr = SELFIP;
	//printf("SELFIP为：%s\n", inet_ntoa(selfaddr));


	// ===== 构造以太网帧头 =====
	memcpy(ehdr->dest_mac_addr, dstmac, ETH_ADDR_SIZE);  // 设置目的MAC地址（广播）
	memcpy(ehdr->src_mac_addr, srcmac, ETH_ADDR_SIZE);   // 设置源MAC地址
	ehdr->ethertype = 0x0008;                            // 设置以太网类型（IP协议）

	// ===== 构造IP头 =====
	iphdr = (ip_header*)((void*)ehdr + ETH_HLEN);
	iphdr->ver_ihl = (4 << 4 | sizeof(ip_header) / sizeof(unsigned long));  // 版本4，IHL=5
	iphdr->tos = 0;               // 不区分服务
	iphdr->tlen = htons(sizeof(ip_header));  // 初始化总长度
	iphdr->identification = 1;    // 标识字段
	iphdr->flags_fo = 0;          // 标志和片偏移为0
	iphdr->ttl = 50;              // 生存时间
	iphdr->proto = IPPROTO_UDP;   // 协议类型为UDP
	iphdr->crc = 0;               // 初始化校验和
	memcpy((char*)&iphdr->saddr, srcip, 4);    // 设置源IP地址
	memcpy((char*)&iphdr->daddr, dstip, 4);    // 设置目的IP地址（广播）


	// ===== 构造UDP头 =====
	udphdr = (udp_header*)((void*)iphdr + sizeof(ip_header));
	udphdr->sport = htons(16000);              // 设置UDP源端口
	//udphdr->dport = htons(15008);
	udphdr->dport = htons(7700);               // 设置UDP目的端口（7700）
	udphdr->len = 0;                           // UDP长度初始化
	udphdr->crc = 0x0000;                      // UDP校验和初始化

	// ===== 初始化所有邻居链路信息 =====
	/* 预初始化所有32个节点的链路表，将其清零 */
	for(j=1;j<33;j++)
	{
		// 清空该节点在系统表中的信息
		reset_systeminfo_table(j);
		
		// 清空该节点在链路表中的信息（SNR、接收电平、吞吐率）
		memset((char*)&stlinkdata,0,sizeof(stLink));
		stlinkdata.m_stNbInfo[j-1].nbid1=j;            // 邻居节点ID
		stlinkdata.m_stNbInfo[j-1].snr1=0;             // 信噪比为0
		stlinkdata.m_stNbInfo[j-1].getlv1=0;           // 接收电平为0
		stlinkdata.m_stNbInfo[j-1].flowrate1=0;        // 吞吐率为0
		updateData_linkinfo(&stlinkdata,j-1,SELFID);   // 更新数据库
	}
	
	// ===== 主循环：周期性收集网络拓扑信息 =====
	while (TRUE) {
		// ===== 清空消息结构体 =====
		bzero(&self_msg, sizeof(struct mgmt_send));  // 清空自身消息结构
		node_num = 1;           // 初始化节点计数为1（本节点）
		offset = sizeof(ethernet_header_t) + sizeof(ip_header) + sizeof(udp_header) + sizeof(Smgmt_header) + sizeof(Snodefind);
		// ===== 从内核获取网络信息 =====
		mgmt_netlink_get_info(0, MGMT_CMD_GET_ROUTE_INFO, NULL, (char*)&route_msg);   // 获取路由表信息
		mgmt_netlink_get_info(0, MGMT_CMD_GET_VETH_INFO, NULL, (char*)&self_msg);    // 获取虚拟网卡信息
		
		// ===== 设置报文序列号和节点ID =====
		self_msg.seqno = seqno;        // 设置自身消息序列号
		self_msg.node_id = SELFID;     // 设置自身节点ID
		if (seqno == 0xffffffff)       // 序列号溢出处理
			seqno = 0;
		else
			seqno++;

		// ===== 构造节点发现报文 =====
		snodefind->selfid = htons(SELFID);        // 设置本节点ID（网络字节序）
		snodefind->selfip = iphdr->saddr;         // 设置本节点IP地址
		printf("node_%d has %d neigh\r\n",SELFID,self_msg.neigh_num);  // 打印邻居节点数
		
		// ===== 统计邻居信息 =====
		memset(neighid_info,0,sizeof(neighid_info));  // 清空邻居ID数组
		for (i = 0; i < self_msg.neigh_num; i++)      // 遍历所有邻居
		{
			if (self_msg.msg[i].mcs != 0x0f && self_msg.msg[i].node_id != SELFID)  // 有效邻居判断
				{
					//printf("%d mcs %d\n", i, self_msg.msg[i].mcs);
					node_num++;
					ipaddr = (int*)(buf + offset); 
					*ipaddr = htonl(0xc0a80200 + self_msg.msg[i].node_id);
					offset += sizeof(int);
				/* 存下邻居信息 */
					neighid_info[i]=self_msg.msg[i].node_id;
					mcs_all[i]=self_msg.msg[i].mcs;  //存放组网的所有mcs
					//rssi_all[i]=self_msg.msg[i].rssi;
					// printf("neigh:%d mcs:%d\t",i,mcs_all[i]);
				}
			}
	//		printf("\n");

			//mgmt_selfcheck_report();
		// ===== 从内核获取网络信息 =====
		mgmt_netlink_get_info(0, MGMT_CMD_GET_ROUTE_INFO, NULL, (char*)&route_msg);   // 获取路由表信息
		mgmt_netlink_get_info(0, MGMT_CMD_GET_VETH_INFO, NULL, (char*)&self_msg);    // 获取虚拟网卡信息
		
		// ===== 设置报文序列号和节点ID =====
		self_msg.seqno = seqno;        // 设置自身消息序列号
		self_msg.node_id = SELFID;     // 设置自身节点ID
		if (seqno == 0xffffffff)       // 序列号溢出处理
			seqno = 0;
		else
			seqno++;

		// ===== 构造节点发现报文 =====
		snodefind->selfid = htons(SELFID);        // 设置本节点ID（网络字节序）
		snodefind->selfip = iphdr->saddr;         // 设置本节点IP地址
		printf("node_%d has %d neigh\r\n",SELFID,self_msg.neigh_num);  // 打印邻居节点数
		
		// ===== 统计邻居信息 =====
		memset(neighid_info,0,sizeof(neighid_info));  // 清空邻居ID数组
		for (i = 0; i < self_msg.neigh_num; i++)      // 遍历所有邻居
		{
			if (self_msg.msg[i].mcs != 0x0f && self_msg.msg[i].node_id != SELFID)  // 有效邻居判断
			{
				node_num++;                // 有效邻居计数
				ipaddr = (int*)(buf + offset);  // 获取缓冲区中的IP地址指针
				*ipaddr = htonl(0xc0a80200 + self_msg.msg[i].node_id);  // 构造邻居IP地址
				offset += sizeof(int);     // 偏移量向后移动
				
				/* 存下邻居信息用于后续处理 */
				neighid_info[i]=self_msg.msg[i].node_id;  // 记录邻居ID
				mcs_all[i]=self_msg.msg[i].mcs;           // 记录邻居MCS编码方式
			}
		}

		// ===== 计算报文长度并设置各层协议头 =====
		snodefind->node_num = htons(node_num);   // 设置节点总数（包括本身）
		len = offset;                             // 计算最终报文长度

		// 计算各层报文长度（从后向前）
		hmsg->mgmt_len = htons(len - (sizeof(ethernet_header_t) + sizeof(ip_header) + sizeof(udp_header) + sizeof(Smgmt_header)));
		udphdr->len = htons(len - sizeof(ethernet_header_t) - sizeof(ip_header));
		iphdr->tlen = htons(len - sizeof(ethernet_header_t));
		iphdr->crc = ipCksum((void*)iphdr, 20);   // 计算IP头校验和

		// ===== 根据节点角色发送拓扑信息 =====
		// 网关节点持续向网管发送自身拓扑包
		if (is_conned == 1) {
			// 网管查询完节点信息就和节点连接上，网管地址会赋值到全局的wg_addr，节点持续发送
			send_topo_msg(wg_addr, self_msg);     // 发送拓扑消息给网管
			send_topo_request();                   // 发送拓扑请求

		}
		
		// 邻居节点收到网关节点的拓扑信息请求，则持续向请求节点发送拓扑信息
		if (gotRequest == 1) {
			send_topo_msg(gate_addr, self_msg);   // 发送拓扑消息给网关

		}

		// ===== 版本一致性检查 =====
		/* 检查veth（虚拟网卡）、agent（代理）、ctrl（控制）模块版本是否一致 */
		
		// 检查虚拟网卡版本
		memset(version_compare,0,sizeof(version_compare));
		sprintf(version_compare,"V%d.%d.%d",self_msg.veth_version[1],self_msg.veth_version[2],self_msg.veth_version[3]);
		if(strcmp(version, version_compare) != 0)
		{
			// 版本不一致，可在此处处理
		}

		// 检查代理模块版本
		memset(version_compare,0,sizeof(version_compare));
		sprintf(version_compare,"V%d.%d.%d",self_msg.agent_version[1],self_msg.agent_version[2],self_msg.agent_version[3]);
		if(strcmp(version, version_compare) != 0)
		{
			// agent版本不一致
		}

		// 检查控制模块版本
		memset(version_compare,0,sizeof(version_compare));
		sprintf(version_compare,"V%d.%d.%d",self_msg.ctrl_version[1],self_msg.ctrl_version[2],self_msg.ctrl_version[3]);
			if(strcmp(version, version_compare) != 0)
			{
				//printf("The version of mac-ctrl is inconsistent %s\n",version_compare);
			}




		//更新数据库
		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","ipaddr");
		sprintf(stsysteminfodata.value,"%d.%d.%d.%d",SELFIP_s[0],SELFIP_s[1],SELFIP_s[2],SELFIP_s[3]);
		stsysteminfodata.state[0] = '1';
		updateData_systeminfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","device");
		sprintf(stsysteminfodata.value,"%d",SELFID);
		stsysteminfodata.state[0] = '1';
		updateData_systeminfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","g_ver");
		sprintf(stsysteminfodata.value,"%s",version);
		stsysteminfodata.state[0] = '1';
		updateData_systeminfo(stsysteminfodata);	

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","rf_freq");
		sprintf(stsysteminfodata.value,"%d",FREQ_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_systeminfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","m_chanbw");
		sprintf(stsysteminfodata.value,"%d",BW_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_systeminfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","m_txpower");
		sprintf(stsysteminfodata.value,"%d",POWER_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_systeminfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","devicetype");
		sprintf(stsysteminfodata.value,"%d",DEVICETYPE_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_meshinfo(stsysteminfodata);

		

		

		
		if(self_msg.neigh_num==0)
		{
			/* 不存在邻居，刷新数据库*/
			//printf("refresh systeminfo neighbor info \r\n");
			for(j=1;j<33;j++)
			{
				/*  clear systemInfo table */

				reset_systeminfo_table(j);
				
				/*  clear link table */
				memset((char*)&stlinkdata,0,sizeof(stLink));
				stlinkdata.m_stNbInfo[j-1].nbid1=j;
				stlinkdata.m_stNbInfo[j-1].snr1=0;
				stlinkdata.m_stNbInfo[j-1].getlv1=0;
				stlinkdata.m_stNbInfo[j-1].flowrate1=0;
				updateData_linkinfo(&stlinkdata,j-1,SELFID);

			}
		}
		else
		{
			for(j=1;j<33;j++)
			{
				id_index=neighid_isexit(neighid_info,sizeof(neighid_isexit),j);
				if(id_index<0)
				{
					/* 非邻居节点，全部置0*/
					//update systemInfo table
					reset_systeminfo_table(j);

					//update link table
					memset((char*)&stlinkdata,0,sizeof(stLink));
					stlinkdata.m_stNbInfo[j-1].nbid1=j;
					stlinkdata.m_stNbInfo[j-1].snr1=0;
					stlinkdata.m_stNbInfo[j-1].getlv1=0;
					stlinkdata.m_stNbInfo[j-1].flowrate1=0;
					updateData_linkinfo(&stlinkdata,j-1,SELFID);
					//continue;
				}
				else
				{
//					printf("index:%d,find neighbor id: %d info\r\n",id_index,self_msg.msg[id_index].node_id);
					if(self_msg.msg[id_index].mcs != 0x0f && self_msg.msg[id_index].node_id != SELFID)
					{
						// printf("update neigh info:id:%d,time_jitter:%d,snr:%d,rssi:%d,mcs:%d \r\n"
						// 	,self_msg.msg[id_index].node_id,self_msg.msg[id_index].time_jitter
						// 	,self_msg.msg[id_index].snr,self_msg.msg[id_index].rssi,self_msg.msg[id_index].mcs);

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",id[self_msg.msg[id_index].node_id-1]);  //name : deviceX
//						printf("update name %s\r\n",stsysteminfodata.name);
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].node_id);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);


						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",ip[j-1]);  //name : ipX
						sprintf(stsysteminfodata.value,"192.168.2.%d",self_msg.msg[id_index].node_id);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",timejitter[j-1]);  //name : timejitterX
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].time_jitter);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",snr[j-1]);  //name : snrX
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].snr);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",rssi[j-1]);  //name : rssiX
						sprintf(stsysteminfodata.value,"%d",-self_msg.msg[id_index].rssi);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",linkquality[j-1]);  //name : linkqualityX
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].mcs);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);//更新本地 SQLite 数据库中 systemInfo 表数据	

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",good[j-1]);  //name : goodX
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].good);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);
						
						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",noise[j-1]);  //name : ucdsX
						sprintf(stsysteminfodata.value,"%d",-self_msg.msg[id_index].noise);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",bad[j-1]);  //name : ucdsX
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].bad);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);	

						memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						sprintf(stsysteminfodata.name,"%s",ucds[j-1]);  //name : deviceX
						sprintf(stsysteminfodata.value,"%d",self_msg.msg[id_index].ucds);
						stsysteminfodata.state[0] = '1';
						updateData_systeminfo(stsysteminfodata);

						for(k=1;k<33;k++)
						{
							if(self_msg.mac_information_part1.nbr_list[k] == LinkSt_h2)
							{
								memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
								sprintf(stsysteminfodata.name,"%s",id2hop[j-1]);  //name : id2X
								sprintf(stsysteminfodata.value,"%d",k);
								stsysteminfodata.state[0] = '1';
								updateData_systeminfo(stsysteminfodata);


								memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
								sprintf(stsysteminfodata.name,"%s",ip2hop[j-1]);  //name : ip2X
								sprintf(stsysteminfodata.value,"192.168.2.%d",k);
								stsysteminfodata.state[0] = '1';
								updateData_systeminfo(stsysteminfodata);
							}
						}


					//update link table
						memset((char*)&stlinkdata,0,sizeof(stLink));
						stlinkdata.m_stNbInfo[id_index].nbid1=self_msg.msg[id_index].node_id;
//						printf("update neigh_%d link info\r\n ",stlinkdata.m_stNbInfo[id_index].nbid1);
						stlinkdata.m_stNbInfo[id_index].snr1=self_msg.msg[id_index].snr;
						stlinkdata.m_stNbInfo[id_index].getlv1=-self_msg.msg[id_index].rssi;
						stlinkdata.m_stNbInfo[id_index].flowrate1=10;
						updateData_linkinfo(&stlinkdata,id_index,SELFID);
					}
				}


			}
			if(rate_auto==1)
			{
				//rate auto mode
				if(mcs_stat!=find_minMcs(mcs_all,self_msg.neigh_num))
				{
					/* mcs档位需要切换 */
					printf("mcs update ");
					bzero(buffer, buflen);//清空参数设置报文缓冲区
					memset(cmd,0,sizeof(cmd));
					mhead->mgmt_head = htons(HEAD);
					mhead->mgmt_len = sizeof(Smgmt_set_param);
					mhead->mgmt_type = 0;
					//mhead->mgmt_type |= MGMT_SET_MULTICAST_MCS;
					//mparam->mgmt_virt_multicast_mcs=mcs;

					mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
					mparam->mgmt_virt_unicast_mcs=find_minMcs(mcs_all,self_msg.neigh_num);
					printf("set param mcs:%d\r\n",mparam->mgmt_virt_unicast_mcs);
					mhead->mgmt_type = htons(mhead->mgmt_type);
					mgmt_netlink_set_param(buffer, buflen,NULL);
					sleep(1);		
					if (!persist_test_db()) {
						printf("[mgmt_transmit] persist test.db failed after auto MCS update\n");
					}

					mcs_stat=find_minMcs(mcs_all,self_msg.neigh_num);

					memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
					sprintf(stsysteminfodata.name,"%s","m_rate");
					sprintf(stsysteminfodata.value,"%d",mparam->mgmt_virt_unicast_mcs);
					stsysteminfodata.state[0] = '1';
					updateData_systeminfo(stsysteminfodata);
				}

			}
		}	

		memset(ts_time_slot_color,1,NET_SIZE*2);

		ts_time_slot_color[SELFID] = 0;

		update_time_slot_table(&(self_msg.mac_information_part1),ts_time_slot_color);//更新数据库中的时隙表
		
		for(j=0;j<NET_SIZE*2;j++)
		{
			updateData_timeslotinfo(ts_time_slot_color[j],j+1);
		}


		
		sleep(5);  //间隔5秒写库
		

//update node table
		// sprintf(stnode.id,"%d",SELFID);
		// sprintf(stnode.ip,"192.168.2.%d",SELFID);
		// sprintf(stnode.txpower,"",);
		// sprintf(stnode.bw,"192.168.2.%d",SELFID);
		// sprintf(stnode.freq,"192.168.2.%d",SELFID);
		// sprintf(stnode.mcs,"192.168.2.%d",SELFID);
		// sprintf(stnode.mode,"192.168.2.%d",SELFID);
		// sprintf(stnode.max,"192.168.2.%d",SELFID);
		// sprintf(stnode.nbor,"192.168.2.%d",SELFID);
		// sprintf(stnode.interval,"192.168.2.%d",SELFID);
		// sprintf(stnode.lotd,"192.168.2.%d",SELFID);
		// sprintf(stnode.latd,"192.168.2.%d",SELFID);
		// sprintf(stnode.softver,"192.168.2.%d",SELFID);
		// sprintf(stnode.harver,"192.168.2.%d",SELFID);
		//updateData_nodeinfo(stnode);
		

		/////////////////////////////////////////////

		//		if(SELFID == GROUND_STA){
		//			//send
		////			mgmt_mysql_write((int)SELFID,(char*)&self_msg,sizeof(struct mgmt_send));
		//			len = sizeof(struct mgmt_send) - (NET_SIZE - self_msg.neigh_num)*sizeof(struct mgmt_msg);
		//			memcpy(buf,(char*)&self_msg,len);
		////			SendUDPClient(SOCKET_GROUND,buf,len,&S_GROUND_PC);
		//			SendNodeMsg(buf,len);
		//		}else{
		//			len = sizeof(struct mgmt_send) - (NET_SIZE - self_msg.neigh_num)*sizeof(struct mgmt_msg);
		//			//printf("len = %d %d %d\n",len,sizeof(struct mgmt_send),self_msg.neigh_num);
		//			self_msg.node_id = SELFID;
		//			memcpy(buf,(char*)&self_msg,len);
		////			ret = SendUDPClient(SOCKET_MGMT,buf,len,&S_GROUND_STD);
		////			printf("ret = %d %d\n",ret,self_msg.neigh_num);
		//			SendNodeMsg(buf,len);
		//		}
		////		if(ISLOGIN)
		////			SendNodeMsg(buf,len);
		//		//mgmt_mysql_con(&mgmt_info);
		
		
		
	}
}


void updateData_init(void){
stInData stsysteminfodata;

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","rf_freq");
		sprintf(stsysteminfodata.value,"%d",FREQ_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_meshinfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","m_chanbw");
		sprintf(stsysteminfodata.value,"%d",BW_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_meshinfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","m_txpower");
		sprintf(stsysteminfodata.value,"%d",POWER_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_meshinfo(stsysteminfodata);

		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","workmode");
		sprintf(stsysteminfodata.value,"%d",NET_WORKMOD_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_meshinfo(stsysteminfodata);
		
		memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
		sprintf(stsysteminfodata.name,"%s","devicetype");
		sprintf(stsysteminfodata.value,"%d",DEVICETYPE_INIT);
		stsysteminfodata.state[0] = '0';
		updateData_meshinfo(stsysteminfodata);
}

//add by yang 20130312
//将double的数据转换成网络字节序
/*
double double_to_network(double num) {
	uint64_t x;
	double res = num;
	memcpy(&x, &num, sizeof(double));
	x = htonll(x);
	memcpy(&res,&x,sizeof(double));
	return res;
}
*/


void mgmt_status_report(struct sockaddr_in from) {

	//测试打印
	struct sockaddr_in report_addr = from;
	mgmt_status_header ms_header;
	mgmt_status_data* ms_data;
	char buff[2048];
	//int len = sizeof(buff);

	char selfip[4] = { 0xc0,0xa8,0x02,0x01 };
	selfip[3] = SELFID;

	// 创建结构体指针
	mgmt_status_header* pms_header = malloc(sizeof(mgmt_status_header));
	//状态包头部填充
	pms_header->flag = htons(HEAD);
	pms_header->type = htons(MGMT_DEVINFO_REPORT);
	pms_header->reserved = 0;

	//状态包内容填充.
	ms_data = &(pms_header->status_data);
	ms_data->selfid = htons(SELFID);
	memcpy((char*)&ms_data->selfip, selfip, 4);
	//ms_data->selfip = htonl(ms_data->selfip);
	ms_data->tv_route = htons(1000);
	ms_data->maxHop = htons(50);
	ms_data->num_queues = htons(100);
	ms_data->depth_queues = htons(100);
	ms_data->qos_policy = 0;
	ms_data->mcs_unicast = MCS_INIT;
	ms_data->mcs_broadcast = 0;
	ms_data->bw = BW_INIT;
	ms_data->reserved = 0;
	ms_data->freq = htonl(FREQ_INIT);
	ms_data->txpower = htons(POWER_INIT);
	ms_data->work_mode = htons(MACMODE_INIT);
	//ms_data->longitude = double_to_network(118.76);
	//ms_data->latitude = double_to_network(32.04);
	ms_data->longitude = htond(longitude);
	ms_data->latitude = htond(latitude);
	strcpy(ms_data->software_version, "version");
	strcpy(ms_data->hardware_version, "version");

	pms_header->len = htons(sizeof(mgmt_status_data) + 8);

	//测试打印-------------------------------
	printf("ms_header数据---------------------\n");
	printf("sizeof(ms_header) = %d\n", sizeof(ms_header));
	printf("%04x ", pms_header->flag);
	printf("%04x ", pms_header->len);
	printf("%04x ", pms_header->type);
	printf("%04x ", pms_header->reserved);
	printf("数据内容：\n");
	printf("%08x ", ms_data->selfip);
	printf("%04x ", ms_data->selfid);
	printf("%04x ", ms_data->tv_route);
	printf("%04x ", ms_data->maxHop);
	printf("%04x ", ms_data->num_queues);
	printf("%04x ", ms_data->qos_policy);
	printf("%02x ", ms_data->mcs_unicast);
	printf("%02x ", ms_data->mcs_broadcast);
	printf("%02x ", ms_data->bw);
	printf("%02x ", ms_data->reserved);
	printf("freq：%d,%08x ", ms_data->freq, ms_data->freq);//00007805，正确应该是00000578
	printf("%04x ", ms_data->txpower);
	printf("%04x ", ms_data->work_mode);
	printf("%016x ", ms_data->longitude);
	printf("%016x ", ms_data->latitude);
	printf("%s ", ms_data->software_version);
	printf("%s ", ms_data->hardware_version);
	printf("\n");
	printf("ms_header数据---------------------\n");
	printf("buff拷贝前数据---------------------\n");
	printf("%s", buff);
	printf("\n");
	printf("buff拷贝前数据---------------------\n");
	printf("\n");
	//测试打印-------------------------------	

	memcpy(buff, pms_header, sizeof(mgmt_status_header));

	printf("buff拷贝后数据---------------------\n");
	printf("拷贝的数据大小：%d\n", sizeof(mgmt_status_header));
	printf("%s", buff);
	printf("\n");
	printf("buff拷贝后数据---------------------\n");

	int ret = SendUDPClient(SOCKET_UDP_WG, buff, sizeof(ms_header), &report_addr);
	printf("状态数据包socket发送情况：%d\n", ret);
	printf("状态数据包socket发送端口%u\n", ntohs(report_addr.sin_port));
}

/*  CRC-16  */
uint16_t    CRC_TABLE[256]=
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
//----------------------------------------------------------
//-- CRC16 calculate: X16 + X12 + X5 + 1
//----------------------------------------------------------
uint16_t CalculateCRC(uint16_t *source_Dat, int Dat_len)
{
	uint16_t    CRC, tmp;
    int               i;
	if(NULL == source_Dat)
	{
		return 0;
	}

    CRC = 0;
    for(i = 0; i <Dat_len; i++)
	{
	// 首先计算每个字的高8位，再计算低8位
        tmp =((CRC>>8)&0xff)^((*source_Dat>>8)&0xff);
        CRC =((CRC<<8)&0xff00)^CRC_TABLE[tmp];
        tmp =((CRC>>8)&0xff)^ (*source_Dat&0xff);
        CRC =((CRC<<8)&0xff00)^CRC_TABLE[tmp];
        source_Dat++;
    }
	
    return CRC;
}

#ifdef Radio_QK

//试验控制指令应答
void report_cmd_ctrl_ack(int seq,uint16_t type)
{
	char buffer[1024];
	
	//static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		

	CMD_ACK cmd_ack;
	memset(&cmd_ack,0,sizeof(CMD_ACK));		

	int app_len=sizeof(APP_HEAD);
	int cmd_len=sizeof(CMD_ACK);
	
	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+cmd_len;
	app_head.packet_type=CMD_EXPERIMENT_ACK;   //试验控制指令应答
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=cmd_len;
	app_head.recv_type=type;    //4 :业务模拟软件
	app_head.recv_id=0;

	cmd_ack.state=0;
	cmd_ack.type=1;

	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,&cmd_ack,cmd_len);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",QK_CJ_PORT);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));


	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+cmd_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);

 
}


/* 配置指令执行应答 
len:数据域长度
type：指令类型      0：设备参数配置指令，2：业务通道1配置指令
recv_type:接收端表示
*/
void report_device_param_set_ack(int seq,uint8_t type,uint16_t recv_type)
{
	char buffer[1024];
	
	//static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		

	PARAM_SET_ACK param_ack;
	memset(&param_ack,0,sizeof(PARAM_SET_ACK));		

	// DEVICE_PARAM_SET device_param_set;
	// memset(&device_param_set,0,sizeof(DEVICE_PARAM_SET));		

	int app_len=sizeof(APP_HEAD);
	int ack_len=sizeof(PARAM_SET_ACK);
	
	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+ack_len;
	app_head.packet_type=CMD_CONFIG_ACK;    //宽带台配置指令执行应答
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=ack_len;
	app_head.recv_type=recv_type;    //4 :业务模拟软件
	app_head.recv_id=0;
	// device_param_set.routing_prot=;
	param_ack.type=type;   
	param_ack.state=0;

	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,&param_ack,ack_len);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",QK_WG_PORT);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));


	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+ack_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);

}


/* 设备网络状态上报 */
void report_device_network_status(void* data,int seq,int port,uint16_t type)
{
	char buffer[1024];
	
	//static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		

	DEVCIE_NETWORK dev_net;
	memset(&dev_net,0,sizeof(DEVCIE_NETWORK));		
	memcpy(&dev_net,(DEVCIE_NETWORK*)data,sizeof(DEVCIE_NETWORK));


	int app_len=sizeof(APP_HEAD);
	int dev_net_len=sizeof(DEVCIE_NETWORK);
	
	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+dev_net_len;
	app_head.packet_type=CMD_NET_STATUS_REPORT;    //宽带台设备网络状态上报
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=dev_net_len;
	app_head.recv_type=type;    //4 :业务模拟软件
	app_head.recv_id=0;

	dev_net.dev_type=1;
	dev_net.dev_id=SELFID;
	
	dev_net.longitude=110.123123;
	dev_net.latitude=30.123123;
	dev_net.height=1500;

	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,&dev_net,dev_net_len);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",port);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));


	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+dev_net_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);

}

/* 设备指标评估数据 
*/
void report_device_evolution(DEVICE_EVALUATION_REPORT *dev_evolution,int seq,int port,uint16_t type)
{
	//static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	char buffer[1024];

	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		
	
	int app_len=sizeof(APP_HEAD);
	int dev_evolution_len=sizeof(DEVICE_EVALUATION_REPORT);

	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+dev_evolution_len;
	app_head.packet_type=CMD_METRICS_REPORT;    //宽带台设备网络状态上报
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=dev_evolution_len;
	app_head.recv_type=type;    //4 :业务模拟软件
	app_head.recv_id=0;

	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,dev_evolution,dev_evolution_len);


	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",port);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+dev_evolution_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);


}

/* 自检指令应答 */
void report_self_test_ack(SELFCHECK_STATUS_INFO *sc_status,int seq,uint16_t type)
{
	int broadcast_enable = 1;
	int cli_s;
	char buffer[1024];

	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		
	
	int app_len=sizeof(APP_HEAD);
	int dev_sc_len=sizeof(SELFCHECK_STATUS_INFO);

	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+dev_sc_len;
	app_head.packet_type=CMD_SEFL_TEST_ACK;    //宽带台设备自检状态信息
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;   //宽带电台

	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=dev_sc_len;
	app_head.recv_type=type;    
	app_head.recv_id=0;
	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,sc_status,dev_sc_len);


	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",QK_CJ_PORT);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+dev_sc_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);

}

/* 设备自检状态信息数据 */
void report_dev_selfcheck_status(DEVICE_SC_STATUS_REPORT *sc_info,int seq,uint16_t type)
{
	//static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	char buffer[1024];

	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		
	
	int app_len=sizeof(APP_HEAD);
	int dev_sc_len=sizeof(DEVICE_SC_STATUS_REPORT);

	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+dev_sc_len;
	app_head.packet_type=CMD_SELF_TEST_INFO;    //宽带台设备自检状态信息
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;   //宽带电台
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=dev_sc_len;
	app_head.recv_type=type;    //4 :业务模拟软件
	app_head.recv_id=0;
	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,sc_info,dev_sc_len);


	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",QK_WG_PORT);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+dev_sc_len,&addr);
	//printf("send selftest status_len %d \r\n",send_len);
	close(cli_s);

}

/* 设备状态信息数据 */
void report_device_status(DEVICE_STATUS_REPORT *dev_status,int seq,int port,uint16_t type)
{
	//	static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	char buffer[1024];

	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		
	
	int app_len=sizeof(APP_HEAD);
	int dev_status_len=sizeof(DEVICE_STATUS_REPORT);

	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+dev_status_len;
	app_head.packet_type=CMD_DEV_STATUS_INFO;    //宽带台设备自检状态信息
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=dev_status_len;
	app_head.recv_type=type;    //4 :业务模拟软件
	app_head.recv_id=0;

	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,dev_status,dev_status_len);


	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",port);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+dev_status_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);

}
// 业务通道1状态信息上报
void report_device_ch_param(CHANNEL_PARAM_REPORT *ch_param,int seq,int port,uint16_t type)
{
	//static int seq=0;
	int broadcast_enable = 1;
	int cli_s;
	char buffer[1024];

	APP_HEAD app_head;
	memset(&app_head,0,sizeof(APP_HEAD));		
	
	int app_len=sizeof(APP_HEAD);
	int ch_param_len=sizeof(CHANNEL_PARAM_REPORT);

	app_head.head=240;
	app_head.len=app_len;
	app_head.info_len=app_len+ch_param_len;
	app_head.packet_type=CMD_CH1_STATUS_INFO;    //宽带台设备自检状态信息
	app_head.activity_type=1;
	app_head.send_type=DEVICE_TYPE_KD;
	app_head.send_id=SELFID;
	app_head.seq=seq;
	app_head.data_len=ch_param_len;
	app_head.recv_type=type;    //4 :业务模拟软件
	app_head.recv_id=0;

	memcpy(buffer,&app_head,app_len);
	memcpy(buffer+app_len,ch_param,ch_param_len);


	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	cli_s=createUdpClient(&addr,"192.168.2.255",port);
	
	setsockopt(cli_s, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

	int send_len=SendUDPClient(cli_s,(char*)&buffer,app_len+ch_param_len,&addr);
	//printf("send status_len %d \r\n",len);
	close(cli_s);
}

//接收qk业务模拟信息
void mgmt_recv_from_qkwg(void)
{
	int ret;
	int yw_len=0;
	char yw_buf[1024];
	struct sockaddr_in from;
	static int yw_seq=0;
	// static uint8_t s_sync_mode;

	bool isset=FALSE;

	char selfAddr[4] = {0xc0,0xa8,0x02,0x01};
	uint32_t selfip;
	selfAddr[3]=SELFID;
	memcpy(&selfip,selfAddr,sizeof(uint32_t));

	int qk_s = CreateUDPServer(QK_WG_PORT);
	if (qk_s <= 0)
	{
		printf("ERROR: create socket_qk_wg \n");
		exit(1);
	}

	int app_len=sizeof(APP_HEAD);

	int socklen=0;

	printf("create thread : get msg from 20 yw_web\r\n");

	/* 试验控制指令 */
	// CMD_INFO cmd_info;
	// memset(&cmd_info,0,sizeof(CMD_INFO));

	// 设备参数配置指令数据
	DEVICE_PARAM_SET dev_param_set;
	memset(&dev_param_set,0,sizeof(DEVICE_PARAM_SET));
	
	/* 业务通道1参数配置指令数据 */
	CHANNEL_PARAM_SET 	ch_param_set;
	memset(&ch_param_set,0,sizeof(CHANNEL_PARAM_SET));


	uint8_t cmd[200];
	INT8 buffer[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];
	INT32 buflen = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);
	memset(buffer,0,buflen);
	Smgmt_header* mhead = (Smgmt_header*)buffer;
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;


	static uint8_t s_wokrmode=1;

	// bzero(buffer, buflen);
	// memset(cmd,0,sizeof(cmd));
	// mhead->mgmt_head = htons(HEAD);
	// mhead->mgmt_len = sizeof(Smgmt_set_param);
	// mhead->mgmt_type = 0;
 	// mhead->mgmt_keep = 0;


	while(TRUE)
	{
		memset(cmd,0,sizeof(cmd));
		mhead->mgmt_head = htons(HEAD);
		mhead->mgmt_len = sizeof(Smgmt_set_param);
		mhead->mgmt_type = 0;
		mhead->mgmt_keep = 0;


		yw_len=recvfrom(qk_s, yw_buf, BUFLEN, 0, &from, &socklen);
		if(yw_len<=0||selfip==from.sin_addr.s_addr)
		{
			continue;
		}
		//buflen = RecvUDPClient(qk_s, buffer, BUFLEN, &from, &socklen);
		
		//printf("[YW_20] len %d \r\n",yw_len);
		APP_HEAD app_head;
		memcpy(&app_head,yw_buf,app_len);
		//解析head信息

		switch(app_head.packet_type)
		{
			// case CMD_EXPERIMENT_CTRL:
				
			// 	memset(&cmd_info,0,sizeof(CMD_INFO));
			// 	memcpy(&cmd_info,buffer+app_len,sizeof(CMD_INFO));
			// 	printf("recv cmd ctrl info \r\n");
			// 	printf("---state:%d---\r\n",cmd_info.state);
			// 	printf("---scenario:%d----\r\n",cmd_info.scenario);
			// 	report_cmd_ctrl_ack();
			// break;
			
			case CMD_DEV_CONFIG:
				
				memset(&dev_param_set,0,sizeof(DEVICE_PARAM_SET));
				memcpy(&dev_param_set,yw_buf+app_len,sizeof(DEVICE_PARAM_SET));
				printf("recv device param set info \r\n");
				// printf("---routing prot:%#x--- \r\n",dev_param_set.routing_prot);
				// printf("---lon:%lf--- \r\n",dev_param_set.lon);
				// printf("---lat:%lf--- \r\n",dev_param_set.lat);
				// printf("---height:%lf--- \r\n",dev_param_set.height);
				//printf("---network role:%d---\r\n",dev_param_set.work_mode);
				/* set param  */ 

				if(dev_param_set.routing_prot!=0x5a)
				{
					//set route
					switch(dev_param_set.routing_prot){
						case KD_ROUTING_OLSR:
							printf("set route olsr \r\n");
							g_radio_param.g_route=KD_ROUTING_OLSR;
							sprintf(cmd,
								"sed -i \"s/router .*/router %d/g\" /etc/node_xwg",
							KD_ROUTING_OLSR);		
							system(cmd);
							sleep(1);		
							ret = system("/home/root/cs_olsr.sh");
							if(ret == -1) printf("change olsr failed\r\n");
							break;
							
						case KD_ROUTING_AODV:
							// aodv
							printf("set route aodv \r\n");
							sprintf(cmd,
								"sed -i \"s/router .*/router %d/g\" /etc/node_xwg",
							KD_ROUTING_AODV);		
							system(cmd);
							sleep(1);
							ret = system("/home/root/cs_aodv.sh");
							if(ret == -1) printf("change batman failed\r\n");
							
							g_radio_param.g_route=KD_ROUTING_AODV;
							break;
							
						case KD_ROUTING_CROSS_LAYER:  //batman
							printf("set route batman \r\n");
							g_radio_param.g_route=KD_ROUTING_CROSS_LAYER;
							sprintf(cmd,
								"sed -i \"s/router .*/router %d/g\" /etc/node_xwg",
							KD_ROUTING_CROSS_LAYER);		
							system(cmd);
							sleep(1);
							ret = system("/home/root/cs_batman.sh");
							if(ret == -1) printf("change batman failed\r\n");
							break;
							
						default:
							break;
					}

					// updateData_meshinfo_qk("m_route",dev_param_set.routing_prot);
				}

				/* report ack to yw_wg */
				report_device_param_set_ack(yw_seq,0,DEVICE_TYPE_YW);
			break;
			
			case CMD_CH1_CONFIG:
				
				memset(&ch_param_set,0,sizeof(CHANNEL_PARAM_SET));
				memcpy(&ch_param_set,yw_buf+app_len,sizeof(CHANNEL_PARAM_SET));
				printf("recv channel param set info \r\n");
				/* set param  */ 
				// printf("---mcs:%d---\r\n",ch_param_set.mcs_mode);
				// printf("---workmode:%d---\r\n",ch_param_set.work_mode);
				// printf("---bw:%d---\r\n",ch_param_set.bw);
				// printf("---freq:%d---\r\n",ch_param_set.freq);
				// printf("---select freq_1 %d freq_2 %d freq-3 %d freq_4 %d ---\r\n",
				// ch_param_set.select_freq1,ch_param_set.select_freq2,ch_param_set.select_freq3,ch_param_set.select_freq4);


				/* set channel param */
				if(ch_param_set.mcs_mode!=0x5a)
				{
					//set mcs 0-7
					isset=TRUE;
					mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;
					mparam->mgmt_virt_unicast_mcs=ch_param_set.mcs_mode;   
					
					// memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
					// sprintf(stsysteminfodata.name,"%s","m_rate");
					// sprintf(stsysteminfodata.value,"%d",ch_param_set.mcs_mode);
					// stsysteminfodata.state[0] = '1';
					// updateData_systeminfo(stsysteminfodata);
					
					updateData_systeminfo_qk("m_rate",ch_param_set.mcs_mode);
					// updateData_meshinfo_qk("m_rate",ch_param_set.mcs_mode);
					g_radio_param.g_rate=ch_param_set.mcs_mode;
				}
				if(ch_param_set.transmode!=0x5a)
				{
					isset=TRUE;
					mhead->mgmt_type |= MGMT_SET_TEST_MODE;
					mparam->mgmt_mac_work_mode=htons(ch_param_set.transmode);

					// updateData_meshinfo_qk("m_trans_mode",ch_param_set.transmode);

					g_radio_param.g_trans_mode=ch_param_set.transmode;
				}
				if(ch_param_set.bw!=0x5a)
				{
					// set bw 
					isset=TRUE;
					mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
					mparam->mgmt_mac_bw=ch_param_set.bw;

					// memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
					// sprintf(stsysteminfodata.name,"%s","m_chanbw");
					// sprintf(stsysteminfodata.value,"%d",ch_param_set.bw);
					// stsysteminfodata.state[0] = '1';
					// updateData_systeminfo(stsysteminfodata);
					
					// updateData_meshinfo_qk("m_chanbw",ch_param_set.bw);
					updateData_systeminfo_qk("m_chanbw",ch_param_set.bw);
					g_radio_param.g_chanbw=ch_param_set.bw;
				}
				if(ch_param_set.kylb==1)
				{
					printf("[YW DEBUG]open kylb \r\n");
					if(s_wokrmode==4)
					{
						break;
					}
					isset=TRUE;
					mhead->mgmt_type |= MGMT_SET_WORKMODE;
					mparam->mgmt_net_work_mode.NET_work_mode=5;
					sprintf(cmd,
								"sed -i \"s/kylb .*/kylb %d/g\" /etc/node_xwg",
							KYLB_MODE_OPEN);		
					system(cmd);
					s_wokrmode=5;
					// updateData_meshinfo_qk("workmode",5);
				}
				else
				{
					printf("[YW DEBUG]close kylb \r\n");
					// if(s_wokrmode==4)
					// {
					// 	break;
					// }
					sprintf(cmd,
								"sed -i \"s/kylb .*/kylb %d/g\" /etc/node_xwg",
							KYLB_MODE_CLOSE);		
					system(cmd);					
				}
				if(ch_param_set.work_mode==4)
				{
					//自适应选频
					isset=TRUE;
					mhead->mgmt_type |= MGMT_SET_WORKMODE;
					//mhead->mgmt_type = htons(mhead->mgmt_type);
					mparam->mgmt_net_work_mode.NET_work_mode=ch_param_set.work_mode;

					printf("set workmode:%d \r\n",mparam->mgmt_net_work_mode.NET_work_mode);


					//meshinfo.select_freq_isset=0;
					printf("[YW_20] set select freq,freq1:%d,freq2:%d,freq3:%d，freq4:%d \r\n",
								ch_param_set.select_freq1,ch_param_set.select_freq2,ch_param_set.select_freq3,ch_param_set.select_freq4);
					mparam->mgmt_net_work_mode.fh_len=4;
					mparam->mgmt_net_work_mode.hop_freq_tb[0]=ch_param_set.select_freq1;
					mparam->mgmt_net_work_mode.hop_freq_tb[1]=ch_param_set.select_freq2;
					mparam->mgmt_net_work_mode.hop_freq_tb[2]=ch_param_set.select_freq3;
					mparam->mgmt_net_work_mode.hop_freq_tb[3]=ch_param_set.select_freq4;

					s_wokrmode=4;

					sprintf(cmd, "sed -i \"s/workmode .*/workmode %d/; \
								s/select_freq1 .*/select_freq1 %d/; \
								s/select_freq2 .*/select_freq2 %d/; \
								s/select_freq3 .*/select_freq3 %d/; \
								s/select_freq4 .*/select_freq4 %d/\" /etc/node_xwg", 
							WORK_MODE_TYPE_ZSYXP, ch_param_set.select_freq1, ch_param_set.select_freq2, ch_param_set.select_freq3, ch_param_set.select_freq4);
					system(cmd);
					
					// updateData_meshinfo_qk("workmode",4);
					// updateData_meshinfo_qk("m_select_freq1",ch_param_set.select_freq1);	
					// updateData_meshinfo_qk("m_select_freq2",ch_param_set.select_freq2);	
					// updateData_meshinfo_qk("m_select_freq3",ch_param_set.select_freq3);	
					// updateData_meshinfo_qk("m_select_freq4",ch_param_set.select_freq4);	

				}
				else if(ch_param_set.work_mode==1)
				{
						isset=TRUE;
						mhead->mgmt_type |= MGMT_SET_WORKMODE;
						mparam->mgmt_net_work_mode.NET_work_mode=1;

						
						mhead->mgmt_type |= MGMT_SET_FREQUENCY;
						printf("[YW_20] freq %d\r\n",ch_param_set.freq);
						if(ch_param_set.freq<DT_MIN_FREQ)
						{
							printf("[YW_20] WARNING freq <225 \r\n ");
							ch_param_set.freq=DT_MIN_FREQ;
						}
						if(ch_param_set.freq>DT_MAX_FREQ)
						{
							printf("[YW_20] WARNING freq > 4000 \r\n ");
							ch_param_set.freq=DT_MAX_FREQ;

						}
						mparam->mgmt_mac_freq=htonl(ch_param_set.freq);

						// memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
						// sprintf(stsysteminfodata.name,"%s","rf_freq");
						// sprintf(stsysteminfodata.value,"%d",ch_param_set.freq);
						// stsysteminfodata.state[0] = '1';
						// updateData_systeminfo(stsysteminfodata);

						g_radio_param.g_workmode=1;
						g_radio_param.g_rf_freq=ch_param_set.freq;

						sprintf(cmd,
							"sed -i \"s/workmode .*/workmode %d/g\" /etc/node_xwg",
							WORK_MODE_TYPE_DP);		
						system(cmd);
						s_wokrmode=1;
					// updateData_meshinfo_qk("workmode",1);
					// updateData_meshinfo_qk("rf_freq",ch_param_set.freq);
				}
				if(ch_param_set.tx_power!=0x5a)
				{
					// set tx_power				
					//isset=TRUE;
					// mhead->mgmt_type |= MGMT_SET_POWER;
					// mparam->mgmt_mac_txpower=htons(ch_param_set.tx_power);

/* 更新 systeminfo ,同步显示*/
					// memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));
					// sprintf(stsysteminfodata.name,"%s","m_txpower");
					// sprintf(stsysteminfodata.value,"%d",39-m_power);
					// stsysteminfodata.state[0] = '1';
					// updateData_systeminfo(stsysteminfodata);

				}
				if(ch_param_set.slot_len!=0x5a)
				{
					//set slot len
					isset=TRUE;
					mhead->mgmt_keep |= MGMT_SET_SLOTLEN;
					mparam->u8Slotlen=ch_param_set.slot_len;

				// updateData_meshinfo_qk("m_slot_len",ch_param_set.slot_len);
					g_radio_param.g_slot_len=ch_param_set.slot_len;
				}
				if(ch_param_set.sync_mode!=0x5a)
				{
					/*同步模式 0：内同步，1：外同步*/
					printf("[YW DEBUG]sync mode %d \r\n",ch_param_set.sync_mode);
					sprintf(cmd,
						"sed -i \"s/sync_mode .*/sync_mode %d/g\" /etc/node_xwg",
						ch_param_set.sync_mode);		
					system(cmd);

				}


				if(isset)
				{
					isset=FALSE;
					if(buffer == NULL) {     
						printf("ERROR: buffer is NULL\n");         
						continue;
       				 }
					mhead->mgmt_type = htons(mhead->mgmt_type);
					mhead->mgmt_keep = htons(mhead->mgmt_keep);
					mgmt_netlink_set_param(buffer, buflen,NULL);	
					sleep(1);
					if (!persist_test_db()) {
						printf("[mgmt_transmit] persist test.db failed after channel param update\n");
					}
				}				
				/* report ack to wg */
				report_device_param_set_ack(yw_seq,2,DEVICE_TYPE_YW);

			break;
			
			default:
			break;
		}

		usleep(50000);
	}

}

//
void read_node_xwg_file(const char* filename,Node_Xwg_Pairs* xwg_info,int num_pairs)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("无法打开文件");
        return;
    }	

	for (int i = 0; i < num_pairs; i++) {
        xwg_info[i].found = 0;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // 移除行尾的换行符
        line[strcspn(line, "\n")] = 0;
        
        // 分割键和值
        char *key = strtok(line, " ");
        char *value_str = strtok(NULL, " ");
        
        if (key != NULL && value_str != NULL) {
            // 检查是否为需要提取的键
            for (int i = 0; i < num_pairs; i++) {
                if (strcmp(key, xwg_info[i].key) == 0) {
                    memcpy(xwg_info[i].value,value_str,100);
                    xwg_info[i].found = 1;
                    break;
                }
            }
        }
    }
    fclose(file);	

}

// 快速获取键对应的值
const char *get_value(Node_Xwg_Pairs* pairs,const char *key) {
    for (int i = 0; i < MAX_XWG_PAIRS; i++) {
        if (strcmp(key, pairs[i].key) == 0 && pairs[i].found) {
            return pairs[i].value;
        }
    }
    return NULL;
}

double get_double_value(Node_Xwg_Pairs* pairs,const char *key)
{
	const char *value = get_value(pairs,key);
    if (value != NULL) {
        return atof(value);
    }
    return 0.0;

}

// 快速获取键对应的整数值
int get_int_value(Node_Xwg_Pairs* pairs,const char *key) {
    const char *value = get_value(pairs,key);
    if (value != NULL) {
        return atoi(value);
    }
    return -1;
}



/* 接收场景系统指令 */
void mgmt_recv_from_qkcj(void)
{
	int buflen=0;
	char buffer[1024];
	struct sockaddr_in from;
	static int cj_seq=0;
	static uint8_t cmd_state=CMD_STATE_START;     

	char selfAddr[4] = {0xc0,0xa8,0x02,0x01};
	uint32_t selfip;
	selfAddr[3]=SELFID;
	memcpy(&selfip,selfAddr,sizeof(uint32_t));
	// struct in_addr selfIP;
	// selfIP.s_addr = selfip;


	int qk_s = CreateUDPServer(CJ_MULTIADDR_PORT);
	if (qk_s <= 0)
	{
		printf("ERROR: create socket_qk_wg \n");
		exit(1);
	}
	add_multiaddr_group(qk_s,CJ_MULTIADDR_IP);


	int app_len=sizeof(APP_HEAD);

	int socklen=0;

	printf("create thread : get msg from 20 cj_web\r\n");
	/* 试验控制指令 */
	CMD_INFO cmd_info;
	memset(&cmd_info,0,sizeof(CMD_INFO));

	/* 自检指令 */
	CMD_SC cmd_sc;
	memset(&cmd_sc,0,sizeof(CMD_SC));

	/* 设备自检状态信息 */
	SELFCHECK_STATUS_INFO sc_ack;
	memset(&sc_ack,1,sizeof(SELFCHECK_STATUS_INFO));
/* test */
	sc_ack.dev_type=1;
	sc_ack.dev_id=SELFID;

	while(TRUE)
	{
		buflen=recvfrom(qk_s, buffer, BUFLEN, 0, &from, &socklen);
		// printf("[CJ DEBUG] cj len:%d\r\n",buflen);
		if(buflen<=0||selfip==from.sin_addr.s_addr)   			//过滤自身发出的包
		{
			continue;
		}

		//buflen = RecvUDPClient(qk_s, buffer, BUFLEN, &from, &socklen);
		APP_HEAD app_head;
		memcpy(&app_head,buffer,app_len);
		// printf("cj packet from ip addr:%s",inet_ntoa(from.sin_addr));
		// printf("send_type:%d,send_id:%d,recv_type:%d\r\n",app_head.send_type,app_head.send_id,app_head.recv_type);

		switch(app_head.packet_type)
		{
			case CMD_SELF_TEST:
				memset(&cmd_sc,0,sizeof(CMD_SC));
				memcpy(&cmd_sc,buffer+app_len,sizeof(CMD_SC));
				// printf("recv cmd selfcheck info \r\n");
				// printf("---state:%#x---\r\n",cmd_sc.time);
				// printf("---scenario:%d----\r\n",cmd_sc.type);
				
				report_self_test_ack(&sc_ack,cj_seq,DEVICE_TYPE_CJ);
			break;
			case CMD_EXPERIMENT_CTRL:
				memset(&cmd_info,0,sizeof(CMD_INFO));
				memcpy(&cmd_info,buffer+app_len,sizeof(CMD_INFO));
				// printf("recv cmd ctrl info \r\n");
				// printf("---state:%d---\r\n",cmd_info.state);
				// printf("---scenario:%d----\r\n",cmd_info.scenario);

				/* 更新试验状态 */
				cmd_state=cmd_info.state;
				g_cj_state=cmd_state;
				report_cmd_ctrl_ack(cj_seq,DEVICE_TYPE_CJ);

			break;
		}
		cj_seq++;

		usleep(50000);
	}
}





void thread_report_test(void)
{

	static int seq=0;
	uint8_t i=0;
	uint8_t id_index=0;


	struct mgmt_send self_msg;
	/* 设备指标评估信息 */
	DEVICE_EVALUATION_REPORT dev_evo;
	memset(&dev_evo,0,sizeof(DEVICE_EVALUATION_REPORT));
/* test */


	/* 设备自检状态信息 */
	DEVICE_SC_STATUS_REPORT sc_info;
	memset(&sc_info,0,sizeof(DEVICE_SC_STATUS_REPORT));

	/* 填充设备网络状态信息 */
	DEVCIE_NETWORK dev_net_info;
	memset(&dev_net_info,0,sizeof(DEVCIE_NETWORK));
	dev_net_info.dev_type=1;
	dev_net_info.dev_id=SELFID;
	dev_net_info.zsyxp=1;
	dev_net_info.kylb=1; 
	dev_net_info.longitude=110.123123;
	dev_net_info.latitude=29.123123;
	dev_net_info.height=1500;

	DEVICE_STATUS_REPORT dev_status;
	memset(&dev_status,0,sizeof(DEVICE_STATUS_REPORT));
	dev_status.routing_prot=g_radio_param.g_route;
	dev_status.work_mode=g_radio_param.g_workmode;
	memset(dev_status.reserved,100,4);


	Node_Xwg_Pairs param_pairs[] = {
        {"channel", 0, 0},{"power", 0, 0},{"bw", 0, 0},{"mcs", 0, 0},
        {"macmode", 0, 0},{"slotlen", 0, 0},{"router", 0, 0},{"workmode", 0, 0},
		{"select_freq1", 0, 0},{"select_freq2", 0, 0},{"select_freq3", 0, 0},{"select_freq4", 0, 0},
		{"sync_mode",0,0},{"kylb",0,0}
    };

	// read_node_xwg_file("node_xwg",pairs,7);

	// 填充业务通道1状态信息
	CHANNEL_PARAM_REPORT ch_param;
	memset(&ch_param,0x5a,sizeof(CHANNEL_PARAM_REPORT));
	ch_param.wave_type=1;
	ch_param.multi_access=0x02;
	ch_param.sync_mode=0;
	printf("start thread :report info to yw\r\n");


	memset(&self_msg,0,sizeof(self_msg));
	while(1)
	{
		read_node_xwg_file("/etc/node_xwg",param_pairs,MAX_XWG_PAIRS);

		mgmt_netlink_get_info(0, MGMT_CMD_GET_VETH_INFO, NULL, (char*)&self_msg);

		//上报给业务模拟系统
		//自检信息
			memcpy(&sc_info,&self_msg.amp_infomation,sizeof(sc_info));
			report_dev_selfcheck_status(&sc_info,seq,DEVICE_TYPE_YW);

		// 设备网络状态信息	
			for(i=0;i<NET_SIZE;i++)
			{
				if(self_msg.msg[i].mcs!=0x0f)
				{
					dev_net_info.connectivity=1;
					id_index=self_msg.msg[i].node_id;
					dev_net_info.neighbor_info[id_index]=	1;
				}

			}
			if((uint8_t)get_int_value((void*)param_pairs,"workmode")==4)
			{
				dev_net_info.zsyxp=0;
			}
			else if((uint8_t)get_int_value((void*)param_pairs,"workmode")==5)
			{
				dev_net_info.kylb=0;
			}
			report_device_network_status((void*)&dev_net_info,seq,QK_WG_PORT,DEVICE_TYPE_YW);


		//设备指标评估数据
			
			for(i=0;i<NET_SIZE;i++)
			{
				dev_evo.snr[i]=self_msg.msg[i].snr;
				//dev_evo.ber[i]=self_msg.msg[i].
			}

			report_device_evolution(&dev_evo,seq,QK_WG_PORT,DEVICE_TYPE_YW);
		//设备状态信息数据
			report_device_status(&dev_status,seq,QK_WG_PORT,DEVICE_TYPE_YW);
		// 业务通道1状态信息上报
			ch_param.transmode=(uint8_t)get_int_value((void*)param_pairs,"macmode");
			ch_param.mcs_mode=(uint8_t)get_int_value((void*)param_pairs,"mcs");
			ch_param.slot_len=(uint8_t)get_int_value((void*)param_pairs,"slotlen");
			ch_param.freq=(uint16_t)get_int_value((void*)param_pairs,"channel");
			ch_param.sync_mode=(uint8_t)get_int_value((void*)param_pairs,"sync_mode");
			ch_param.kylb=0; //空域滤波默认关闭 
			ch_param.work_mode=(uint8_t)get_int_value((void*)param_pairs,"workmode");
			if((uint8_t)get_int_value((void*)param_pairs,"kylb")==KYLB_MODE_OPEN) 
			{
				ch_param.kylb=1;
			}
			ch_param.bw=(uint8_t)get_int_value((void*)param_pairs,"bw");
			
			ch_param.select_freq1=get_int_value((void*)param_pairs,"select_freq1");
			ch_param.select_freq2=get_int_value((void*)param_pairs,"select_freq2");
			ch_param.select_freq3=get_int_value((void*)param_pairs,"select_freq3");
			ch_param.select_freq4=get_int_value((void*)param_pairs,"select_freq4");

			// ch_param.select_freq1=1500;
			// ch_param.select_freq2=1500;
			// ch_param.select_freq3=1500;
			// ch_param.select_freq4=1500;



			ch_param.tx_power=1;   //需要更新
			ch_param.tx_power_atten=10; //需要更新

			//printf("业务通道1状态信息上报: \r\n");
			// printf("[YW REPORT] workmode:%d \r\n",ch_param.work_mode);
			// printf("[YW REPORT] bw:%d \r\n",ch_param.bw);
			// printf("[YW REPORT] freq:%d \r\n",ch_param.freq);
			// printf("[YW REPORT] mcs:%d \r\n",ch_param.mcs_mode);
			// printf("[YW REPORT] slot_len:%d \r\n",ch_param.slot_len);

			report_device_ch_param(&ch_param,seq,QK_WG_PORT,DEVICE_TYPE_YW);

		//上报给场景系统
		if(g_cj_state==CMD_STATE_START||g_cj_state==CMD_STATE_CONTINUE)
		{
			report_device_network_status((void*)&dev_net_info,seq,QK_CJ_PORT,DEVICE_TYPE_CJ);
			report_device_evolution(&dev_evo,seq,QK_CJ_PORT,DEVICE_TYPE_CJ);
			
			//report_device_status(&dev_status,seq,QK_WG_PORT);
			report_device_ch_param(&ch_param,seq,QK_CJ_PORT,DEVICE_TYPE_CJ);
		}



			seq++;
			sleep(5);
	}


}

#endif

#ifdef Radio_CEC
tdpa_slot_type get_slot_type(uint32_t slot_num)
{
    tdpa_slot_type type;
    //每个小周期（以22个时隙为一个周期）
    //第1个时隙为扫描时隙，第2个时隙为控制时隙
    //后面的20个时隙为业务时隙
    tdpa_network_work_phase work_phase;
    if(NET_WORKMOD_INIT == PRIOR_POSIZTION_MODE){
		work_phase = DATA_PHASE;

	}
	else{
		NET_WORKMOD_INIT = SCAN_PHASE;
	}

	
    if(work_phase == SCAN_PHASE){
        if (slot_num % (Sub_N_Slots) == 0)
        {
            type = Control_Slot;
        }
        else if ( slot_num % (Sub_N_Slots) == 1 ||
        		slot_num % (Sub_N_Slots) == 2 ||
				slot_num % (Sub_N_Slots) == 3 ||
				slot_num % (Sub_N_Slots) == 4 ||
				slot_num % (Sub_N_Slots) == 5)
        {
            type = Scan_Slot;
        }
        else
        {
            type = Traffic_Slot;
        }
    }else if(work_phase == DATA_PHASE){
        if (slot_num % (Sub_N_Slots) == 0)
        {
            type = Control_Slot;
        }
        else
        {
            type = Traffic_Slot;
        }
    }

    return type;
}


uint16_t change_rec_beam_to_2bytes(uint8_t direction){

    int interface_num = 0;
	uint16_t rec_beam = 0;
	uint8_t  direc_temp = 0;
    if (direction >= 1 && direction <= 15 )
    {
        interface_num = 1;
    }
    else if (direction >= 16 && direction <= 30)
    {
        interface_num = 2;
    }
    else if (direction >= 31 && direction <= 45)
    {
        interface_num = 3;
    }
    else if (direction >= 46 && direction <= 60)
    {
        interface_num = 4;
    }
	else{
        printf("ERROR:input error direction value = %d\n" ,direction);
	}
	rec_beam = direction - (interface_num-1)*15;

	rec_beam = rec_beam << ((interface_num-1)*4);


    return rec_beam;

}

void Beam_msg_send(void){

     uint32_t header    = 0x474B5A4A;
	 uint32_t ICD_coder = 0x00001003;
	 uint32_t state_num = BEAM_STATE_NUM;
	 uint8_t  data_buf[BEAM_DATA_LEN];
	 int i;
	 uint32_t TOD_s;
	 uint32_t TOD_p;
	 uint8_t  tx_beam;
	 uint16_t  rx_beam;

	 int offset=0;
	 int traffic_slot_cnt = 0;
	 int index;
	 uint8_t  tx_node_id;
	 uint8_t  rx_node_id;
	 int ret;


	 //header = htonl(header);
	 //ICD_coder = htonl(ICD_coder);
	 //state_num = htonl(state_num);

	 memcpy(data_buf,&header,4);
	 memcpy(data_buf+4,&ICD_coder,4);
	 memcpy(data_buf+8,&state_num,4);

	 

	 for(i=0;i<BEAM_STATE_NUM;i++){

       TOD_s = START_TIME + pps_cnt;
	   TOD_p = i + (frame_cnt - 1)* TOTAL_SLOT_NUM;
       tx_beam = 0;
	   rx_beam = 0;
		if(get_slot_type(i) == Control_Slot){
			if( i/Sub_N_Slots == SELFID -1){
               index = (frame_cnt-1)%NEIGBOR_NUM;
			   //printf("1: index = %d,Neigbor_direction[%d] = %d\n",index,index,Neigbor_direction[index]);
			   tx_beam = Neigbor_direction[index];
			}
			else if (i/Sub_N_Slots <= NEIGBOR_NUM) {
				index = i/Sub_N_Slots-1;
				// printf("2: index = %d,Neigbor_direction[%d] = %d\n",index,index,Neigbor_direction[index]);
				 rx_beam= change_rec_beam_to_2bytes(Neigbor_direction[index]);
			}

		}
		else if(get_slot_type(i) == Traffic_Slot){
            
           
		   tx_node_id = traffic_slot_cnt%NET_SIZE +1;
		   if(tx_node_id == SELFID){
              index = (traffic_slot_cnt/NET_SIZE)%NEIGBOR_NUM;
			  //printf("3: index = %d,Neigbor_direction[%d] = %d\n",index,index,Neigbor_direction[index]);
			  tx_beam = Neigbor_direction[index];

		   }
		   else{
		   	  	index = tx_node_id-2;
				//printf("4: index = %d,Neigbor_direction[%d] = %d\n",index,index,Neigbor_direction[index]);
				rx_beam= change_rec_beam_to_2bytes(Neigbor_direction[index]);
		   	}

		   traffic_slot_cnt++;


		}

        //TOD_s = htonl(TOD_s);
		//TOD_p = htonl(TOD_p);
		//rx_beam = htons(rx_beam);
		memcpy(data_buf+12+i*BEAM_STATE_LEN,&TOD_s,4);
		memcpy(data_buf+12+i*BEAM_STATE_LEN+4,&TOD_p,4);
		memcpy(data_buf+12+i*BEAM_STATE_LEN+8,&tx_beam,1);
		memcpy(data_buf+12+i*BEAM_STATE_LEN+9,&rx_beam,2);

		


	 }
	ret = SendUDPClient(SOCKET_UDP_BEAM, data_buf, BEAM_DATA_LEN, &BEAM_RX_add);
    if(ret<0){
       printf("ERROR: socket Send beam infor fail\r\n");
	}


}

void Beam_infor_update_cyclic(void){


    while(1){


	  frame_cnt++;

	  if(frame_cnt>8){
	    pps_cnt++;
		frame_cnt = 1;
	  }
	  
      sleep(5);
	  //usleep(110000);

	  Beam_msg_send();


	}

}

#endif
