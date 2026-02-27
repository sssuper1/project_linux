#ifndef MGMT_TYPES_H_
#define MGMT_TYPES_H_
//#include <stdint.h>
#include <asm/byteorder.h>
#include <linux/types.h>
#include <linux/if_ether.h>
#define MGMT_NL_NAME "lumgmt"
#define POWER_CHANNEL_NUM 4

//#define Radio_7800
//#define Radio_220
//#define Radio_SWARM_k1
//#define Radio_SWARM_k2
//#define Radio_SWARM_k4
//#define Radio_SWARM_WNW
#define Radio_SWARM_S2
//#define Radio_CEC


#define MGMT_NL_MCAST_GROUP_TPMETER	"lutpmeter"

#ifdef Radio_SWARM_k1 
#define NODE_MAX 112
#elif defined Radio_SWARM_k2
#define NODE_MAX 112
#elif defined Radio_SWARM_k4
#define NODE_MAX 112
#elif defined Radio_SWARM_WNW
#define NODE_MAX 112
#elif defined Radio_220
#define NODE_MAX 28
#elif defined Radio_7800
#define NODE_MAX 112
#elif defined Radio_SWARM_S2
#define NODE_MAX 56
#elif defined Radio_CEC
#define NODE_MAX 5

#endif

#define NET_SIZE NODE_MAX
#define MCS_NUM  8
#define MAX_QUEUE_NUM 256

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
	MGMT_TP_REASON_COMPLETE		    = 3,
	MGMT_TP_REASON_CANCEL			= 4,
	/* error status >= 128 */
	MGMT_TP_REASON_DST_UNREACHABLE	= 128,
	MGMT_TP_REASON_RESEND_LIMIT		= 129,
	MGMT_TP_REASON_ALREADY_ONGOING	= 130,
	MGMT_TP_REASON_MEMORY_ERROR		= 131,
	MGMT_TP_REASON_CANT_SEND		= 132,
	MGMT_TP_REASON_TOO_MANY		    = 133,
};

typedef enum SMGMT_HEADER_TYPE {
	MGMT_SET_ID = 0x01,
	MGMT_SET_INTERVAL=0x02,
	MGMT_SET_TTL=0x04,
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
	MGMT_SET_SLOTLEN = 0x10000,
	MGMT_SET_POWER_LEVEL = 0x20000,
	MGMT_SET_POWER_ATTENUATION = 0x40000,
	MGMT_SET_RX_CHANNEL_MODE = 0x80000
#ifdef Radio_CEC
    MGMT_SET_NCU = 0x4000,

#endif
} Smgmt_header_type;

typedef struct __attribute__((__packed__)){
	uint16_t mgmt_head;
	uint16_t mgmt_len;
	uint32_t mgmt_type;
	uint8_t  mgmt_data[];
}Smgmt_header;



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
typedef struct __attribute__((__packed__)){
	u8	 NET_work_mode;
	u8	 fh_len;
	u16  res;
	u32   hop_freq_tb[32];//[32]

}Smgmt_net_work_mode;

typedef struct{
	uint32_t trig_mode;
	uint32_t catch_addr;
	uint32_t catch_length;
}Smgmt_iq_catch;

typedef struct __attribute__((__packed__)){
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
#ifdef Radio_CEC
    uint8_t  mgmt_net_work_mode;
	uint8_t  mgmt_NCU_node_id;
	uint16_t reserved2;
#endif
	Smgmt_phy mgmt_phy;
	uint8_t  u8Slotlen;
	uint8_t resv;  
}Smgmt_set_param;



