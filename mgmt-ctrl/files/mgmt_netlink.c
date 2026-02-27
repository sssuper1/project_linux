#include "mgmt_netlink.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
//#include <net/ethernet.h>
//#include <net/if.h>
//#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mgmt_transmit.h"

static int last_err;
struct print_opts {
	int read_opt;
	float orig_timeout;
	float watch_interval;
	nl_recvmsg_msg_cb_t callback;
	char* remaining_header;
	const char* static_header;
	uint8_t nl_cmd;
};

Smgmt_transmit_info mgmt_info;

ob_state_part1 slot_info;


#define MIN_TXPOWER 0
#define MAX_TXPOWER 70
#define MIN_BW		0
#define MAX_BW		4
// #define MIN_FREQ	225
// #define MAX_FREQ	3000

static int print_error(struct sockaddr_nl* nla, struct nlmsgerr* nlerr, void* arg)
{
	if (nlerr->error != -EOPNOTSUPP)
		fprintf(stderr, "Error received: %s\n",
			strerror(-nlerr->error));

	last_err = nlerr->error;

	return NL_STOP;
}


static const struct nla_policy mgmt_netlink_policy[NUM_MGMT_ATTR] = {
	[MGMT_ATTR_GET_INFO] = {.type = NLA_U32 },
	[MGMT_ATTR_SET_PARAM] = {.type = NLA_U32 },
	[MGMT_ATTR_NODEID] = {.type = NLA_U8 },
	[MGMT_ATTR_VERSION_ROUTE] = {.type = NLA_STRING },
	[MGMT_ATTR_PKTNUMB_ROUTE] = {.type = NLA_STRING },
	[MGMT_ATTR_TABLE_ROUTE] = {.maxlen = sizeof(struct routetable) },
	[MGMT_ATTR_UCDS_ROUTE] = {.type = NLA_STRING },

	[MGMT_ATTR_VERSION_VETH] = {.type = NLA_STRING },
	[MGMT_ATTR_INFO_VETH] = {.type = NLA_STRING,.maxlen = sizeof(jgk_report_infor) },

//	[MGMT_ATTR_INFO_VETH2]	      = {
//									  .type = NLA_STRING,
//									  .maxlen = sizeof(jgk_report_infor12) },
	[MGMT_ATTR_VETH_ADDRESS] = {.type = NLA_UNSPEC,.minlen = ETH_ALEN,.maxlen = ETH_ALEN },
	[MGMT_ATTR_VETH_TX] = {.type = NLA_U32 },
	[MGMT_ATTR_VETH_RX] = {.type = NLA_U32 },
	[MGMT_ATTR_VERSION_MAC_PHY] = {.type = NLA_STRING },
	[MGMT_ATTR_FREQ] = {.type = NLA_U32 },
	[MGMT_ATTR_BW] = {.type = NLA_U16 },
	[MGMT_ATTR_TXPOWER] = {.type = NLA_S16 },
	[MGMT_ATTR_SET_NODEID] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_INTERVAL] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_TTL] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_QUEUE_NUM] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_QUEUE_LENGTH] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_QOS_STATEGY] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_UNICAST_MCS] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_MULTICAST_MCS] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_FREQUENCY] = {.type = NLA_U32 },
	[MGMT_ATTR_SET_POWER] = {.type = NLA_BINARY}, 
	[MGMT_ATTR_SET_BANDWIDTH] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_TEST_MODE] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_TEST_MODE_MCS] = {.type = NLA_U16 },
	[MGMT_ATTR_SET_PHY] = {.type = NLA_STRING,.maxlen = sizeof(Smgmt_phy)},
	[MGMT_ATTR_SET_WORKMODE] = {.type = NLA_STRING,.maxlen = sizeof(Smgmt_net_work_mode)},
	[MGMT_ATTR_SET_IQ_CATCH] = {.type = NLA_STRING,.maxlen = sizeof(Smgmt_IQ_Catch)},
	[MGMT_ATTR_SET_SLOTLEN] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_POWER_LEVEL] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_POWER_ATTENUATION] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_RX_CHANNEL_MODE] = {.type = NLA_U8 },
#ifdef Radio_CEC
	[MGMT_ATTR_SET_NCU] = {.type = NLA_U8 },

#endif
};


static int missing_mandatory_attrs(struct nlattr* attrs[],
	const int mandatory[], int num)
{
	int i;

	for (i = 0; i < num; i++)
		if (!attrs[mandatory[i]])
			return -EINVAL;

	return 0;
}

static const int info_mandatory[] = {
};

/* rssi档位对应关系
    85 < rssi              0
	75＜ rssi ≤ 85         2
	65＜ rssi ≤ 75         3
	55＜ rssi ≤ 65         4
		 rssi ≤ 55         5


*/
static UINT8 rssi_tab[] = { 55,65,75,85 };  //更新后的档位表  20241111 edit by sdg
static UINT8 mgmt_q_2_rssi(uint32_t rssi_back)
{
	uint8_t ret=0;
	if(rssi_back<=rssi_tab[0])
	{
		return 5;
	}
	else if(rssi_back<=rssi_tab[1])
	{
		return 4;
	}
	else if(rssi_back<=rssi_tab[2])
	{
		return 3;
	}
	else if(rssi_back<=rssi_tab[1])
	{
		return 2;
	}
	else 
		return 0;
	
	
	return ret;


}
//static UINT8 qmcs_tab[MCS_NUM] = { 3,6,8,10,11,12,13,14 };
static UINT8 qmcs_tab[MCS_NUM] = { 2,6,9,14 };  //更新后的档位表  20241107 edit by sdg

//mcs档位映射  
/*
0＜mcs≤2   0
2＜mcs≤6   2
6＜mcs≤9   3
9＜mcs<15   4
*/
static UINT8 mgmt_q_2_mcs(UINT8 q_back) {
	UINT8 i = 0;
	if (q_back == 0x0f)
		return 0x0f;
	else {
		if (q_back <= qmcs_tab[0])
			return 0;
		else if(q_back <= qmcs_tab[1])
			return 2;
		else if(q_back <= qmcs_tab[2])
			return 3;
		else if(q_back <= qmcs_tab[3])
			return 4;
		else;
		
	}
	return 0x0f;
}

/* 链路自适应平滑处理机制 */
uint8_t smooth_mcs(uint8_t  new_data)
{
	// const uint8_t size=2;   //定义窗口大小
	static uint8_t arr[MCS_WINDOW_SIZE];
	static int index = 0;
	static uint8_t flag =0;

	//uint8_t sum = 0;  

	arr[index] = new_data;
	
	if (index == MCS_WINDOW_SIZE-1)
	{
		/* 窗口已满 */
		flag = 1; 
	}
	index = (index + 1) % MCS_WINDOW_SIZE; //下标后移
	if (flag)
	{

		return find_minMcs(arr,MCS_WINDOW_SIZE);  //计算窗口内的平均值
	}
	else		
		return find_minMcs(arr,index);
}

uint32_t smmoth_rssi(uint32_t new_rssi)
{
	static uint32_t arr[MCS_WINDOW_SIZE];
	static int index = 0;
	static uint8_t flag =0;

	//uint8_t sum = 0;  

	arr[index] = new_rssi;
	
	if (index == MCS_WINDOW_SIZE-1)
	{
		/* 窗口已满 */
		flag = 1; 
	}
	index = (index + 1) % MCS_WINDOW_SIZE; //下标后移
	if (flag)
	{

		return find_max(arr,MCS_WINDOW_SIZE);  //计算窗口内的平均值
	}
	else		
		return find_max(arr,index);
}

