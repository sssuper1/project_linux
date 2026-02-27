#ifndef MGMT_TYPES_H_
#define MGMT_TYPES_H_
#include <stdint.h>
#include <asm/byteorder.h>
#include <linux/types.h>
#include <linux/if_ether.h>


#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include  "wg_config.h"
#define LINUX

#ifdef LINUX
#include <netinet/in.h>
#define PACKED __attribute__((packed))
#else
#include <Windows.h>
#endif

//#define Radio_7800
//#define Radio_220

//#define Radio_SWARM_k1
//#define Radio_SWARM_k2
//#define Radio_SWARM_k4
//#define Radio_SWARM_WNW
//#define Radio_CEC
#define Radio_SWARM_S2
#define Radio_QK 





#ifdef Radio_SWARM_k1
  #define NODE_MAX 112
#elif defined Radio_220
  #define NODE_MAX 28
#elif defined Radio_7800
  #define NODE_MAX 112
#elif defined Radio_SWARM_S2
  #define NODE_MAX 64
#elif defined Radio_CEC
  #define NODE_MAX 5
#elif defined Radio_SWARM_k2
  #define NODE_MAX 112
#elif defined Radio_SWARM_k4
  #define NODE_MAX 112
#elif defined Radio_SWARM_WNW
  #define NODE_MAX 112
#endif


#ifdef Radio_SWARM_k1
#define NET_SIZE 112
#elif defined Radio_SWARM_k2
#define NET_SIZE 112
#elif defined Radio_SWARM_k4
#define NET_SIZE 112
#elif defined Radio_SWARM_WNW
#define NET_SIZE 112
#elif defined Radio_220
#define NET_SIZE     28
#elif defined Radio_7800
#define NET_SIZE 112
#elif defined Radio_SWARM_S2
#define NET_SIZE 56             
#elif defined Radio_CEC
#define NET_SIZE 5
#endif

#define MCS_NUM  8
#define MAX_QUEUE_NUM 256
#define HEAD 0x4C4A //PH
#define MCS_WINDOW_SIZE 5   //链路自适应窗口大小

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
#define QK_WG_PORT   10000      //20所上位机  add by sdg
#define QK_CJ_PORT   7075
#define CJ_MULTIADDR_IP   "239.255.255.255"    //CJ 组播IP
#define CJ_MULTIADDR_PORT  7075    //CJ 组播IP
#define DT_MIN_FREQ			225
#define DT_MAX_FREQ			4000	
typedef enum
{
	FALSE = 0,
	TRUE = 1
}BOOL;


typedef int INT32;
typedef short INT16;
typedef char INT8;

typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

#define RETURN_OK 0
#define RETURN_FAILED -1
#define RETURN_NULL -2
#define RETURN_TIMEOUT -3
#define TCPCONNECTNUM 6
#define TIME_WAIT_FOR_EVER 0
#define NUMBERNULL -1

#define TCP_PROTOCOL 0x06
#define UDP_PROTOCOL 0x11

#define NO_MCS 0x0f

#define NODEPARAMTYPE 0x0001

#define MSGKEY   1002
#define MSG_TYPE 1
#define ETH_ADDR_SIZE 6

#define HOP_FREQ_NUM 32

#define POWER_CHANNEL_NUM 4
#define POWER_TABLE_SIZE 71


#define MGMT_NL_NAME "lumgmt"

#define MGMT_NL_MCAST_GROUP_TPMETER	"lutpmeter"

enum mgmt_nl_attrs {
	MGMT_ATTR_UNSPEC,
	MGMT_ATTR_GET_INFO,
	MGMT_ATTR_SET_PARAM,
	MGMT_ATTR_NODEID,


	//route
	MGMT_ATTR_VERSION_ROUTE,
	MGMT_ATTR_PKTNUMB_ROUTE,
	MGMT_ATTR_TABLE_ROUTE,
	MGMT_ATTR_UCDS_ROUTE,
	//	MGMT_ATTR_MESH_IFINDEX,
	//veth
	MGMT_ATTR_VERSION_VETH,
	MGMT_ATTR_INFO_VETH,
	//	MGMT_ATTR_INFO_VETH2,

	MGMT_ATTR_VETH_ADDRESS,
	MGMT_ATTR_VETH_TX,
	MGMT_ATTR_VETH_RX,

	//mac phy
	MGMT_ATTR_VERSION_MAC_PHY,
	MGMT_ATTR_MSG_MAC,