typedef struct SMGMT_TRANSMIT_INFO{
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

enum{
	ROUTE_NO_FLAGS=0x00,
	ROUTE_SET_INTERVAL=0x01,
	ROUTE_SET_TTL=0x02

};


struct batadv_jgk_node{
	__be32   nodeid;
	__be32   s_ogm;
	__be32   r_ogm;
	__be32   f_ogm;
	__be32   s_bcast;
	__be32   r_bcast;
	__be32   f_bcast;
	__be32   num;
};

struct batadv_jgk_route{
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


typedef struct{
	uint32_t  indication;
	uint32_t  queue_length;
	uint8_t   enqueue_num;
	uint8_t   QoS_Stategy;
	uint8_t   unicast_msc;
	uint8_t   multicast_msc;
	uint16_t   frequency;
	 uint16_t   power;
	uint16_t   power_ch[POWER_CHANNEL_NUM];
	uint8_t   bandwidth;
	uint8_t   test_send_mode;
	uint8_t   test_send_mode_mcs;
	uint8_t   res;
	Smgmt_net_work_mode work_mode_msg;
	Smgmt_iq_catch iq_catch;
	Smgmt_phy phy_msg;
	uint8_t  u8Slotlen;
	uint8_t power_level;
	uint8_t power_attenuation;
	uint8_t rx_channel_mode;
	uint8_t resv;
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
	VETH_SET_PHY = 0x400,
	VETH_SET_WORKMODE = 0x800,
	VETH_SET_IQ_CATCH = 0x1000,
	VETH_SET_SLOT_LEN = 0x2000,
	VETH_SET_POWER_LEVEL = 0x4000,
	VETH_SET_POWER_ATTENUATION = 0x8000,
	VETH_SET_RX_CHANNEL_MODE = 0x10000
#ifdef Radio_CEC
//    VETH_SET_NCU = 0x800,

#endif
};

typedef struct{
//	uint8_t  packet_class[3]; //0x03_0xcc_0x04
//	uint8_t  node_id;
	uint8_t  nbr_list[NET_SIZE];
	uint16_t  slot_list[NET_SIZE];
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

typedef struct{
//	uint8_t  packet_class[3]; //0x01_0xaa_0xbb
//	uint8_t  node_id;
	uint16_t time_jitter[NET_SIZE];
	uint8_t  snr [NET_SIZE];
	uint8_t  rssi[NET_SIZE];
	uint8_t  mcs [NET_SIZE];
	uint16_t good[NET_SIZE];
	uint16_t bad [NET_SIZE];
	uint8_t  ucds[NET_SIZE];
	uint8_t  noise[NET_SIZE];
}ob_state_part2;

typedef struct{
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

#ifdef Radio_SWARM_S2
//设备自检状态信息数据
typedef struct __attribute__((__packed__)){
    /* 综合模块状态 */
    int8_t temperature;               /* 综合模块温度 有符号char型。精度为℃，取值范围-127到﹢127    */
    uint8_t voltage;                   /* 综合模块电压  相对12V的偏移量，LSB表示0.1V，有符号数。例如，0x01表示12.1V，0x81表示11.9V*/
    uint8_t fan_status;                /* 综合模块风机转速状态 取值0-100，表示当前为0-100%*/
    uint8_t nav_lock_status;           /* 综合模块卫导锁定状态 0x00：已锁定，0x01：未锁定*/
    uint8_t sync_status;               /* 综合模块内外同步状态 0x00：外同步  0x01：内同步 */
    uint8_t clock_switch_status;       /* 综合模块内外时钟切换状态 0x00：板载时钟 0x01：外供时钟 */
    uint16_t panel_rs232_rx_count;      /* 综合模块与面板RS232接收消息记数 无符号数，取值范围0-65535  */
    uint16_t panel_rs232_tx_count;     /* 综合模块与面板RS232发送消息记数 */
    uint16_t module_power_rs422_rx_count;     /* 综合模块与电源变换RS422接收消息记数 无符号数，取值范围0-65535。*/
    uint16_t module_power_rs422_tx_count;     /* 综合模块与电源变换RS422发送消息记数 */
    uint16_t module_freq_rs422_rx_count;      /* 综合模块与频率源RS422接收消息记数 无符号数，取值范围0-65535*/
    uint16_t module_freq_rs422_tx_count;      /* 综合模块与频率源RS422发送消息记数 */
    uint16_t rf_rs422_rx_count;        /* 综合模块与射频前端RS422接收消息记数 无符号数，取值范围0-65535*/
    uint16_t rf_rs422_tx_count;        /* 综合模块与射频前端RS422发送消息记数 */
    uint8_t soc1_online;              /* 综合模块SOC1在线 0x00：程序未正常运行，0x01：程序正常运*/
    uint8_t soc2_online;               /* 综合模块SOC2在线 */
    uint8_t soc3_online;               /* 综合模块SOC3在线 */
    
    /* ADC/DAC状态 */
    uint8_t adc_ch1_status;            /* 综合模块通道1ADC状态 0x00：正常，0x01：异常*/
    uint8_t adc_ch2_status;            /* 综合模块通道2ADC状态 */
    uint8_t adc_ch3_status;            /* 综合模块通道3ADC状态 */
    uint8_t adc_ch4_status;            /* 综合模块通道4ADC状态 */
    uint8_t dac_ch1_status;            /* 综合模块通道1DAC状态 */
    uint8_t dac_ch2_status;            /* 综合模块通道2DAC状态 */
    uint8_t dac_ch3_status;            /* 综合模块通道3DAC状态 */
    uint8_t dac_ch4_status;            /* 综合模块通道4DAC状态 */
    uint8_t sense_adc_status;          /* 综合模块感知通道ADC状态  */
    
    /* 频率源状态 */
    uint8_t freq_power_fault;          /* 频率源上电/故障指示    无符号char，1表示故障，0表示正常，默认为0。*/
    uint8_t freq_ref1_ready;           /* 频率源输出参考时钟1就绪 无符号char，1表示就绪，0表示失锁，默认为0*/
    uint8_t freq_ref2_ready;           /* 频率源输出参考时钟2就绪 */
    uint8_t freq_sense_status;         /* 频率源通信感知状态指示 */
    uint8_t freq_12v_voltage;          /* 频率源12V供电电压 对12V的偏移量，LSB表示0.1V，有符号数。例如，0x01表示12.1V，0x81表示11.9V。*/
    int8_t freq_temperature;            /* 频率源温度 有符号char型。精度为℃，取值范围-127到﹢127*/
    uint16_t freq_rs422_rx_count;      /* 频率源RS422接收消息记数 */
    uint16_t freq_rs422_tx_count;      /* 频率源RS422发送消息记数 */
    uint16_t freq_word_count;          /* 频率源频率字下发次数记数 */
    uint16_t freq_pulse_count;         /* 频率源频率更新脉冲下发次数记数 */
    uint8_t freq_ch_power_status;      /* 频率源各通道加电状态指示 */
    uint8_t freq_lo_ready;             /* 频率源各通道输出本振就绪 */
    uint16_t freq_lo1_freq;            /* 频率源本振1对应频点 */
    uint16_t freq_lo2_freq;            /* 频率源本振2对应频点 */
    uint16_t freq_lo3_freq;            /* 频率源本振3对应频点 */
    uint16_t freq_lo4_freq;            /* 频率源本振4对应频点 */
    
    /* 电源变换状态 */
    uint8_t power_power_fault;         /* 电源变换上电/故障指示 */
    uint8_t power_temperature;         /* 电源变换温度 */
    uint16_t power_ac220_power;        /* 电源变换AC220输入功耗 */
    uint16_t power_dc24v_power;        /* 电源变换DC24V输入功耗 */
    uint16_t power_rs422_rx_count;     /* 电源变换RS422接收消息记数 */
    uint16_t power_rs422_tx_count;     /* 电源变换RS422发送消息记数 */
    uint8_t power_ch_power_status;     /* 电源变换各路加电控制状态 */
    uint8_t power_fault_status;        /* 电源变换各路输出故障指示 */
    uint8_t power_overload_status;     /* 电源变换各路过载状态指示 */
    
    /* 射频前端状态 */
    uint8_t rf_power_fault;            /* 射频前端上电/故障指示 */
    uint8_t rf_28v_voltage;            /* 射频前端28V供电电压 */
    uint8_t rf_12v_voltage;            /* 射频前端12V供电电压 */
    uint8_t rf_tx_power_status;        /* 射频前端发射功率检波状态 */
    uint8_t rf_antenna_l_vswr;         /* 射频前端天线L口驻波状态 */
    uint8_t rf_antenna_h1_vswr;        /* 射频前端天线H-1口驻波状态 */
    uint8_t rf_antenna_h2_vswr;        /* 射频前端天线H-2口驻波状态 */
    uint8_t rf_tx_rx_status;           /* 射频前端收发状态指示 */
    uint8_t rf_antenna_select;         /* 射频前端天线选择控制状态 */
    uint8_t rf_sense_status;           /* 射频前端通信感知状态 */
    uint8_t rf_power_status;           /* 射频前端加电状态指示 */
    uint8_t rf_ch_power_level;         /* 射频前端各通道功率等级 */
    
    /* 射频前端通道1 */
    uint8_t rf_ch1_rf_power;           /* 射频前端通道1接收射频功率检波 */
    uint8_t rf_ch1_if_power;           /* 射频前端通道1接收中频功率检波 */
    uint8_t rf_ch1_temp1;              /* 射频前端通道1温度1 */
    uint16_t rf_ch1_freq;              /* 射频前端通道1当前工作频点 */
    uint8_t rf_ch1_agc_atten;          /* 射频前端通道1当前AGC衰减值 */
    uint16_t rf_ch1_rs422_rx_count;    /* 射频前端通道1RS422接收消息记数 */
    uint16_t rf_ch1_rs422_tx_count;    /* 射频前端通道1RS422发送消息记数 */
    uint16_t rf_ch1_freq_word_count;   /* 射频前端通道1频率字下发次数记数 */
    uint16_t rf_ch1_agc_count;         /* 射频前端通道1AGC控制下发次数记数 */
    uint16_t rf_ch1_freq_pulse_count;  /* 射频前端通道1频率更新脉冲下发次数记数 */
    uint16_t rf_ch1_power_consumption; /* 射频前端通道1通道功耗 */
    uint8_t rf_ch1_bandwidth;          /* 射频前端通道1当前信号带宽 */
    uint8_t rf_ch1_power_adjust;       /* 射频前端通道1功率调整量 */
    
    /* 射频前端通道2 */
    uint8_t rf_ch2_rf_power;
    uint8_t rf_ch2_if_power;
    uint8_t rf_ch2_temp1;
    uint16_t rf_ch2_freq;
    uint8_t rf_ch2_agc_atten;
    uint16_t rf_ch2_rs422_rx_count;
    uint16_t rf_ch2_rs422_tx_count;
    uint16_t rf_ch2_freq_word_count;
    uint16_t rf_ch2_agc_count;
    uint16_t rf_ch2_freq_pulse_count;
    uint16_t rf_ch2_power_consumption;
    uint8_t rf_ch2_bandwidth;
    uint8_t rf_ch2_power_adjust;
    
    /* 射频前端通道3 */
    uint8_t rf_ch3_rf_power;
    uint8_t rf_ch3_if_power;
    uint8_t rf_ch3_temp1;
    uint16_t rf_ch3_freq;
    uint8_t rf_ch3_agc_atten;
    uint16_t rf_ch3_rs422_rx_count;
    uint16_t rf_ch3_rs422_tx_count;
    uint16_t rf_ch3_freq_word_count;
    uint16_t rf_ch3_agc_count;
    uint16_t rf_ch3_freq_pulse_count;
    uint16_t rf_ch3_power_consumption;
    uint8_t rf_ch3_bandwidth;
    uint8_t rf_ch3_power_adjust;
    
    /* 射频前端通道4 */
    uint8_t rf_ch4_rf_power;
    uint8_t rf_ch4_if_power;
    uint8_t rf_ch4_temp1;
    uint16_t rf_ch4_freq;
    uint8_t rf_ch4_agc_atten;
    uint16_t rf_ch4_rs422_rx_count;
    uint16_t rf_ch4_rs422_tx_count;
    uint16_t rf_ch4_freq_word_count;
    uint16_t rf_ch4_agc_count;
    uint16_t rf_ch4_freq_pulse_count;
    uint16_t rf_ch4_power_consumption;
    uint8_t rf_ch4_bandwidth;
    uint8_t rf_ch4_power_adjust;
    
    /* 电池状态 */
    uint8_t battery_level;             /* 电池当前电量 */
    uint8_t battery_self_test;         /* 电池自检状态 */
    uint8_t battery_voltage;           /* 电池当前电压 */
    uint8_t battery_temperature;       /* 电池温度 */
    uint16_t battery_rs422_rx_count;   /* 电池RS422接收消息记数 */
    uint16_t battery_rs422_tx_count;   /* 电池RS422发送消息记数 */
    
    //uint8_t reserved;              /* 预留 */
}DEVICE_SC_STATUS_REPORT;

#endif


typedef struct{
	u8  veth_version[4];
	u8  agent_version[4];
	u8  ctrl_version[4];
	uint32_t  enqueue_bytes[MCS_NUM]; //每个mcs队列对应的入队列比特数
	uint32_t  outqueue_bytes[MCS_NUM]; //每个mcs队列对应的处队列比特数
	ob_state_part1 mac_information_part1; //mac层监管控信息part1
	ob_state_part2 mac_information_part2; //mac层监管控信息part2
	virt_eth_jgk_info traffic_queue_information; //业务模块监管控信息
#ifdef	Radio_SWARM_S2
	DEVICE_SC_STATUS_REPORT amp_infomation;     //add by sdg 功放数据结构
#endif
}jgk_report_infor;



struct mgmt_msg{
	uint8_t  node_id:7;
	uint8_t  mcs:4;
	uint8_t  snr:6;
	uint8_t  ucds:1;
	uint32_t  enqueue_bytes:23;
	uint32_t  outqueue_bytes:23;
};


struct mgmt_send{
	uint8_t node_id;
	uint8_t bw;
	uint16_t txpower;
	uint32_t freq;
	uint32_t neigh_num;
	uint32_t seqno;
	struct mgmt_msg msg[MCS_NUM];
};

struct mgmt_header{
	uint8_t node_id;
	uint8_t bw;
	uint16_t txpower;
	uint32_t freq;
	uint32_t neigh_num;
	uint32_t seqno;
};




//typedef struct{
//	uint32_t  enqueue_bytes[MCS_NUM]; //每个mcs队列对应的入队列比特数
//	uint32_t  outqueue_bytes[MCS_NUM]; //每个mcs队列对应的处队列比特数
//	ob_state_part1 mac_information_part1; //mac层监管控信息part1
//	ob_state_part2 mac_information_part2; //mac层监管控信息part2
//	//virt_eth_jgk_info traffic_queue_information; //业务模块监管控信息
//}jgk_report_infor11;
//
//typedef struct{
////	uint32_t  enqueue_bytes[MCS_NUM]; //每个mcs队列对应的入队列比特数
////	uint32_t  outqueue_bytes[MCS_NUM]; //每个mcs队列对应的处队列比特数
////	ob_state_part1 mac_information_part1; //mac层监管控信息part1
////	ob_state_part2 mac_information_part2; //mac层监管控信息part2
//	virt_eth_jgk_info traffic_queue_information; //业务模块监管控信息
//}jgk_report_infor12;
















#endif /* MGMT_TYPES_H_ */