static int mgmt_netlink_info_callback(struct nl_msg* msg, void* arg)
{
	struct nlattr* attrs[MGMT_ATTR_MAX + 1];
	struct nlmsghdr* nlh = nlmsg_hdr(msg);
	struct print_opts* opts = arg;
	const uint8_t* primary_mac;
	struct genlmsghdr* ghdr;
	const uint8_t* mesh_mac;
	const char* primary_if;
	const char* mesh_name;
	const char* version;
	char* extra_info = NULL;
	uint8_t ttvn = 0;
	uint16_t bla_group_id = 0;
	const char* algo_name;
	const char* extra_header;
	int ret;
	int value;
	char* addr;
	char* jgk_node;
	char* croutet;
	char* veth_jgk_data;
	struct batadv_jgk_node* bat_jgk_node;
	struct routetable* routet;
	uint8_t i = 0;
	int neigh_num = 0;
	struct mgmt_send* smsg = (struct mgmt_send*)(opts->remaining_header);




	//	printf("mgmt_netlink_info_callback\n");
	//	struct routetable1* routet1;
	jgk_report_infor* jgk_information_data;
	// printf("[MGMT_CTRL] get netlink data size %d,copy amp info from kernel \r\n",sizeof(jgk_report_infor));

	//	jgk_report_infor11* test1;
	//	jgk_report_infor12* test2;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);
	//
	//	if (ghdr->cmd != MGMT_CMD_GET_ROUTE_INFO)
	//		return NL_OK;

	if (nla_parse(attrs, MGMT_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		genlmsg_len(ghdr), mgmt_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	switch (opts->nl_cmd) {
	case MGMT_CMD_GET_ROUTE_INFO:
	{
		smsg = opts->remaining_header;
		jgk_node = nla_data(attrs[MGMT_ATTR_PKTNUMB_ROUTE]);
		bat_jgk_node = (struct batadv_jgk_node*)jgk_node;
		//		printf("mgmt_ctrl bat_jgk_node nodeid: %d, s_ogm: %d, r_ogm: %d, f_ogm: %d, s_bcast: %d, r_bcast: %d, f_bcast: %d\n", bat_jgk_node->nodeid,
		//				bat_jgk_node->s_ogm, bat_jgk_node->r_ogm, bat_jgk_node->f_ogm, bat_jgk_node->s_bcast, bat_jgk_node->r_bcast, bat_jgk_node->f_bcast);

		croutet = nla_data(attrs[MGMT_ATTR_TABLE_ROUTE]);
		routet = (struct routetable*)croutet;
		memcpy(smsg, croutet, sizeof(struct routetable));
		//		printf("routet %d\n",routet->bat_jgk_route[0].route);

		//		smsg->node_id = bat_jgk_node->nodeid;


		break;
	}
	case MGMT_CMD_GET_VETH_INFO:
	{

		veth_jgk_data = nla_data(attrs[MGMT_ATTR_INFO_VETH]);
		jgk_information_data = (jgk_report_infor*)veth_jgk_data;
		//		printf("mgmt_ctrl jgk_information_data enqueue_bytes %d outqueue_bytes %d node_id %d tx_inall %d tx_outall %d rx_inall %d rx_outall %d\n",
		//					jgk_information_data->enqueue_bytes[0],jgk_information_data->outqueue_bytes[0],jgk_information_data->mac_information_part1.node_id,
		//					jgk_information_data->traffic_queue_information.tx_inall,jgk_information_data->traffic_queue_information.tx_outall,
		//					jgk_information_data->traffic_queue_information.rx_inall,jgk_information_data->traffic_queue_information.rx_outall);

		//		smsg->node_id = jgk_information_data->mac_information_part1.node_id;
		smsg->freq = FREQ_INIT;
		smsg->bw = BW_INIT;
		smsg->txpower = POWER_INIT;
		memcpy(smsg->veth_version,jgk_information_data->veth_version,sizeof(int));
		memcpy(smsg->agent_version,jgk_information_data->agent_version,sizeof(int));
		memcpy(smsg->ctrl_version,jgk_information_data->ctrl_version,sizeof(int));
		// printf("get netlink data size %d,copy amp info from kernel \r\n",sizeof(jgk_report_infor));
		memcpy(&smsg->amp_infomation,&jgk_information_data->amp_infomation,sizeof(DEVICE_SC_STATUS_REPORT));

		// printf("[MGMT_CTRL] amp info: power_temperature:%d,power_ac220_power:%d,freq_12v_voltage:%d,freq_lo1_freq:%d\r\n",jgk_information_data->amp_infomation.power_temperature,
			// jgk_information_data->amp_infomation.power_ac220_power,	jgk_information_data->amp_infomation.freq_12v_voltage,jgk_information_data->amp_infomation.freq_lo1_freq);

//		smsg->veth_version=jgk_information_data->veth_version;
//		smsg->agent_version = jgk_information_data->agent_version;
//		smsg->ctrl_version = jgk_information_data->ctrl_version;
		//		printf("%d %d %d %d\n",jgk_information_data->mac_information_part2.rssi[108],
		//				jgk_information_data->mac_information_part2.rssi[109],
		//				jgk_information_data->mac_information_part2.rssi[110],
		//				jgk_information_data->mac_information_part2.rssi[111]);
		for (i = 1; i < MCS_NUM; i++) {
			smsg->rx += jgk_information_data->enqueue_bytes[i];
			smsg->tx += jgk_information_data->outqueue_bytes[i];
		}
		for (i = 1; i < NET_SIZE; i++) {
//						printf("i %d %d\n",i,jgk_information_data->mac_information_part2.mcs[i]);
			if (jgk_information_data->mac_information_part2.mcs[i] != NO_MCS) {
				smsg->msg[neigh_num].node_id = i;
				//				smsg->msg[neigh_num].enqueue_bytes = jgk_information_data->enqueue_bytes[4]/1000;
				//				smsg->msg[neigh_num].outqueue_bytes = jgk_information_data->outqueue_bytes[4]/1000;
				//smsg->msg[neigh_num].mcs = mgmt_q_2_mcs(smooth_mcs(jgk_information_data->mac_information_part2.mcs[i]));//mgmt_q_2_mcs(smooth_mcs(jgk_information_data->mac_information_part2.mcs[i]))
				smsg->msg[neigh_num].mcs=jgk_information_data->mac_information_part2.mcs[i];
				smsg->msg[neigh_num].rssi = jgk_information_data->mac_information_part2.rssi[i];//mgmt_q_2_rssi(smmoth_rssi(jgk_information_data->mac_information_part2.rssi[i]))
				smsg->msg[neigh_num].snr = jgk_information_data->mac_information_part2.snr[i];
				smsg->msg[neigh_num].noise = jgk_information_data->mac_information_part2.noise[i];
				smsg->msg[neigh_num].ucds = jgk_information_data->mac_information_part2.ucds[i];
				smsg->msg[neigh_num].time_jitter = jgk_information_data->mac_information_part2.time_jitter[i];
				smsg->msg[neigh_num].good = jgk_information_data->mac_information_part2.good[i];
				smsg->msg[neigh_num].bad = jgk_information_data->mac_information_part2.bad[i];
//				printf("neigh i %d node %d mcs %d rssi %d snr %d noise %d good %d bad %d \n", i, smsg->msg[neigh_num].node_id, smsg->msg[neigh_num].mcs,
//					smsg->msg[neigh_num].rssi, smsg->msg[neigh_num].snr,smsg->msg[neigh_num].noise,smsg->msg[neigh_num].good,smsg->msg[neigh_num].bad);
				neigh_num++;
			}
		}
		smsg->neigh_num = neigh_num;
//		printf("n_used_l0 = %d,n_ol0_hx=%d,n_free_hx=%d,n_free_h1=%d,n_ol0_h1=%d,n_free_h2=%d,n_ol0_h2=%d,n_used_l1=%d\n",
//			jgk_information_data->mac_information_part1.n_used_l0,
//			jgk_information_data->mac_information_part1.n_ol0_hx,
//			jgk_information_data->mac_information_part1.n_free_hx,
//			jgk_information_data->mac_information_part1.n_free_h1,
//			jgk_information_data->mac_information_part1.n_ol0_h1,		
//			jgk_information_data->mac_information_part1.n_free_h2,
//			jgk_information_data->mac_information_part1.n_ol0_h2,			
//			jgk_information_data->mac_information_part1.n_used_l1);
		memcpy((void *)&smsg->mac_information_part1,(void *)&jgk_information_data->mac_information_part1,sizeof(ob_state_part1));
//		for(i = 1; i < NET_SIZE; i++)
//		{
//			printf("i %d link %d\n",i,smsg->mac_information_part1.nbr_list[i]);
//		}
		break;
	}
	}

	//	mgmt_info.id = nla_get_u16(attrs[MGMT_ATTR_NODEID]);
	//	printf("id %d\n",mgmt_info.id);
	//	addr = nla_data(attrs[MGMT_ATTR_VETH_ADDRESS]);
	////	printf("mac %02x %02x %02x %02x %02x %02x\n",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
	//
	//	memcpy(mgmt_info.macaddr,addr,ETH_ALEN);
	//	mgmt_info.txrate = nla_get_u32(attrs[MGMT_ATTR_VETH_TX]);
	//	mgmt_info.rxrate = nla_get_u32(attrs[MGMT_ATTR_VETH_RX]);
	//	mgmt_info.freq = nla_get_u32(attrs[MGMT_ATTR_FREQ]);
	//	mgmt_info.bw = nla_get_u16(attrs[MGMT_ATTR_BW]);
	//	mgmt_info.txpower = nla_get_s16(attrs[MGMT_ATTR_TXPOWER]);
	//	printf("mgmt_netlink_info_callback\n");
	//	jgk_node = nla_data(attrs[MGMT_ATTR_PKTNUMB_ROUTE]);
	//	bat_jgk_node = (struct batadv_jgk_node*)jgk_node;
	//	printf("mgmt_ctrl bat_jgk_node nodeid: %d, s_ogm: %d, r_ogm: %d, f_ogm: %d, s_bcast: %d, r_bcast: %d, f_bcast: %d\n", bat_jgk_node->nodeid,
	//			bat_jgk_node->s_ogm, bat_jgk_node->r_ogm, bat_jgk_node->f_ogm, bat_jgk_node->s_bcast, bat_jgk_node->r_bcast, bat_jgk_node->f_bcast);

	//	croutet = nla_data(attrs[MGMT_ATTR_TABLE_ROUTE]);
	//	routet1 = (struct routetable1*)croutet;
	//	printf("routet %d\n",routet->bat_jgk_route[0].route);
	//	veth_jgk_data = nla_data(attrs[MGMT_ATTR_INFO_VETH]);
	//	jgk_information_data = (jgk_report_infor*)veth_jgk_data;


	//	veth_jgk_data = nla_data(attrs[MGMT_ATTR_INFO_VETH]);
	//	test1 = (jgk_report_infor11*)veth_jgk_data;
	//
	//	veth_jgk_data = nla_data(attrs[MGMT_ATTR_INFO_VETH2]);
	//	test2 = (jgk_report_infor12*)veth_jgk_data;



	//	mgmt_info.txrate = nla_get_u32(attrs[MGMT_ATTR_VETH_TX]);
		//printf("txrate %d\n",mgmt_info.txrate);
		//printf("jgk_information_data %x %d  %x %d\n",(int)test1,test1->enqueue_bytes[0],(int)test2,test2->traffic_queue_information.ucds[0]);

	//	printf("mgmt_ctrl jgk_information_data enqueue_bytes %d outqueue_bytes %d node_id %d tx_inall %d tx_outall %d rx_inall %d rx_outall %d\n",
	//				jgk_information_data->enqueue_bytes[0],jgk_information_data->outqueue_bytes[0],jgk_information_data->mac_information_part1.node_id,
	//				jgk_information_data->traffic_queue_information.tx_inall,jgk_information_data->traffic_queue_information.tx_outall,
	//				jgk_information_data->traffic_queue_information.rx_inall,jgk_information_data->traffic_queue_information.rx_outall);

	return NL_STOP;
}


char* mgmt_netlink_get_info(int ifindex, uint8_t nl_cmd, const char* header, char* remaining)
{
	struct nl_sock* sock;
	struct nl_msg* msg;
	struct nl_cb* cb;
	int family;
	struct print_opts opts = {
		.read_opt = 0,
		.nl_cmd = nl_cmd,
		.remaining_header = remaining,
		.static_header = header,
	};

	sock = nl_socket_alloc();
	if (!sock)
		return NULL;

	genl_connect(sock);

	family = genl_ctrl_resolve(sock, MGMT_NL_NAME);
	if (family < 0) {
		printf("family error\n");
		nl_socket_free(sock);
		return NULL;
	}

	msg = nlmsg_alloc();
	if (!msg) {
		nl_socket_free(sock);
		return NULL;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, 0,
		nl_cmd, 1);

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err_free_sock;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, mgmt_netlink_info_callback, &opts);
	nl_cb_err(cb, NL_CB_CUSTOM, print_error, NULL);

	nla_put_u8(msg, MGMT_ATTR_NODEID, SELFID);
	//printf("mgmt_netlink_get_info id %d\n",SELFID);

	nl_send_auto_complete(sock, msg);

	nlmsg_free(msg);

	nl_recvmsgs(sock, cb);

err_free_sock:
	nl_socket_free(sock);

	return opts.remaining_header;
}

static int mgmt_netlink_param_callback(struct nl_msg* msg, void* arg)
{
	struct nlattr* attrs[MGMT_ATTR_MAX + 1];
	struct nlmsghdr* nlh = nlmsg_hdr(msg);
	struct print_opts* opts = arg;
	const uint8_t* primary_mac;
	struct genlmsghdr* ghdr;
	const uint8_t* mesh_mac;
	const char* primary_if;
	const char* mesh_name;
	const char* version;
	char* extra_info = NULL;
	uint8_t ttvn = 0;
	uint16_t bla_group_id = 0;
	const char* algo_name;
	const char* extra_header;
	int ret;
	int value;

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	ghdr = nlmsg_data(nlh);

	if (ghdr->cmd != MGMT_CMD_SET_PARAM)
		return NL_OK;

	if (nla_parse(attrs, MGMT_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		genlmsg_len(ghdr), mgmt_netlink_policy)) {
		fputs("Received invalid data from kernel.\n", stderr);
		exit(1);
	}

	//set ok
//	printf("mgmt_netlink_param_set ok\n");

	return NL_STOP;
}

//void set_param(uint8_t *buffer,int buflen, const char* header)
//{
//	//测试打印------------------------------
//		printf("调用mgmt_netlink_set_param函数\n");
//		//测试打印------------------------------
//		Smgmt_header* hmsg;
//		Smgmt_set_param* sparam;
//		int paramdata = 0;
//		//	int routeparamdata = 0;
//		//	int virtparamdata = 0;
//		struct nl_sock* sock;
//		struct nl_msg* msg;
//		struct nl_cb* cb;
//		int family;
//		uint8_t cmd[200];
//		char ret = -1;
//		struct print_opts opts = {
//			.read_opt = 0,
//			.nl_cmd = MGMT_CMD_SET_PARAM,
//			.remaining_header = NULL,
//			.static_header = header,
//		};
//		memset(cmd,0,sizeof(cmd));
//		hmsg = (Smgmt_header*)buffer;
//		sparam = (Smgmt_set_param*)hmsg->mgmt_data;
//		//测试打印-------------------------------
//		printf("Smgmt_headerr数据---------------------\n");
//		printf("sizeof(Smgmt_header) = %d\n", sizeof(hmsg));
//		printf("%04x ", hmsg->mgmt_head);
//		printf("%04x ", hmsg->mgmt_len);
//		printf("%04x ", hmsg->mgmt_type);
//		printf("%04x ", hmsg->mgmt_keep);
//		printf("数据内容：\n");
//		printf("%04x ", sparam->mgmt_id);
//		printf("%04x ", sparam->mgmt_route_interval);
//		printf("%04x ", sparam->mgmt_route_ttl);
//		printf("%04x ", sparam->mgmt_virt_queue_num);
//		printf("%04x ", sparam->mgmt_virt_queue_length);
//		printf("%04x ", sparam->mgmt_virt_qos_stategy);
//		printf("%02x ", sparam->mgmt_virt_unicast_mcs);
//		printf("%02x ", sparam->mgmt_virt_multicast_mcs);
//		printf("%02x ", sparam->mgmt_mac_bw);
//		printf("%02x ", sparam->reserved);
//		printf("%08x ", sparam->mgmt_mac_freq);
//		printf("%04x ", sparam->mgmt_mac_txpower);
//		printf("%04x ", sparam->mgmt_mac_work_mode);
//		printf("\n");
//		//测试打印-------------------------------
//
//
//		sock = nl_socket_alloc();
//		if (!sock)
//			return ret;
//
//		genl_connect(sock);
//
//		family = genl_ctrl_resolve(sock, MGMT_NL_NAME);
//		if (family < 0) {
//			printf("family error\n");
//			nl_socket_free(sock);
//			return ret;
//		}
//
//		msg = nlmsg_alloc();
//		if (!msg) {
//			nl_socket_free(sock);
//			return ret;
//		}
//
//		genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, 0,
//			MGMT_CMD_SET_PARAM, 1);
//
//		paramdata = ntohs(hmsg->mgmt_type);
//		//	printf("mgmt_type %d\n",ntohs(hmsg->mgmt_type));
//
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_ID) {
//			//		paramdata |= SET_ID;
//			//		nla_put_u16(msg,MGMT_ATTR_SET_NODEID,ntohs(sparam->mgmt_id));
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_INTERVAL) {
//			printf("设置MGMT_SET_INTERVAL参数:%04x\n", sparam->mgmt_route_interval);
//			//		routeparamdata |= ROUTE_SET_INTERVAL;
//			nla_put_u16(msg, MGMT_ATTR_SET_INTERVAL, ntohs(sparam->mgmt_route_interval));
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_TTL) {
//			printf("设置MGMT_SET_TTL参数:%04x\n", sparam->mgmt_route_ttl);
//			//		routeparamdata |= ROUTE_SET_TTL;
//			nla_put_u16(msg, MGMT_ATTR_SET_TTL, ntohs(sparam->mgmt_route_ttl));
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_QUEUE_NUM) {
//			printf("设置MGMT_SET_QUEUE_NUM参数:%04x\n", sparam->mgmt_virt_queue_num);
//			//		virtparamdata |= VETH_SET_QUEUE_NUM;
//			nla_put_u16(msg, MGMT_ATTR_SET_QUEUE_NUM, ntohs(sparam->mgmt_virt_queue_num));
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_QUEUE_LENGTH) {
//			printf("设置MGMT_SET_QUEUE_LENGTH参数:%04x\n", sparam->mgmt_virt_queue_length);
//			//		virtparamdata |= VETH_SET_QUEUE_LENGTH;
//			nla_put_u16(msg, MGMT_ATTR_SET_QUEUE_LENGTH, ntohs(sparam->mgmt_virt_queue_length));
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_QOS_STATEGY) {
//			printf("设置MGMT_SET_QOS_STATEGY参数:%04x\n", sparam->mgmt_virt_qos_stategy);
//			//		virtparamdata |= VETH_SET_QOS_STATEGY;
//			nla_put_u16(msg, MGMT_ATTR_SET_QOS_STATEGY, ntohs(sparam->mgmt_virt_qos_stategy));
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_UNICAST_MCS) {
//
//
//			printf("设置MGMT_SET_UNICAST_MCS参数%02x\n", sparam->mgmt_virt_unicast_mcs);
//			sprintf(cmd,
//				"sed -i \"s/mcs .*/mcs %d/g\" /etc/node_xwg",
//				sparam->mgmt_virt_unicast_mcs);
//			system(cmd);
//	#ifdef Radio_SWARM_WNW
//	        if(sparam->mgmt_virt_unicast_mcs == 7){
//	           sparam->mgmt_virt_unicast_mcs = 6;
//			}
//	#endif
//			nla_put_u8(msg, MGMT_ATTR_SET_UNICAST_MCS, sparam->mgmt_virt_unicast_mcs);
//			MCS_INIT = sparam->mgmt_virt_unicast_mcs;
//		}
//			memset(cmd,0,sizeof(cmd));
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_MULTICAST_MCS) {
//	#ifdef Radio_SWARM_WNW
//			 if(sparam->mgmt_virt_multicast_mcs == 7){
//	           sparam->mgmt_virt_multicast_mcs = 6;
//			}
//	#endif
//			printf("设置MGMT_SET_MULTICAST_MCS参数%02x\n", sparam->mgmt_virt_multicast_mcs);
//
//			//		virtparamdata |= VETH_SET_MULTICAST_MCS;
//			nla_put_u8(msg, MGMT_ATTR_SET_MULTICAST_MCS, sparam->mgmt_virt_multicast_mcs);
//		}
//			memset(cmd,0,sizeof(cmd));
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_FREQUENCY) {
//			printf("设置MGMT_SET_FREQUENCY参数%08x\n", sparam->mgmt_mac_freq);
//
//			//		virtparamdata |= VETH_SET_FREQUENCY;
//			//		printf("mgmt_freq %d\n",ntohl(sparam->mgmt_mac_freq));
//			sprintf(cmd,
//				"sed -i \"s/channel .*/channel %d/g\" /etc/node_xwg",
//				ntohl(sparam->mgmt_mac_freq));
//			system(cmd);
//			nla_put_u32(msg, MGMT_ATTR_SET_FREQUENCY, ntohl(sparam->mgmt_mac_freq));
//			FREQ_INIT = ntohl(sparam->mgmt_mac_freq);
//		}
//			memset(cmd,0,sizeof(cmd));
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_POWER) {
//			//		virtparamdata |= VETH_SET_POWER;
//			if (ntohs(sparam->mgmt_mac_txpower) < MIN_TXPOWER)
//				sparam->mgmt_mac_txpower = htons(MIN_TXPOWER);
//			sprintf(cmd,
//				"sed -i \"s/power .*/power %d/g\" /etc/node_xwg",
//				ntohs(sparam->mgmt_mac_txpower));
//			system(cmd);
//			nla_put_u16(msg, MGMT_ATTR_SET_POWER, ntohs(sparam->mgmt_mac_txpower));
//			POWER_INIT = ntohs(sparam->mgmt_mac_txpower);
//		}
//			memset(cmd,0,sizeof(cmd));
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_BANDWIDTH) {
//			sprintf(cmd,
//				"sed -i \"s/bw .*/bw %d/g\" /etc/node_xwg",
//				sparam->mgmt_mac_bw);
//			system(cmd);
//			nla_put_u8(msg, MGMT_ATTR_SET_BANDWIDTH, sparam->mgmt_mac_bw);
//			BW_INIT = sparam->mgmt_mac_bw;
//		}
//			memset(cmd,0,sizeof(cmd));
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_TEST_MODE) {
//			if (ntohs(sparam->mgmt_mac_work_mode) < 100) {
//				sprintf(cmd,
//					"sed -i \"s/macmode .*/macmode %d/g\" /etc/node_xwg",
//					ntohs(sparam->mgmt_mac_work_mode));
//				system(cmd);
//			}
//			nla_put_u16(msg, MGMT_ATTR_SET_TEST_MODE, ntohs(sparam->mgmt_mac_work_mode));
//			MACMODE_INIT = ntohs(sparam->mgmt_mac_work_mode);
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_TEST_MODE_MCS) {
//			//		virtparamdata |= VETH_SET_TEST_MODE_MCS;
//			nla_put_u16(msg, MGMT_ATTR_SET_TEST_MODE_MCS, ntohs(sparam->mgmt_mac_work_mode));
//		}
//
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_PHY) {
//			//		virtparamdata |= VETH_SET_TEST_MODE_MCS;
//
//
//			sparam->mgmt_phy.phy_pre_STS_thresh = ntohs(sparam->mgmt_phy.phy_pre_STS_thresh);
//			sparam->mgmt_phy.phy_pre_LTS_thresh = ntohs(sparam->mgmt_phy.phy_pre_LTS_thresh);
//			sparam->mgmt_phy.phy_tx_iq0_scale = ntohs(sparam->mgmt_phy.phy_tx_iq0_scale);
//			sparam->mgmt_phy.phy_tx_iq1_scale = ntohs(sparam->mgmt_phy.phy_tx_iq1_scale);
//
//		#ifdef Radio_SWARM_S2
//			sprintf(cmd,
//				"sed -i \"s/phymsg .*/phymsg %d %d %d %d %d %d %d %d %d/g\" /etc/node_xwg",
//				sparam->mgmt_phy.rf_agc_framelock_en, sparam->mgmt_phy.phy_cfo_bypass_en,
//				sparam->mgmt_phy.phy_pre_STS_thresh, sparam->mgmt_phy.phy_pre_LTS_thresh,
//				sparam->mgmt_phy.phy_tx_iq0_scale, sparam->mgmt_phy.phy_tx_iq1_scale,
//				sparam->mgmt_phy.phy_msc_length_mode,sparam->mgmt_phy.phy_sfbc_en,
//				sparam->mgmt_phy.phy_cdd_num);
//		#else
//				sprintf(cmd,
//				"sed -i \"s/phymsg .*/phymsg %d %d %d %d %d %d %d %d %d/g\" /etc/node_xwg",
//				sparam->mgmt_phy.rf_agc_framelock_en, sparam->mgmt_phy.phy_cfo_bypass_en,
//				sparam->mgmt_phy.phy_pre_STS_thresh, sparam->mgmt_phy.phy_pre_LTS_thresh,
//				sparam->mgmt_phy.phy_tx_iq0_scale, sparam->mgmt_phy.phy_tx_iq1_scale);
//		#endif
//			system(cmd);
//			nla_put(msg, MGMT_ATTR_SET_PHY, sizeof(Smgmt_phy), &(sparam->mgmt_phy));
//		}
//	#ifdef Radio_CEC
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_NCU) {
//			sprintf(cmd,
//				"sed -i \"s/ncunodeid.*/ncunodeid %d/g\" /etc/node_xwg",
//				sparam->mgmt_NCU_node_id);
//			system(cmd);
//			nla_put_u8(msg, MGMT_ATTR_SET_NCU, sparam->mgmt_NCU_node_id);
//			NCU_NODE_ID_INIT = sparam->mgmt_NCU_node_id;
//		}
//		if (ntohs(hmsg->mgmt_type) & MGMT_SET_WORKMODE) {
//			sprintf(cmd,
//				"sed -i \"s/networkmode.*/networkmode %d/g\" /etc/node_xwg",
//				sparam->mgmt_net_work_mode);
//			system(cmd);
//			nla_put_u8(msg, MGMT_ATTR_SET_WORKMODE, sparam->mgmt_net_work_mode);
//			NET_WORKMOD_INIT = sparam->mgmt_net_work_mode;
//		}
//
//
//	#endif
//
//		nla_put_u32(msg, MGMT_ATTR_SET_PARAM, paramdata);
//
//		nl_send_auto_complete(sock, msg);
//
//		nlmsg_free(msg);
//
//		cb = nl_cb_alloc(NL_CB_DEFAULT);
//		if (!cb)
//			goto err_free_sock;
//
//		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, mgmt_netlink_param_callback, &opts);
//		nl_cb_err(cb, NL_CB_CUSTOM, print_error, NULL);
//
//		nl_recvmsgs(sock, cb);
//		ret = 0;
//	err_free_sock:
//		nl_socket_free(sock);
//		return ret;
//}

char mgmt_netlink_set_param(char* buffer, int buflen, const char* header)
{
	//测试打印------------------------------
	printf("调用mgmt_netlink_set_param函数\n");
	//测试打印------------------------------
	Smgmt_header* hmsg;
	Smgmt_set_param* sparam;
	int paramdata = 0;
	//	int routeparamdata = 0;
	//	int virtparamdata = 0;
	struct nl_sock* sock;
	struct nl_msg* msg;
	struct nl_cb* cb;
	int family;
	uint8_t cmd[200];
	char ret = -1;
	int i,j;
	int row_cnt;
	struct print_opts opts = {
		.read_opt = 0,
		.nl_cmd = MGMT_CMD_SET_PARAM,
		.remaining_header = NULL,
		.static_header = header,
	};


	memset(cmd,0,sizeof(cmd));
	hmsg = (Smgmt_header*)buffer;
	sparam = (Smgmt_set_param*)hmsg->mgmt_data;
	//测试打印-------------------------------
	printf("Smgmt_headerr数据---------------------\n");
//	printf("sizeof(Smgmt_header) = %d\n", sizeof(hmsg));
	printf("%04x ", hmsg->mgmt_head);
	printf("%04x ", hmsg->mgmt_len);
	printf("%04x ", hmsg->mgmt_type);
	printf("%04x ", hmsg->mgmt_keep);
	printf("数据内容：\n");
	printf("%04x ", sparam->mgmt_id);
	printf("%04x ", sparam->mgmt_route_interval);
	printf("%04x ", sparam->mgmt_route_ttl);
	printf("%04x ", sparam->mgmt_virt_queue_num);
	printf("%04x ", sparam->mgmt_virt_queue_length);
	printf("%04x ", sparam->mgmt_virt_qos_stategy);
	printf("%02x ", sparam->mgmt_virt_unicast_mcs);
	printf("%02x ", sparam->mgmt_virt_multicast_mcs);
	printf("%02x ", sparam->mgmt_mac_bw);
	printf("%02x ", sparam->reserved);
	printf("%08x ", sparam->mgmt_mac_freq);
	printf("%04x ", sparam->mgmt_mac_txpower);
	printf("%04x ", sparam->mgmt_mac_work_mode);
	printf("%02x ", sparam->mgmt_net_work_mode);
	printf("%02x ", sparam->u8Slotlen);
	printf("\n");
	//测试打印-------------------------------


	sock = nl_socket_alloc();
	if (!sock)
		return ret;

	genl_connect(sock);

	family = genl_ctrl_resolve(sock, MGMT_NL_NAME);
	if (family < 0) {
		printf("family error\n");
		nl_socket_free(sock);
		return ret;
	}

	msg = nlmsg_alloc();
	if (!msg) {
		nl_socket_free(sock);
		return ret;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, 0,
		MGMT_CMD_SET_PARAM, 1);

	paramdata = ntohs(hmsg->mgmt_type);
	//printf("Smgmt_header paramdata = %#x\n",paramdata);
	if(ntohs(hmsg->mgmt_keep) & MGMT_SET_SLOTLEN)
	{
		paramdata |= MGMT_SET_SLOTLEN << 16;
		//printf("Smgmt_header paramdata = %#x\n",paramdata);
	}
	if(ntohs(hmsg->mgmt_keep) & MGMT_SET_POWER_LEVEL)
	{
		paramdata |= MGMT_SET_POWER_LEVEL << 16;
	}
	if(ntohs(hmsg->mgmt_keep) & MGMT_SET_POWER_ATTENUATION)
	{
		paramdata |= MGMT_SET_POWER_ATTENUATION << 16;
	}
	if(ntohs(hmsg->mgmt_keep) & MGMT_SET_RX_CHANNEL_MODE)
	{
		paramdata |= MGMT_SET_RX_CHANNEL_MODE << 16;
	}

	
	//	printf("mgmt_type %d\n",ntohs(hmsg->mgmt_type));

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_ID) {
		//		paramdata |= SET_ID;
		//		nla_put_u16(msg,MGMT_ATTR_SET_NODEID,ntohs(sparam->mgmt_id));
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_INTERVAL) {
		printf("设置MGMT_SET_INTERVAL参数:%04x\n", sparam->mgmt_route_interval);
		//		routeparamdata |= ROUTE_SET_INTERVAL;
		nla_put_u16(msg, MGMT_ATTR_SET_INTERVAL, ntohs(sparam->mgmt_route_interval));
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_TTL) {
		printf("设置MGMT_SET_TTL参数:%04x\n", sparam->mgmt_route_ttl);
		//		routeparamdata |= ROUTE_SET_TTL;
		nla_put_u16(msg, MGMT_ATTR_SET_TTL, ntohs(sparam->mgmt_route_ttl));
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_QUEUE_NUM) {
		printf("设置MGMT_SET_QUEUE_NUM参数:%04x\n", sparam->mgmt_virt_queue_num);
		//		virtparamdata |= VETH_SET_QUEUE_NUM;
		nla_put_u16(msg, MGMT_ATTR_SET_QUEUE_NUM, ntohs(sparam->mgmt_virt_queue_num));
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_QUEUE_LENGTH) {
		printf("设置MGMT_SET_QUEUE_LENGTH参数:%04x\n", sparam->mgmt_virt_queue_length);
		//		virtparamdata |= VETH_SET_QUEUE_LENGTH;
		nla_put_u16(msg, MGMT_ATTR_SET_QUEUE_LENGTH, ntohs(sparam->mgmt_virt_queue_length));
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_QOS_STATEGY) {
		printf("设置MGMT_SET_QOS_STATEGY参数:%04x\n", sparam->mgmt_virt_qos_stategy);
		//		virtparamdata |= VETH_SET_QOS_STATEGY;
		nla_put_u16(msg, MGMT_ATTR_SET_QOS_STATEGY, ntohs(sparam->mgmt_virt_qos_stategy));
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_UNICAST_MCS) {


		printf("设置MGMT_SET_UNICAST_MCS参数%02x\n", sparam->mgmt_virt_unicast_mcs);
		sprintf(cmd,
			"sed -i \"s/mcs .*/mcs %d/g\" /etc/node_xwg",
			sparam->mgmt_virt_unicast_mcs);
		system(cmd);
#ifdef Radio_SWARM_WNW
        if(sparam->mgmt_virt_unicast_mcs == 7){
           sparam->mgmt_virt_unicast_mcs = 6;
		}
#endif
		nla_put_u8(msg, MGMT_ATTR_SET_UNICAST_MCS, sparam->mgmt_virt_unicast_mcs);
		MCS_INIT = sparam->mgmt_virt_unicast_mcs;
	}

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_MULTICAST_MCS) {
#ifdef Radio_SWARM_WNW
		 if(sparam->mgmt_virt_multicast_mcs == 7){
           sparam->mgmt_virt_multicast_mcs = 6;
		}
#endif
		printf("设置MGMT_SET_MULTICAST_MCS参数%02x\n", sparam->mgmt_virt_multicast_mcs);

		//		virtparamdata |= VETH_SET_MULTICAST_MCS;
		nla_put_u8(msg, MGMT_ATTR_SET_MULTICAST_MCS, sparam->mgmt_virt_multicast_mcs);
	}
		if (ntohs(hmsg->mgmt_type) & MGMT_SET_WORKMODE) {
		// 1表示定频，2表示跳频，3表示认知跳频
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,
			"sed -i \"s/networkmode.*/networkmode %d/g\" /etc/node_xwg",
			sparam->mgmt_net_work_mode.NET_work_mode);
		
		system(cmd);