	MGMT_ATTR_FREQ,
	MGMT_ATTR_BW,
	MGMT_ATTR_TXPOWER,

	//set
	//mgmt
	MGMT_ATTR_SET_NODEID,
	//route
	MGMT_ATTR_SET_INTERVAL,
	MGMT_ATTR_SET_TTL,
	//veth
	MGMT_ATTR_SET_QUEUE_NUM,
	MGMT_ATTR_SET_QUEUE_LENGTH,
	MGMT_ATTR_SET_QOS_STATEGY,
	MGMT_ATTR_SET_UNICAST_MCS,
	MGMT_ATTR_SET_MULTICAST_MCS,
	//mac phy
	MGMT_ATTR_SET_FREQUENCY,
	MGMT_ATTR_SET_POWER,
	MGMT_ATTR_SET_BANDWIDTH,
	MGMT_ATTR_SET_TEST_MODE,
	MGMT_ATTR_SET_TEST_MODE_MCS,
	MGMT_ATTR_SET_PHY,
	MGMT_ATTR_SET_WORKMODE,
	MGMT_ATTR_SET_IQ_CATCH,
	MGMT_ATTR_SET_SLOTLEN,
	MGMT_ATTR_SET_POWER_LEVEL,
	MGMT_ATTR_SET_POWER_ATTENUATION,
	MGMT_ATTR_SET_RX_CHANNEL_MODE,
#ifdef Radio_CEC
    MGMT_ATTR_SET_NCU,
    
#endif

	/* add attributes above here, update the policy in netlink.c */
	__MGMT_ATTR_AFTER_LAST,
	NUM_MGMT_ATTR = __MGMT_ATTR_AFTER_LAST,
	MGMT_ATTR_MAX = __MGMT_ATTR_AFTER_LAST - 1
};

enum mgmt_nl_commands {
	MGMT_CMD_UNSPEC,
	MGMT_CMD_GET_ROUTE_INFO,
	MGMT_CMD_GET_VETH_INFO,
	MGMT_CMD_SET_PARAM,
	MGMT_CMD_NODEID,


	//route
	MGMT_CMD_VERSION_ROUTE,


	//veth
	MGMT_CMD_VERSION_VETH,
	MGMT_CMD_VETH_ADDRESS,
	MGMT_CMD_VETH_TX,
	MGMT_CMD_VETH_RX,
	//mac phy
	MGMT_CMD_VERSION_MAC_PHY,
	MGMT_CMD_FREQ,
	MGMT_CMD_BW,
	MGMT_CMD_TXPOWER,
	/* add new commands above here */
	__MGMT_CMD_AFTER_LAST,
	MGMT_CMD_MAX = __MGMT_CMD_AFTER_LAST - 1
};

enum mgmt_tp_meter_reason {
	MGMT_TP_REASON_COMPLETE = 3,
	MGMT_TP_REASON_CANCEL = 4,
	/* error status >= 128 */
	MGMT_TP_REASON_DST_UNREACHABLE = 128,
	MGMT_TP_REASON_RESEND_LIMIT = 129,
	MGMT_TP_REASON_ALREADY_ONGOING = 130,
	MGMT_TP_REASON_MEMORY_ERROR = 131,
	MGMT_TP_REASON_CANT_SEND = 132,
	MGMT_TP_REASON_TOO_MANY = 133
};

typedef enum EMGMT_TYPE {
	MGMT_NODEFIND = 0x3c01,
	MGMT_DEVINFO = 0x4d01,
	MGMT_DEVINFO_REPORT = 0x3c02,
	MGMT_SET_PARAM = 0x4d02,
	MGMT_TOPOLOGY_INFO = 0x3c03,
	MGMT_SLOT_INFO = 0x3c04,
	MGMT_SPECTRUM_QUERY = 0x4d05,
	MGMT_SPECTRUM_REPORT = 0x3c05,

	MGMT_POWEROFF = 0x4d06,
	MGMT_RESTART = 0x4d07,
	MGMT_FACTORY_RESET = 0x4d08,
	MGMT_FIRMWARE_UPDATE = 0x4d09,
	MGMT_FILE_UPDATE = 0x4d0a,
	MGMT_MULTIPOINT_SET = 0x4d0b,
	MGMT_TOPOLOGY_REQUEST = 0x4d0e	//add by yang,拓扑请求
} Emgmt_type;