//		nla_put_u8(msg, MGMT_ATTR_SET_WORKMODE, sparam->mgmt_net_work_mode);
		printf("设置工作模式为%d\n",sparam->mgmt_net_work_mode.NET_work_mode);
		NET_WORKMOD_INIT = sparam->mgmt_net_work_mode.NET_work_mode;
//		for(i=0;i<32;i++)
//		{
//			HOP_FREQ_TB_INIT[i] = 1400 + i*50;
//		}
		
		if(sparam->mgmt_net_work_mode.NET_work_mode == HOP_FREQ_MODE) //跳频模式
		{
			memcpy((void *)sparam->mgmt_net_work_mode.hop_freq_tb,(void *)HOP_FREQ_TB_INIT,sizeof(HOP_FREQ_TB_INIT));

			row_cnt = 0;
			for(i=0;i<4;i++)
			{
				row_cnt++;
				memset(cmd,0,sizeof(cmd));
				sprintf(cmd,
					"sed -i \"%ds/.*/%d %d %d %d %d %d %d %d /g\" /etc/node_hop",
					row_cnt,
					HOP_FREQ_TB_INIT[i*8], HOP_FREQ_TB_INIT[i*8+1],
					HOP_FREQ_TB_INIT[i*8+2], HOP_FREQ_TB_INIT[i*8+3],
					HOP_FREQ_TB_INIT[i*8+4], HOP_FREQ_TB_INIT[i*8+5],
					HOP_FREQ_TB_INIT[i*8+6], HOP_FREQ_TB_INIT[i*8+7]);
				system(cmd);
			}
			sparam->mgmt_net_work_mode.fh_len = 0;
			for(i = 0 ; i < HOP_FREQ_NUM ; i ++)
			{
				if(0 == HOP_FREQ_TB_INIT[i])
				{
					break;
				}
				else
				{
					sparam->mgmt_net_work_mode.fh_len ++;
				}
			}
		}
		else if(sparam->mgmt_net_work_mode.NET_work_mode == FIX_FREQ_MODE)
		{
			if((ntohs(hmsg->mgmt_type) & MGMT_SET_FREQUENCY) == 0)
			{
				nla_put_u32(msg, MGMT_ATTR_SET_FREQUENCY, FREQ_INIT);
				paramdata |= MGMT_SET_FREQUENCY;
			}
		}

		nla_put(msg, MGMT_ATTR_SET_WORKMODE, sizeof(Smgmt_net_work_mode), (void *)&(sparam->mgmt_net_work_mode));
	}

	if ((ntohs(hmsg->mgmt_type) & MGMT_SET_FREQUENCY) && (NET_WORKMOD_INIT == FIX_FREQ_MODE)) {
		memset(cmd,0,sizeof(cmd));
		printf("设置MGMT_SET_FREQUENCY参数--%d\n", ntohl(sparam->mgmt_mac_freq));



		//		virtparamdata |= VETH_SET_FREQUENCY;
		//		printf("mgmt_freq %d\n",ntohl(sparam->mgmt_mac_freq));
		sprintf(cmd,
			"sed -i \"s/channel .*/channel %d/g\" /etc/node_xwg",
			ntohl(sparam->mgmt_mac_freq));
		system(cmd);
		nla_put_u32(msg, MGMT_ATTR_SET_FREQUENCY, ntohl(sparam->mgmt_mac_freq));
		FREQ_INIT = ntohl(sparam->mgmt_mac_freq);
	}

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_POWER) {
		//		virtparamdata |= VETH_SET_POWER;
		memset(cmd,0,sizeof(cmd));

		if (ntohs(sparam->mgmt_mac_txpower) < MIN_TXPOWER)
			sparam->mgmt_mac_txpower = htons(MIN_TXPOWER);
		if (ntohs(sparam->mgmt_mac_txpower) > MAX_TXPOWER)
			sparam->mgmt_mac_txpower = htons(MAX_TXPOWER);
		printf("设置MGMT_SET_POWER参数--%d\n", ntohs(sparam->mgmt_mac_txpower));

		sprintf(cmd,
			"sed -i \"s/power .*/power %d/g\" /etc/node_xwg",
			ntohs(sparam->mgmt_mac_txpower));
		system(cmd);
		uint16_t txpower_payload[POWER_CHANNEL_NUM];
		bool has_per_channel = false;
		for (int idx = 0; idx < POWER_CHANNEL_NUM; ++idx) {
			if (sparam->mgmt_mac_txpower_ch[idx] != 0) {
				has_per_channel = true;
			}
			txpower_payload[idx] = ntohs(sparam->mgmt_mac_txpower_ch[idx]);
		}
		if (!has_per_channel) {
			txpower_lookup_channels(ntohs(sparam->mgmt_mac_txpower), txpower_payload);
		}
		nla_put(msg, MGMT_ATTR_SET_POWER, sizeof(txpower_payload), txpower_payload);
		POWER_INIT = ntohs(sparam->mgmt_mac_txpower);
	}

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_BANDWIDTH) {
		memset(cmd,0,sizeof(cmd));
		if(sparam->mgmt_mac_bw<=MIN_BW)
		{
			sparam->mgmt_mac_bw=MIN_BW;
		}
		if(sparam->mgmt_mac_bw>=MAX_BW)
		{
			sparam->mgmt_mac_bw=MAX_BW;
		}
		printf("MGMT_SET_BANDWIDTH参数--%d\n", sparam->mgmt_mac_bw);

		sprintf(cmd,
			"sed -i \"s/bw .*/bw %d/g\" /etc/node_xwg",
			sparam->mgmt_mac_bw);
		system(cmd);
		nla_put_u8(msg, MGMT_ATTR_SET_BANDWIDTH, sparam->mgmt_mac_bw);
		BW_INIT = sparam->mgmt_mac_bw;
	}

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_TEST_MODE) {
		memset(cmd,0,sizeof(cmd));
		printf("MGMT_SET_TEST_MODE参数--%d\n", sparam->mgmt_mac_work_mode);
		if (ntohs(sparam->mgmt_mac_work_mode) < 100) {
			sprintf(cmd,
				"sed -i \"s/macmode .*/macmode %d/g\" /etc/node_xwg",
				ntohs(sparam->mgmt_mac_work_mode));
			system(cmd);
		}
		nla_put_u16(msg, MGMT_ATTR_SET_TEST_MODE, ntohs(sparam->mgmt_mac_work_mode));
		MACMODE_INIT = ntohs(sparam->mgmt_mac_work_mode);
	}
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_TEST_MODE_MCS) {
		//		virtparamdata |= VETH_SET_TEST_MODE_MCS;
		nla_put_u16(msg, MGMT_ATTR_SET_TEST_MODE_MCS, ntohs(sparam->mgmt_mac_work_mode));
	}

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_PHY) {
		//		virtparamdata |= VETH_SET_TEST_MODE_MCS;

		memset(cmd,0,sizeof(cmd));
		sparam->mgmt_phy.phy_pre_STS_thresh = ntohs(sparam->mgmt_phy.phy_pre_STS_thresh);
		sparam->mgmt_phy.phy_pre_LTS_thresh = ntohs(sparam->mgmt_phy.phy_pre_LTS_thresh);
		sparam->mgmt_phy.phy_tx_iq0_scale = ntohs(sparam->mgmt_phy.phy_tx_iq0_scale);
		sparam->mgmt_phy.phy_tx_iq1_scale = ntohs(sparam->mgmt_phy.phy_tx_iq1_scale);

	#ifdef Radio_SWARM_S2
		sprintf(cmd,
			"sed -i \"s/phymsg .*/phymsg %d %d %d %d %d %d %d %d %d/g\" /etc/node_xwg",
			sparam->mgmt_phy.rf_agc_framelock_en, sparam->mgmt_phy.phy_cfo_bypass_en,
			sparam->mgmt_phy.phy_pre_STS_thresh, sparam->mgmt_phy.phy_pre_LTS_thresh,
			sparam->mgmt_phy.phy_tx_iq0_scale, sparam->mgmt_phy.phy_tx_iq1_scale,
			sparam->mgmt_phy.phy_msc_length_mode,sparam->mgmt_phy.phy_sfbc_en,
			sparam->mgmt_phy.phy_cdd_num);
	#else 
			sprintf(cmd,
			"sed -i \"s/phymsg .*/phymsg %d %d %d %d %d %d %d %d %d/g\" /etc/node_xwg",
			sparam->mgmt_phy.rf_agc_framelock_en, sparam->mgmt_phy.phy_cfo_bypass_en,
			sparam->mgmt_phy.phy_pre_STS_thresh, sparam->mgmt_phy.phy_pre_LTS_thresh,
			sparam->mgmt_phy.phy_tx_iq0_scale, sparam->mgmt_phy.phy_tx_iq1_scale);
	#endif
		system(cmd);
		nla_put(msg, MGMT_ATTR_SET_PHY, sizeof(Smgmt_phy), &(sparam->mgmt_phy));
	}

	if (ntohs(hmsg->mgmt_type) & MGMT_SET_IQ_CATCH) {
		// set iq
		nla_put(msg, MGMT_SET_IQ_CATCH, sizeof(Smgmt_IQ_Catch),&(sparam->mgmt_mac_iq_catch));
	}
	if (ntohs(hmsg->mgmt_keep) & MGMT_SET_SLOTLEN) {
		printf("MGMT_SET_SLOTLEN:%#x\n", sparam->u8Slotlen);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,
			"sed -i \"s/slotlen .*/slotlen %d/g\" /etc/node_xwg",
			sparam->u8Slotlen);
		system(cmd);
		nla_put_u8(msg, MGMT_ATTR_SET_SLOTLEN, sparam->u8Slotlen);
		
	}
	if (ntohs(hmsg->mgmt_keep) & MGMT_SET_POWER_LEVEL) {
		printf("MGMT_SET_POWER_LEVEL:%#x\n", sparam->mgmt_mac_power_level);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,
			"sed -i \"s/power_level .*/power_level %d/g\" /etc/node_xwg",
			sparam->mgmt_mac_power_level);
		system(cmd);
		nla_put_u8(msg, MGMT_ATTR_SET_POWER_LEVEL, sparam->mgmt_mac_power_level);
	}
	if (ntohs(hmsg->mgmt_keep) & MGMT_SET_POWER_ATTENUATION) {
		printf("MGMT_SET_POWER_ATTENUATION:%#x\n", sparam->mgmt_mac_power_attenuation);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,
			"sed -i \"s/power_attenuation .*/power_attenuation %d/g\" /etc/node_xwg",
			sparam->mgmt_mac_power_attenuation);
		system(cmd);
		nla_put_u8(msg, MGMT_ATTR_SET_POWER_ATTENUATION, sparam->mgmt_mac_power_attenuation);
	}
	if (ntohs(hmsg->mgmt_keep) & MGMT_SET_RX_CHANNEL_MODE) {
		printf("MGMT_SET_RX_CHANNEL_MODE:%#x\n", sparam->mgmt_rx_channel_mode);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,
			"sed -i \"s/rx_channel_mode .*/rx_channel_mode %d/g\" /etc/node_xwg",
			sparam->mgmt_rx_channel_mode);
		system(cmd);
		nla_put_u8(msg, MGMT_ATTR_SET_RX_CHANNEL_MODE, sparam->mgmt_rx_channel_mode);
		RX_CHANNEL_MODE_INIT = sparam->mgmt_rx_channel_mode;
	}