typedef enum EMGMT_FIRMWARE_TYPE {
	MGMT_NAME = 0x0901,
	MGMT_CONTENT = 0x0902,
	MGMT_END = 0x0903,
	MGMT_UPDATE_FIRMWARE = 0x0904
} Emgmt_firmware_type;

typedef enum EMGMT_FILE_TYPE {
	MGMT_FILENAME = 0x0a01,
	MGMT_FILECONTENT = 0x0a02,
	MGMT_FILEEND = 0x0a03,
	MGMT_UPDATE_FILE = 0x0a04
} Emgmt_file_type;

typedef struct __attribute__((__packed__)) {
	uint16_t node_num;
	uint16_t selfid;
	uint32_t selfip;
}Snodefind;

typedef enum EMGMT_HEADER_TYPE {
	MGMT_LOGIN = 0x0000,
	MGMT_SET_ID = 0x01,
	MGMT_SET_INTERVAL = 0x02,
	MGMT_SET_TTL = 0x04,
	MGMT_SET_QUEUE_NUM = 0x08,
	MGMT_SET_QUEUE_LENGTH = 0x10,
	MGMT_SET_QOS_STATEGY = 0x20,
	MGMT_SET_UNICAST_MCS = 0x40,
	MGMT_SET_MULTICAST_MCS = 0x80,
	MGMT_SET_FREQUENCY = 0x100,
	MGMT_SET_POWER = 0x200,
	MGMT_SET_BANDWIDTH = 0x400,
	MGMT_SET_TEST_MODE = 0x800,
	MGMT_SET_TEST_MODE_MCS = 0x1000,
	MGMT_SET_PHY = 0x2000,
	MGMT_SET_WORKMODE = 0x4000,
	MGMT_SET_IQ_CATCH = 0x8000,
	//MGMT_SET_SLOTLEN = 0x10000
#ifdef Radio_CEC
    MGMT_SET_NCU = 0x4000,

#endif	
} Emgmt_header_type;

typedef enum EMGMT_HEADER_PHY_TYPE {
	MGMT_SET_SLOTLEN = 0x0001,
	MGMT_SET_POWER_LEVEL = 0x0002,
	MGMT_SET_POWER_ATTENUATION = 0x0004,
	MGMT_SET_RX_CHANNEL_MODE = 0x0008
} Emgmt_header_phy_type;

enum{
	FIX_FREQ_MODE = 1,
	HOP_FREQ_MODE,
	COGNITVE_HOP_FREQ_MODE
};

typedef struct __attribute__((__packed__)) {
	uint16_t mgmt_head;
	uint16_t mgmt_len;
	uint16_t mgmt_type;
	uint16_t mgmt_keep;
	uint8_t  mgmt_data[];
}Smgmt_header;

typedef struct{
	uint8_t	 NET_work_mode;
	uint8_t	 fh_len;
	uint16_t res;
	uint32_t hop_freq_tb[HOP_FREQ_NUM];
}Smgmt_net_work_mode;

typedef struct{
	uint32_t trig_mode;
	uint32_t catch_addr;
	uint32_t catch_length;
}Smgmt_IQ_Catch;

#ifdef Radio_SWARM_S2
typedef struct __attribute__((__packed__)){
	uint8_t rf_agc_framelock_en;
	uint8_t phy_cfo_bypass_en;
	uint16_t phy_pre_STS_thresh;
	uint16_t phy_pre_LTS_thresh;
	uint16_t phy_tx_iq0_scale;
	uint16_t phy_tx_iq1_scale;
	uint8_t  phy_msc_length_mode;
 	uint8_t  phy_sfbc_en;
 	uint8_t  phy_cdd_num;
 	uint8_t  reserved;
}Smgmt_phy;

#else
typedef struct __attribute__((__packed__)){
	uint8_t rf_agc_framelock_en;
	uint8_t phy_cfo_bypass_en;
	uint16_t phy_pre_STS_thresh;
	uint16_t phy_pre_LTS_thresh;
	uint16_t phy_tx_iq0_scale;
	uint16_t phy_tx_iq1_scale;
}Smgmt_phy;
#endif


typedef struct __attribute__((__packed__)) {
	uint16_t mgmt_id;
	uint16_t mgmt_route_interval;
	uint16_t mgmt_route_ttl;
	uint16_t mgmt_virt_queue_num;
	uint16_t mgmt_virt_queue_length;
	uint16_t mgmt_virt_qos_stategy;
	uint8_t mgmt_virt_unicast_mcs;
	uint8_t mgmt_virt_multicast_mcs;
	uint8_t  mgmt_mac_bw;
	uint8_t  reserved;
	uint32_t mgmt_mac_freq;
	uint16_t mgmt_mac_txpower;
	uint16_t mgmt_mac_txpower_ch[POWER_CHANNEL_NUM];
	uint8_t  mgmt_mac_power_level;
	uint8_t  mgmt_mac_power_attenuation;
	uint8_t  mgmt_rx_channel_mode;
	uint16_t mgmt_mac_work_mode;
	Smgmt_net_work_mode  mgmt_net_work_mode;
	Smgmt_IQ_Catch  mgmt_mac_iq_catch;
#ifdef Radio_CEC

	uint8_t  mgmt_NCU_node_id;
	uint16_t reserved2;
#endif
	Smgmt_phy mgmt_phy;
	uint8_t  u8Slotlen;
	uint8_t resv;
}Smgmt_set_param;

typedef struct __attribute__((__packed__)) {
	uint32_t mgmt_ip;
	uint16_t mgmt_id;
	uint16_t mgmt_route_interval;
	uint16_t mgmt_route_ttl;
	uint16_t mgmt_virt_queue_num;
	uint16_t mgmt_virt_queue_length;
	uint16_t mgmt_virt_qos_stategy;
	uint8_t mgmt_virt_unicast_mcs;
	uint8_t mgmt_virt_multicast_mcs;
	uint8_t  mgmt_mac_bw;
	uint8_t  reserved;
	uint32_t mgmt_mac_freq;
	uint16_t mgmt_mac_txpower;
	uint16_t mgmt_mac_txpower_ch[POWER_CHANNEL_NUM];
	uint8_t  mgmt_rx_channel_mode;
	uint16_t mgmt_mac_work_mode;
	double   mgmt_longitude;
	double   mgmt_latitude;
}Smgmt_param;



typedef struct SMGMT_TRANSMIT_INFO {
	uint16_t id;
	uint32_t ip;
	uint8_t  macaddr[ETH_ALEN];
	uint32_t txrate;
	uint32_t rxrate;
	uint32_t freq;
	uint16_t bw;
	int16_t  txpower;
}Smgmt_transmit_info;


enum {
	NO_FLAGS = 0x00,
	SET_ID = 0x01,
	SET_FREQ = 0x02,
	SET_BW = 0x04,
	SET_TXPOWER = 0x08,
	//	USE_READ_BUFF = 0x10,
	//	SILENCE_ERRORS = 0x20,
	//	NO_OLD_ORIGS = 0x40,
	//	COMPAT_FILTER = 0x80,
	//	SKIP_HEADER = 0x100,
	//	UNICAST_ONLY = 0x200,
	//	MULTICAST_ONLY = 0x400,
};

enum {
	ROUTE_NO_FLAGS = 0x00,
	ROUTE_SET_INTERVAL = 0x01,
	ROUTE_SET_TTL = 0x02

};


typedef enum {LinkSt_h1,LinkSt_h2,LinkSt_idle} nbr_link;


struct batadv_jgk_node {
	__be32   nodeid;
	__be32   s_ogm;
	__be32   r_ogm;
	__be32   f_ogm;
	__be32   s_bcast;
	__be32   r_bcast;
	__be32   f_bcast;
	__be32   num;
};

struct batadv_jgk_route {
	uint8_t   route;
	uint8_t   reserved;
	uint16_t  tq;
	uint8_t   neigh1;
	uint8_t   reserved1;
	uint16_t  tq1;
	uint8_t   neigh2;
	uint8_t   reserved2;
	uint16_t  tq2;
	__be32 s_unicast;
	__be32 r_unicast;
};

struct routetable
{
	struct batadv_jgk_route bat_jgk_route[NODE_MAX];
};

//struct routetable1
//{
//	struct batadv_jgk_route bat_jgk_route[NODE_MAX*2];
//};

struct route_para
{
	int interval;//ms
	int ttl;
};

typedef struct {
	uint32_t  indication;
	uint32_t  queue_length;
	uint8_t   enqueue_num;
	uint8_t   QoS_Stategy;
	uint8_t   unicast_msc;
	uint8_t   multicast_msc;
	uint16_t   frequency;
	uint16_t   power;
	uint8_t   bandwidth;
	uint8_t   test_send_mode;
	uint8_t   test_send_mode_mcs;
	uint8_t   reserved;
#ifdef Radio_CEC
    uint8_t   NCU_node_id;
    uint8_t   NET_work_mode;
	uint16_t  res2;
#endif
}jgk_set_parameter;