#ifdef Radio_CEC
	if (ntohs(hmsg->mgmt_type) & MGMT_SET_NCU) {
		sprintf(cmd,
			"sed -i \"s/ncunodeid.*/ncunodeid %d/g\" /etc/node_xwg",
			sparam->mgmt_NCU_node_id);
		system(cmd);
		nla_put_u8(msg, MGMT_ATTR_SET_NCU, sparam->mgmt_NCU_node_id);
		NCU_NODE_ID_INIT = sparam->mgmt_NCU_node_id;
	}


#endif
	//	switch (ntohs(hmsg->mgmt_type)) {
	//	case MGMT_SET_ID: {mgmt_netlink_set_param
	//		paramdata |= SET_ID;
	//		nla_put_u16(msg,MGMT_ATTR_SET_NODEID,ntohs(sparam->mgmt_id));
	//		break;
	//	}
	//	case MGMT_SET_FREQ: {
	//		paramdata |= SET_FREQ;
	//		nla_put_u32(msg,MGMT_ATTR_SET_FREQ,ntohl(sparam->mgmt_freq));
	//		break;
	//	}
	//	case MGMT_SET_BW: {
	//		paramdata |= SET_BW;
	//		nla_put_u16(msg,MGMT_ATTR_SET_BW,ntohs(sparam->mgmt_bw));
	//		break;
	//	}
	//	case MGMT_SET_TXPOWER: {
	//		paramdata |= SET_TXPOWER;
	//		nla_put_s16(msg,MGMT_ATTR_SET_TXPOWER,ntohs(sparam->mgmt_txpower));
	//		break;
	//	}
	//	default:{
	//		break;
	//	}
	//	}

	// nla_put_u32(msg, MGMT_ATTR_SET_PARAM, paramdata);

	// nl_send_auto_complete(sock, msg);

	if (nla_put_u32(msg, MGMT_ATTR_SET_PARAM, paramdata) < 0) {
		printf("ERROR: 无法添加PARAM属性\n");
		nlmsg_free(msg);
		nl_socket_free(sock);
		return -1;
	}

	// 检查消息发送
	int send_ret = nl_send_auto_complete(sock, msg);
	if (send_ret < 0) {
		printf("ERROR: nl_send_auto_complete失败: %d\n", send_ret);
		nlmsg_free(msg);
		nl_socket_free(sock);
		return -1;
	}

	nlmsg_free(msg);

	system("sync");

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
	{
	    printf("ERROR: nl_cb_alloc失败\n");
		nl_socket_free(sock);
		return -1;	
	}
		// goto err_free_sock;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, mgmt_netlink_param_callback, &opts);
	nl_cb_err(cb, NL_CB_CUSTOM, print_error, NULL);

	nl_recvmsgs(sock, cb);

	nl_cb_put(cb);