enum {
	VETH_SET_QUEUE_NUM = 0x01,
	VETH_SET_QUEUE_LENGTH = 0x02,
	VETH_SET_QOS_STATEGY = 0x04,
	VETH_SET_UNICAST_MCS = 0x08,
	VETH_SET_MULTICAST_MCS = 0x10,
	VETH_SET_FREQUENCY = 0x20,
	VETH_SET_POWER = 0x40,
	VETH_SET_BANDWIDTH = 0x80,
	VETH_SET_TEST_MODE = 0x100,
	VETH_SET_TEST_MODE_MCS = 0x200,
#ifdef Radio_CEC
    VETH_SET_NCU = 0x800,
    VETH_SET_WORKMODE = 0x1000,
#endif
};

typedef struct {
	//	uint8_t  packet_class[3]; //0x03_0xcc_0x04
	//	uint8_t  node_id;
	uint8_t  nbr_list[NET_SIZE];
	uint8_t  slot_list[NET_SIZE * 2];
	uint8_t  n_used_l0;
	uint8_t  n_free_hx;
	uint8_t  n_ol0_hx;
	uint8_t  n_free_h1;
	uint8_t  n_ol0_h1;
	uint8_t  n_free_h2;
	uint8_t  n_ol0_h2;
	uint8_t  n_used_l1;
	uint8_t  ctf_live_num;
	uint8_t  tsn_avgload_demand;
	uint8_t  tsn_traffic_demand;
	uint8_t  reserved;
}ob_state_part1;

typedef struct {
	//	uint8_t  packet_class[3]; //0x01_0xaa_0xbb
	//	uint8_t  node_id;
	uint16_t time_jitter[NET_SIZE];
	uint8_t  snr[NET_SIZE];
	uint8_t  rssi[NET_SIZE];
	uint8_t  mcs[NET_SIZE];
	short    good[NET_SIZE];
	short    bad[NET_SIZE];
	uint8_t  ucds[NET_SIZE];
	uint8_t  noise[NET_SIZE];
}ob_state_part2;

typedef struct {
	uint16_t tx_in[MAX_QUEUE_NUM];
	uint16_t tx_qlen[MAX_QUEUE_NUM];
	uint32_t tx_inall;
	uint32_t tx_outall;
	uint32_t tx_in_lose;
	uint32_t tx_out_lose;
	uint32_t rx_inall;
	uint32_t rx_outall;
	uint32_t rx_in_lose;
	uint32_t rx_out_lose;
	uint32_t mac_list_tx_cnt;
	uint32_t tx_in_cnt;
	uint32_t phy_tx_done_cnt;
	uint32_t phy_rx_done_cnt;
	uint32_t ogm_in;
	uint32_t ogm_in_len;
	uint32_t ogm_slot;
	uint32_t ogm_out_len;
	uint32_t ping_in;
	uint32_t ping_in_len;
	uint32_t ping_slot;
	uint32_t ping_out_len;
	uint32_t bcast_in;
	uint32_t bcast_in_len;
	uint32_t bcast_slot;
	uint32_t bcast_out_len;
	uint32_t ucast_in;
	uint32_t ucast_in_len;
	uint32_t ucast_slot;
	uint32_t ucast_out_len;
}virt_eth_jgk_info;

typedef struct {
	uint8_t  veth_version[4];
	uint8_t  agent_version[4];
	uint8_t  ctrl_version[4];
	uint32_t  enqueue_bytes[MCS_NUM]; //每个mcs队列对应的入队列比特数
	uint32_t  outqueue_bytes[MCS_NUM]; //每个mcs队列对应的处队列比特数
	ob_state_part1 mac_information_part1; //mac层监管控信息part1
	ob_state_part2 mac_information_part2; //mac层监管控信息part2
	virt_eth_jgk_info traffic_queue_information; //业务模块监管控信息
#ifdef	Radio_SWARM_S2
	DEVICE_SC_STATUS_REPORT amp_infomation;     //add by sdg 功放数据结构
#endif

}jgk_report_infor;



//struct mgmt_msg{
//	uint32_t  node_id:8;
//	uint32_t  mcs:4;
//	uint32_t  rssi:8;
//	uint32_t  snr:8;
//	uint32_t  noise:8;
//	uint32_t  ucds:4;
//	uint32_t  enqueue_bytes:28;
//	uint32_t  outqueue_bytes:28;
//}__attribute__((__packed__));

struct mgmt_msg {
	uint32_t  node_id : 8;
	uint32_t  mcs : 8;
	uint32_t  ucds : 8;
	uint32_t  rssi : 8;
	uint32_t  snr : 8;
	int  time_jitter : 16;
	int  good : 16;
	int  bad : 16;
	uint32_t  noise : 8;
	uint32_t  reserved : 8;
}__attribute__((__packed__));


struct mgmt_send {
	uint8_t node_id;
	uint8_t bw;
	uint16_t txpower;
	uint32_t freq;
	uint32_t neigh_num;
	uint32_t seqno;
	uint32_t tx;
	uint32_t rx;
	uint8_t  veth_version[4];
	uint8_t  agent_version[4];
	uint8_t  ctrl_version[4];
	struct mgmt_msg msg[NET_SIZE];
	ob_state_part1 mac_information_part1; //mac层监管控信息part1
#ifdef	Radio_SWARM_S2
	DEVICE_SC_STATUS_REPORT amp_infomation;     //add by sdg 功放数据结构
#endif

};

struct mgmt_header {
	uint8_t node_id;
	uint8_t bw;
	uint16_t txpower;
	uint32_t freq;
	uint32_t neigh_num;
	uint32_t seqno;
};

typedef struct {
	uint8_t                       dest_mac_addr[ETH_ADDR_SIZE];                      // Destination MAC address
	uint8_t                       src_mac_addr[ETH_ADDR_SIZE];                       // Source MAC address
	uint16_t                      ethertype;                                        // EtherType
}__attribute__((packed)) ethernet_header_t;

typedef struct ip_header {
	char ver_ihl; // Version (4 bits) + Internet header length (4 bits)
	char tos; // Type of service
	short tlen; // Total length
	short identification; // Identification
	short flags_fo; // Flags (3 bits) + Fragment offset (13 bits)
	char ttl; // Time to live
	char proto; // Protocol
	short crc; // Header checksum
	unsigned int saddr; // Source address
	unsigned int daddr; // Destination address
}__attribute__((packed)) ip_header;

typedef struct udp_header {
	short sport; // Source port
	short dport; // Destination port
	short len; // Datagram length
	short crc; // Checksum
}__attribute__((packed)) udp_header;


//add by yang 20230312
typedef struct mgmt_status_data {
	uint32_t selfip;
	uint16_t selfid;
	uint16_t tv_route;
	uint16_t maxHop;
	uint16_t num_queues;
	uint16_t depth_queues;
	uint16_t qos_policy;
	uint8_t mcs_unicast;
	uint8_t mcs_broadcast;
	uint8_t bw;
	uint8_t reserved;
	uint32_t freq;
	uint16_t txpower;
	uint16_t work_mode;
	double longitude;
	double latitude;
	char software_version[16];
	char hardware_version[16];
}__attribute__((packed)) mgmt_status_data;
typedef struct mgmt_status_header {
	uint16_t flag;
	uint16_t len;
	uint16_t type;
	uint16_t reserved;
	struct mgmt_status_data status_data;
}__attribute__((packed)) mgmt_status_header;

//拓扑信息
typedef struct neighbor_data {
	uint32_t neighbor_ip;
	uint32_t neighbor_rssi;
	uint32_t neighbor_tx;
}__attribute__((packed)) neighbor_data;

typedef struct topo_data {
	uint32_t selfip;		//设备IP
	double   longitude;		//经度
	double   latitude;		//纬度
	uint32_t noise;			//本地噪声
	uint32_t tx_traffic;	//节点流量发
	uint32_t rx_traffic;	//节点流量收
	uint32_t neighbors_num;	//邻居个数
	struct neighbor_data neighbors_data[];
}__attribute__((packed)) topo_data;

typedef struct {
	uint32_t  srcIp;
	//uint16_t msg_type;//拓扑请求类型
}__attribute__((packed)) topology_request;


#ifdef Radio_CEC
typedef enum {
    Scan_Slot,
    Control_Slot,
    Traffic_Slot
}tdpa_slot_type; //时隙类型，分控制时隙和业务时隙两种

typedef enum {
    SCAN_PHASE,
    DATA_PHASE
}tdpa_network_work_phase; //工作阶段