// 	ret = 0;
// err_free_sock:
// 	nl_socket_free(sock);
	return 0;
}

char mgmt_netlink_set_param_wg(char* buffer, int buflen, const char* header,int type)
{
	//test printf
	printf("调用mgmt_netlink_set_param_wg函数\n");
	printf("buffer长度buflen为:%d\n", buflen);

	//	Smgmt_header *hmsg;
	Smgmt_param* sparam;
	Smgmt_param sparam_for_set;
	int paramdata = 0;
	//	int routeparamdata = 0;
	//	int virtparamdata = 0;
	struct nl_sock* sock;
	struct nl_msg* msg;
	struct nl_cb* cb;
	int family;
	uint8_t cmd[200];
	char ret = -1;
	struct print_opts opts = {
		.read_opt = 0,
		.nl_cmd = MGMT_CMD_SET_PARAM,
		.remaining_header = NULL,
		.static_header = header,
	};

	//	hmsg = (Smgmt_header*)buffer;
	//	sparam = (Smgmt_set_param*)hmsg->mgmt_data;
		//sparam = (Smgmt_param*)buffer;

		//modify by yang
		//方式一：有问题,sparam里的数据都是0

	memset(&sparam_for_set, 0, sizeof(sparam_for_set));
	if(type == MGMT_SET_PARAM)
		{
			memcpy((uint8_t*)&sparam_for_set, buffer , buflen);
		}
	else if(type == MGMT_MULTIPOINT_SET)
		{
			memcpy((uint8_t*)&sparam_for_set + sizeof(uint32_t) + sizeof(uint16_t), buffer, buflen);
		}
	//memcpy(&sparam_for_set + sizeof(uint32_t) + sizeof(uint16_t),buffer + sizeof(uint32_t),buflen - sizeof(uint16_t)*4);//
	sparam = &sparam_for_set;

	//方式二
/*
	int data_size = buflen - sizeof(uint16_t)*4;
	char sparam_copybuff[data_size] = 0;
	memcpy(sparam_copybuff,buff + sizeof(uint32_t),sizeof(data_size));
*/

//测试打印---------------------------------------------------------------
	printf("数据内容：\n");
	printf("sparam->mgmt_ip 08x:%08x \n", sparam->mgmt_ip);
	printf("sparam->mgmt_id 04x:%04x \n", sparam->mgmt_id);
	printf("sparam->mgmt_route_interval 04x:%04x \n", sparam->mgmt_route_interval);
	printf("sparam->mgmt_route_ttl 04x:%04x\n ", sparam->mgmt_route_ttl);
	printf("param->mgmt_virt_queue_num 04x:%04x \n", sparam->mgmt_virt_queue_num);
	printf("sparam->mgmt_virt_queue_length 04x:%04x\n ", sparam->mgmt_virt_queue_length);
	printf("sparam->mgmt_virt_qos_stateg 04x:%04x \n", sparam->mgmt_virt_qos_stategy);
	printf("sparam->mgmt_virt_unicast_mcs 02x:%02x \n", sparam->mgmt_virt_unicast_mcs);
	printf("sparam->mgmt_virt_multicast_mcs 02x:%02x \n", sparam->mgmt_virt_multicast_mcs);
	printf("sparam->mgmt_mac_bw 02x:%02x \n", sparam->mgmt_mac_bw);
	printf("sparam->reserved 02x:%02x \n", sparam->reserved);
	printf("sparam->mgmt_mac_freq 08x:%08x \n", sparam->mgmt_mac_freq);
	printf("sparam->mgmt_mac_txpower 04x:%04x \n", sparam->mgmt_mac_txpower);
	printf("sparam->mgmt_mac_work_mode 04x:%04x \n", sparam->mgmt_mac_work_mode);
	printf("\n");
	//测试打印---------------------------------------------------------------

	sock = nl_socket_alloc();
	if (!sock)
		return ret;

	genl_connect(sock);

	family = genl_ctrl_resolve(sock, MGMT_NL_NAME);
	if (family < 0) {
		printf("family error\n");
		nl_socket_free(sock);
		return ret;
	}

	msg = nlmsg_alloc();
	if (!msg) {
		nl_socket_free(sock);
		return ret;
	}

	if(!genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, 0,
		MGMT_CMD_SET_PARAM, 1));
	{
		printf("ERROR: genlmsg_put失败\n");
		nlmsg_free(msg);
		nl_socket_free(sock);
		return -1;		
	}	
	//	paramdata = ntohs(hmsg->mgmt_type);
	//	printf("mgmt_type %d\n",ntohs(hmsg->mgmt_type));

	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_ID){
	//		paramdata |= SET_ID;
	//		nla_put_u16(msg,MGMT_ATTR_SET_NODEID,ntohs(sparam->mgmt_id));
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_INTERVAL){
	//		routeparamdata |= ROUTE_SET_INTERVAL;
	nla_put_u16(msg, MGMT_ATTR_SET_INTERVAL, ntohs(sparam->mgmt_route_interval));
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_TTL){
	//		routeparamdata |= ROUTE_SET_TTL;
	nla_put_u16(msg, MGMT_ATTR_SET_TTL, ntohs(sparam->mgmt_route_ttl));
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_QUEUE_NUM){
	//		virtparamdata |= VETH_SET_QUEUE_NUM;
	nla_put_u16(msg, MGMT_ATTR_SET_QUEUE_NUM, ntohs(sparam->mgmt_virt_queue_num));
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_QUEUE_LENGTH){
	//		virtparamdata |= VETH_SET_QUEUE_LENGTH;
	nla_put_u16(msg, MGMT_ATTR_SET_QUEUE_LENGTH, ntohs(sparam->mgmt_virt_queue_length));
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_QOS_STATEGY){
	//		virtparamdata |= VETH_SET_QOS_STATEGY;
	nla_put_u16(msg, MGMT_ATTR_SET_QOS_STATEGY, ntohs(sparam->mgmt_virt_qos_stategy));
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_UNICAST_MCS){
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,
		"sed -i \"s/mcs .*/mcs %d/g\" /etc/node_xwg",
		sparam->mgmt_virt_unicast_mcs);
	system(cmd);
	nla_put_u8(msg, MGMT_ATTR_SET_UNICAST_MCS, sparam->mgmt_virt_unicast_mcs);
	MCS_INIT = sparam->mgmt_virt_unicast_mcs;
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_MULTICAST_MCS){
	//		virtparamdata |= VETH_SET_MULTICAST_MCS;
	nla_put_u8(msg, MGMT_ATTR_SET_MULTICAST_MCS, sparam->mgmt_virt_multicast_mcs);
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_FREQUENCY){
	//		virtparamdata |= VETH_SET_FREQUENCY;
	//		printf("mgmt_freq %d\n",ntohl(sparam->mgmt_mac_freq));
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,
		"sed -i \"s/channel .*/channel %d/g\" /etc/node_xwg",
		ntohl(sparam->mgmt_mac_freq));
	system(cmd);
	nla_put_u32(msg, MGMT_ATTR_SET_FREQUENCY, ntohl(sparam->mgmt_mac_freq));
	FREQ_INIT = ntohl(sparam->mgmt_mac_freq);
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_POWER){
	//		virtparamdata |= VETH_SET_POWER;
	if (ntohs(sparam->mgmt_mac_txpower) < MIN_TXPOWER)
		sparam->mgmt_mac_txpower = htons(MIN_TXPOWER);
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,
		"sed -i \"s/power .*/power %d/g\" /etc/node_xwg",
		ntohs(sparam->mgmt_mac_txpower));
	system(cmd);
	uint16_t txpower_payload_wg[POWER_CHANNEL_NUM];
	bool has_per_channel_wg = false;
	for (int idx = 0; idx < POWER_CHANNEL_NUM; ++idx) {
		if (sparam->mgmt_mac_txpower_ch[idx] != 0) {
			has_per_channel_wg = true;
		}
		txpower_payload_wg[idx] = ntohs(sparam->mgmt_mac_txpower_ch[idx]);
	}
	if (!has_per_channel_wg) {
		txpower_lookup_channels(ntohs(sparam->mgmt_mac_txpower), txpower_payload_wg);
	}
	nla_put(msg, MGMT_ATTR_SET_POWER, sizeof(txpower_payload_wg), txpower_payload_wg);
	POWER_INIT = ntohs(sparam->mgmt_mac_txpower);
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_BANDWIDTH){
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,
		"sed -i \"s/bw .*/bw %d/g\" /etc/node_xwg",
		sparam->mgmt_mac_bw);
	system(cmd);
	nla_put_u8(msg, MGMT_ATTR_SET_BANDWIDTH, sparam->mgmt_mac_bw);
	BW_INIT = sparam->mgmt_mac_bw;
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_TEST_MODE){
	memset(cmd,0,sizeof(cmd));
	if (ntohs(sparam->mgmt_mac_work_mode) < 100) {
		sprintf(cmd,
			"sed -i \"s/macmode .*/macmode %d/g\" /etc/node_xwg",
			ntohs(sparam->mgmt_mac_work_mode));
		system(cmd);
	}
	nla_put_u16(msg, MGMT_ATTR_SET_TEST_MODE, ntohs(sparam->mgmt_mac_work_mode));
	MACMODE_INIT = ntohs(sparam->mgmt_mac_work_mode);
	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_TEST_MODE_MCS){
	//		virtparamdata |= VETH_SET_TEST_MODE_MCS;
	nla_put_u16(msg, MGMT_ATTR_SET_TEST_MODE_MCS, ntohs(sparam->mgmt_mac_work_mode));



	//	}
	//	if(ntohs(hmsg->mgmt_type) & MGMT_SET_PHY){
	//		virtparamdata |= VETH_SET_TEST_MODE_MCS;
	//		sparam->mgmt_phy.phy_pre_STS_thresh = ntohs(sparam->mgmt_phy.phy_pre_STS_thresh);
	//		sparam->mgmt_phy.phy_pre_LTS_thresh = ntohs(sparam->mgmt_phy.phy_pre_LTS_thresh);
	//		sparam->mgmt_phy.phy_tx_iq0_scale = ntohs(sparam->mgmt_phy.phy_tx_iq0_scale);
	//		sparam->mgmt_phy.phy_tx_iq1_scale = ntohs(sparam->mgmt_phy.phy_tx_iq1_scale);
	//		sprintf(cmd,
	//				"sed -i \"s/phymsg .*/phymsg %d %d %d %d %d %d/g\" /etc/node_xwg",
	//				sparam->mgmt_phy.rf_agc_framelock_en,sparam->mgmt_phy.phy_cfo_bypass_en,
	//				sparam->mgmt_phy.phy_pre_STS_thresh,sparam->mgmt_phy.phy_pre_LTS_thresh,
	//				sparam->mgmt_phy.phy_tx_iq0_scale,sparam->mgmt_phy.phy_tx_iq1_scale);
	//		system(cmd);
	//		nla_put(msg,MGMT_ATTR_SET_PHY,sizeof(Smgmt_phy),&(sparam->mgmt_phy));
	//	}

	//	switch (ntohs(hmsg->mgmt_type)) {
	//	case MGMT_SET_ID: {mgmt_netlink_set_param
	//		paramdata |= SET_ID;
	//		nla_put_u16(msg,MGMT_ATTR_SET_NODEID,ntohs(sparam->mgmt_id));
	//		break;
	//	}
	//	case MGMT_SET_FREQ: {
	//		paramdata |= SET_FREQ;
	//		nla_put_u32(msg,MGMT_ATTR_SET_FREQ,ntohl(sparam->mgmt_freq));
	//		break;
	//	}
	//	case MGMT_SET_BW: {
	//		paramdata |= SET_BW;
	//		nla_put_u16(msg,MGMT_ATTR_SET_BW,ntohs(sparam->mgmt_bw));
	//		break;
	//	}
	//	case MGMT_SET_TXPOWER: {
	//		paramdata |= SET_TXPOWER;
	//		nla_put_s16(msg,MGMT_ATTR_SET_TXPOWER,ntohs(sparam->mgmt_txpower));
	//		break;
	//	}
	//	default:{
	//		break;
	//	}
	//	}

			memset(cmd,0,sizeof(cmd));
	longitude = htond(sparam->mgmt_longitude);
			sprintf(cmd,
			"sed -i \"s/longitude .*/longitude %.8f/g\" /etc/node_xwg",
			longitude);
		system(cmd);

		memset(cmd,0,sizeof(cmd));
	latitude = htond(sparam->mgmt_latitude);
				sprintf(cmd,
			"sed -i \"s/latitude .*/latitude %.8f/g\" /etc/node_xwg",
			latitude);
		system(cmd);

	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,
		"sed -i \"s/rx_channel_mode .*/rx_channel_mode %d/g\" /etc/node_xwg",
		sparam->mgmt_rx_channel_mode);
	system(cmd);
	nla_put_u8(msg, MGMT_ATTR_SET_RX_CHANNEL_MODE, sparam->mgmt_rx_channel_mode);
	RX_CHANNEL_MODE_INIT = sparam->mgmt_rx_channel_mode;

	nla_put_u32(msg, MGMT_ATTR_SET_PARAM, paramdata);

	nl_send_auto_complete(sock, msg);

	nlmsg_free(msg);

	system("sync");

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto err_free_sock;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, mgmt_netlink_param_callback, &opts);
	nl_cb_err(cb, NL_CB_CUSTOM, print_error, NULL);

	nl_recvmsgs(sock, cb);
	ret = 0;
err_free_sock:
	nl_socket_free(sock);
	return ret;
}