typedef enum {
    PRIOR_POSIZTION_MODE,
	BLIND_SCAN_MODE
}tdpa_network_work_mode; //工作阶段


#endif

//////////////////////////////web
typedef enum EM_TYPE{
	N1,
	N2
}emType;

typedef enum EM_TABLE{
	SYSTEMINFO,
	SURVEYINFO,
	NODEINFO,
	LINKINFO,
	USERINFO,
	MESHINFO
}emTable;

typedef struct ST_NEIGHINFO{
	int deviceid;
	int signallevel;
	int noise;
}stNeighInfo;

typedef struct ST_INDATA{
	char name[20];
	char value[24];
	char state[4];
	char lib[4];
}stInData;

typedef struct ST_SYSTEMINFO{
	char ipaddr[16];
	char runtimeid[24];
	int device;
	char g_ver[12];
	int rf_freq;
	int m_chanbw;
	int m_txpower;
	int m_rate;
	emType m_type;
	stNeighInfo m_stNeighInfo[32];
}stSysteminfo;

typedef struct ST_SURVEYINFO
{
	int id;
	int fre[301];
}stSurveyInfo;

typedef struct ST_NODE{
	int id;
	char time[24];
	char ip[16];
	int txpower;
	int bw;
	int freq;
	int mcs;
	int mode;
	int interval;
	int max;
	int nbor;
	char    lotd[20];			//节点经度
	char    latd[20];			//节点纬度
	char softver[12];
	char harver[12];
}stNode;

typedef struct ST_NBIFNO{
	int nbid1;
	int snr1;
	int getlv1;
	int flowrate1;
}stNbInfo;

typedef struct ST_LINK{
	int id;
	char time[24];
	stNbInfo m_stNbInfo[32];
}stLink;


typedef struct ST_USERINFO{
	char m_ip[16];
	char m_dhcpStart[16];
	char m_dhcpGateway[16];
	char m_dhcpDns[16];
}stUserInfo;

typedef struct ST_MESHINFO{
	int m_txpower;
	int rf_freq;
	int m_chanbw;
	int m_rate;
	int m_distance;
	int m_ssid;
	int m_bcastmode;
}stMeshInfo;

typedef struct BCAST_MESHINFO{
	uint16_t m_txpower;
	uint16_t m_txpower_ch[POWER_CHANNEL_NUM];
	uint32_t rf_freq;
	uint8_t m_chanbw;
	uint8_t m_rate;
	uint8_t m_bcastmode;
	uint8_t workmode;
	uint8_t m_route;
	uint8_t m_slot_len;
	uint16_t m_trans_mode;
	uint32_t m_select_freq_1;	
	uint32_t m_select_freq_2;	
	uint32_t m_select_freq_3;	
	uint32_t m_select_freq_4;	
	uint8_t power_level;
	uint8_t power_attenuation;
	uint8_t rx_channel_mode;
		
	// int m_distance;
	// int m_ssid;
	uint8_t txpower_isset;
	uint8_t freq_isset;
	uint8_t chanbw_isset;
	uint8_t rate_isset;
	uint8_t workmode_isset;
	uint8_t route_isset;
	uint8_t slot_isset;
	uint8_t trans_mode_isset;	
	uint8_t select_freq_isset;
	uint8_t power_level_isset;
	uint8_t power_attenuation_isset;
	uint8_t rx_channel_mode_isset;
	//bool bcastmode_isset;

// 用作更新systeminfo库
	int sys_power;
	int sys_power_level;
	int sys_power_attenuation;
	int sys_freq;
	int sys_rate;
	int sys_bw;
	int sys_workmode;
	int sys_rx_channel_mode;

}bcMeshInfo;

typedef struct 
{
	uint8_t device_id;
	uint16_t g_txpower;
	uint32_t g_rf_freq;
	uint8_t g_chanbw;
	uint8_t g_rate;
	uint8_t g_bcastmode;
	uint8_t g_workmode;
	uint8_t g_route;
	uint8_t g_slot_len;
	uint16_t g_trans_mode;
	uint32_t g_select_freq_1;	
	uint32_t g_select_freq_2;	
	uint32_t g_select_freq_3;	
	uint32_t g_select_freq_4;	

}Global_Radio_Param;

uint8_t find_minMcs(uint8_t *arr,int size);
uint32_t find_max(uint32_t *arr,int size);

#endif /* MGMT_TYPES_H_ */
