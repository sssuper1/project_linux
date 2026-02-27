/*
 * virt_eth_types.h
 *
 *  Created on: 2020-6-15
 *      Author: lu
 */

#ifndef VIRT_ETH_TYPES_H_
#define VIRT_ETH_TYPES_H_

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/of_dma.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/sched/task.h>
#include <linux/dma/xilinx_dma.h>
#include <linux/workqueue.h>

#include <linux/kernel.h>
#include <linux/rpmsg.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/atomic.h>
#include <linux/skbuff.h>
#include <linux/idr.h>

#define POWER_CHANNEL_NUM 4

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <linux/io.h>


//#define Docker_Qualnet
#define Zynq_Platform
//#define Radio_7800
//#define Radio_220

//#define Radio_SWARM_k1
//#define Radio_SWARM_k2
//#define Radio_SWARM_k4
//#define Radio_SWARM_WNW

#define Radio_SWARM_S2





#define MSG_VERSION 0x0f
#define MAX_RPMSG_BUFF_SIZE		2048

#define MCS_NUM 8


#ifdef Radio_SWARM_k1
  #define MAX_NODE 112
#elif defined Radio_220
  #define MAX_NODE 28
#elif defined Radio_7800
  #define MAX_NODE 112
#elif defined Radio_SWARM_S2
  #define MAX_NODE 56
#elif defined Radio_SWARM_k2
  #define MAX_NODE 112
#elif defined Radio_SWARM_k4
  #define MAX_NODE 112
#elif defined Radio_SWARM_WNW
  #define MAX_NODE 64
#endif

#define MAX_SLOT (56)
#define SLOT_TMP 1
#define CHECK_ID(id) (id > (MAX_NODE - 1) ? 1 : 0)

#define MAC_PKT_BUF 2048

#define UNICAST_MCS_DEFAULT     0//4
#define MULTICAST_MCS_DEFAULT   0//4
#define MULTICAST_MCS_DEFAULT_2 0//2

#define QUEUE_MAX_LEN 1200 //800
#define MAX_QUEUE_NUM 256

#ifdef Radio_SWARM_k1
#define MAC_OGM_NUM_WAIT 11
#elif defined Radio_SWARM_k2
#define MAC_OGM_NUM_WAIT 11
#elif defined  Radio_SWARM_k4
#define MAC_OGM_NUM_WAIT 11
#elif defined  Radio_SWARM_WNW 
#define MAC_OGM_NUM_WAIT 11
#elif defined Radio_220
#define MAC_OGM_NUM_WAIT 2
#elif defined Radio_7800
#define MAC_OGM_NUM_WAIT 11
#elif defined Radio_SWARM_S2
#define MAC_OGM_NUM_WAIT 11

#endif

#define TCP_LOOPBACK_TEST 0


#define MULTICAST_ADDR          0xff

#define PKT_BUF_SIZE                                          16384//4096
#define TX_BRAM_ADDR                                          0x84000000U
#define RX_BRAM_ADDR                                          0x82000000U
#define AMP_BRAM_ADDR					      0x86000000U	 //add by sdg
#define AMP_GPIO_ADDR                                         0x41200000


#define TX_PKT_BUF(n)                                         (TX_BRAM_ADDR + ((n)&0xf)*PKT_BUF_SIZE)
#define RX_PKT_BUF(n)                                         (RX_BRAM_ADDR + ((n)&0xf)*PKT_BUF_SIZE)

#define UNICAST_TX_PKT_BUF                                    4
#define MULTICAST_TX_PKT_BUF                                  7
#define NUM_TX_PKT_BUF                                        8
#if TCP_LOOPBACK_TEST
#define NUM_RX_PKT_BUF                                        101
#else
#define NUM_RX_PKT_BUF                                        100
#endif
#define NUM_RX_PKT_IQ                                         100
#define IQ_BUF_LEN                                            (32*1024)
#define UDP_IQ_PKT_LEN                                        1024

#define MULTICAST_TYPE                                     1
#define UNICAST_TYPE                                       0

#define MAC_ADDR_LEN                                       6                                  ///< MAC Address Length (in bytes)
#define ETH_ADDR_SIZE                                      6                   // Length of Ethernet MAC address (in bytes)
#define IP_ADDR_SIZE                                       4                   // Length of IP address (in bytes)


#define MAX_PKT_SIZE_B									   1600
#ifdef Docker_Qualnet
   #define WLAN_PHY_FCS_NBYTES                                0//4U
#elif defined Zynq_Platform
   #define WLAN_PHY_FCS_NBYTES                                4U
#endif

#define QUEUE_METADATA_TYPE_IGNORE                         0x00
#define QUEUE_METADATA_TYPE_STATION_INFO                   0x01
#define QUEUE_METADATA_TYPE_TX_PARAMS                      0x02

#define _impl_PASTE(a, b)                                  a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_, line)[2*!!(predicate)-1];

#define CASSERT(predicate, file)                           _impl_CASSERT_LINE(predicate, __LINE__, file)

#ifdef Radio_SWARM_k1
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        0x8
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        0x8
#elif defined Radio_SWARM_k2
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        0x8
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        0x8
#elif defined Radio_SWARM_k4
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        0x8
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        0x8
#elif defined Radio_SWARM_WNW
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        0x8
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        0x8
#elif defined Radio_220
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        32
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        32
#elif defined Radio_7800
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        0x10
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        0x10
#elif defined Radio_SWARM_S2
#define PHY_RX_PKT_BUF_PHY_HDR_SIZE                        0x10//0x8
#define PHY_TX_PKT_BUF_PHY_HDR_SIZE                        0x8

#endif




#define QUEUE_SCHEDULE 5000

#define VIRT_ETH_TEST 0
#define CDMA_AMP_LOOPBACK_TEST 0



#define UCDS_INIT_MODE 0
#define UCDS_DS_MODE   1
#define UCDS_CS_MODE   2

#define NFIX_MCS_MODE 0
#define FIX_MCS_MODE  1

#define MAX_WAITING_TIME_OGM 1000  //ms
#define MAX_WAITING_TIME_UDP 1000  //ms

   //220-28, k1-112
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

#endif

#define TARTET_TCP_ACK_NO 25

#define HOP_FREQ_NUM 32

struct virt_eth_hard_iface {
	struct list_head list;
	struct net_device *net_dev;
	struct net_device *soft_iface;
	struct packet_type virt_eth_ptype;
	int if_status;
};

enum {
	CONDITION_FALSE =0,
	CONDITION_TURE,
};
enum {
	SET_FALSE =0,
	SET_TURE,
};
enum {
	INET_FIRST_IN	= 0x01,
	INET_LAST_IN	= 0x02,
	INET_COMPLETE	= 0x04,
};

enum {
	Fragment_Initial       	= 0,
	Original_PK_Remaining,
	Fragment_Finish,
	Fragment_Error,
};
enum {
	SET_QUEUE_NUM = 0x01,
	SET_QUEUE_LENGTH = 0x02,
	SET_QOS_STATEGY = 0x04,
	SET_UNICAST_MCS = 0x08,
	SET_MULTICAST_MCS = 0x10,
	SET_FREQUENCY = 0x20,
	SET_POWER = 0x40,
	SET_BANDWIDTH = 0x80,
	SET_TEST_MODE = 0x100,
	SET_TEST_MODE_MCS = 0x200,
	SET_PHY = 0x400,
	SET_WORKMODE = 0x800,
	SET_IQ_CATCH = 0x1000,
	SET_SLOT_LEN = 0x2000,
	SET_POWER_LEVEL = 0x4000,
	SET_POWER_ATTENUATION = 0x8000,
	SET_RX_CHANNEL_MODE = 0x10000
};

enum{
	FIX_FREQ_MODE = 1,
	HOP_FREQ_MODE,
	COGNITVE_HOP_FREQ_MODE
};



typedef struct{
	u32  indication;
	u32  queue_length;
	u8   enqueue_num;
	u8   QoS_Stategy;
	u8   unicast_msc;
	u8   multicast_msc;
	u32   frequency;


	u32   bandwidth;
	u8   mode;
	u8   test_send_mode;
	u8   test_send_mode_mcs;
	u8   channel_type;
    u32   channel_rate;
	
	u16  device_mtu;
	u16   power;
	u16   tx_power_ch[POWER_CHANNEL_NUM];
	u8   device_mac[ETH_ALEN];
	u8   power_level;
	u8   power_attenuation;
	u8   rx_channel_mode;


}jgk_comm_board_parameter;

typedef struct PACKED{
	u8  frame_control;
	u8  dest_id;
	u8  src_id;
	u8  sequence_control;
	s16 rpt_sync_RD;
	u16 length_bytes;
} mac_header_cstdd;

typedef struct __attribute__ ((__packed__)){
    u8   type;
    u8   src_nodeId;
    u8   dest_nodeId;
    u8   corss_q_ind;
    u16  length;
    u16  seqno;
    u16  fragment_offset;
} llc_header_t;

typedef struct{
    u8                       mcs;                          ///< MCS index
    u8                       phy_mode;                     ///< PHY mode selection and flags
    u8                       antenna_mode;                 ///< Tx antenna selection
    s8                       power;                        ///< Tx power (in dBm)
} phy_tx_params_t;


typedef struct{
    u8                       flags;                        ///< Flags affecting waveform construction
    u8                       reserved[3];                  ///< Reserved for 32-bit alignment
} mac_tx_params_t;


typedef struct{
    phy_tx_params_t          phy;                          ///< PHY Tx params
    mac_tx_params_t          mac;                          ///< Lower-level MAC Tx params
} tx_params_t;

typedef enum __attribute__ ((__packed__)) {
   TX_PKT_BUF_UNINITIALIZED   = 0,
   TX_PKT_BUF_h_b       = 1,
   TX_PKT_BUF_h_u       = 2,
   TX_PKT_BUF_l_b       = 3,
   TX_PKT_BUF_l_u       = 4,
   TX_PKT_BUF_beacon    = 5,
} tx_pkt_buf_state_t;

typedef enum __attribute__ ((__packed__)) {
   RX_PKT_BUF_UNINITIALIZED   = 0,
   RX_PKT_BUF_HIGH_CTRL       = 1,
   RX_PKT_BUF_READY           = 2,
   RX_PKT_BUF_LOW_CTRL        = 3
} rx_pkt_buf_state_t;


typedef enum __attribute__ ((__packed__)){
	PKT_BUF_GROUP_GENERAL		= 0,
	PKT_BUF_GROUP_DTIM_MCAST    = 1,
	PKT_BUF_GROUP_OTHER 		= 0xFF,
} pkt_buf_group_t;
CASSERT(sizeof(pkt_buf_group_t) == 1, pkt_buf_group_t_alignment_check);

typedef struct {
    u8                      id;                     	  ///< ID of the Queue
    pkt_buf_group_t         pkt_buf_group;                ///< Packet Buffer Group
    u16                     occupancy;                    ///< Number of elements in the queue when the                                                         ///<   packet was enqueued (including itself)
} tx_queue_details_t;
CASSERT(sizeof(tx_queue_details_t) == 4, tx_queue_details_t_alignment_check);

typedef struct{
    u64                      	timestamp_create;             ///< MAC timestamp of packet creation
    //----- 8-byte boundary ------
    u32                      	delay_accept;                 ///< Time in microseconds between timestamp_create and packet acceptance by CPU Low
    u32                      	delay_done;                   ///< Time in microseconds between acceptance and transmit completion
    //----- 8-byte boundary ------
    u64                      	unique_seq;                   ///< Unique sequence number for this packet (12 LSB used as 802.11 MAC sequence number)
    //----- 8-byte boundary ------
    tx_queue_details_t       	queue_info;                   ///< Information about the TX queue used for the packet (4 bytes)
    u16                       	num_tx_attempts;              ///< Number of transmission attempts for this frame
    u8                       	tx_result;                    ///< Result of transmission attempt - TX_MPDU_RESULT_SUCCESS or TX_MPDU_RESULT_FAILURE
    u8                       	reserved;
    //----- 8-byte boundary ------
    volatile tx_pkt_buf_state_t tx_pkt_buf_state;             ///< State of the Tx Packet Buffer
    u8                       	flags;                        ///< Bit flags en/disabling certain operations by the lower-level MAC
    u8                       	phy_samp_rate;                ///< PHY Sampling Rate
    u8                       	padding0;                     ///< Used for alignment of fields (can be appropriated for any future use)

    u16                      	length;                       ///< Number of bytes in MAC packet, including MAC header and FCS
    u16                      	ID;                           ///< Station ID of the node to which this packet is addressed
    //----- 8-byte boundary ------

    //
    // Place additional fields here.  Make sure the new fields keep the structure 8-byte aligned
    //

    //----- 8-byte boundary ------
    tx_params_t              params;                       ///< Additional lower-level MAC and PHY parameters (8 bytes)
} tx_frame_info_t;

typedef struct {
    u8                       mcs;
    u8                       phy_mode;
    u8                       reserved[2];
    u16                      length;
    u16                      N_DBPS;                       ///< Number of data bits per OFDM symbol
} phy_rx_details_t;

typedef struct __attribute__ ((__packed__)){
    u64                      tx_start_timestamp_mpdu;
    u64                      tx_start_timestamp_ctrl;
    phy_tx_params_t          phy_params_mpdu;
    phy_tx_params_t          phy_params_ctrl;

    u8                       tx_details_type;
    u8                       chan_num;
    u16                      duration;

    s16                      num_slots;
    u16                      cw;

    u8                       tx_start_timestamp_frac_mpdu;
    u8                       tx_start_timestamp_frac_ctrl;
    u8                       src;
    u8                       lrc;

    u16                      ssrc;
    u16                      slrc;

    u8						 flags;
    u8						 reserved;
    u16						 attempt_number;

} wlan_mac_low_tx_details_t;
CASSERT(sizeof(wlan_mac_low_tx_details_t) == 44, wlan_mac_low_tx_details_t_alignment_check);

typedef struct __attribute__ ((__packed__)){
    u8                       	  	flags;                        ///< Bit flags
    u8                       	  	ant_mode;                     ///< Rx antenna selection
    s8                       	 	rx_power;                     ///< Rx power, in dBm
    u8                       	 	rf_gain;                      ///< Gain setting of radio Rx LNA, in [0,1,2]
    u8                       	  	bb_gain;                      ///< Gain setting of radio Rx VGA, in [0,1,...31]
    u8                       	  	channel;                      ///< Channel index
    volatile rx_pkt_buf_state_t     rx_pkt_buf_state;             ///< State of the Rx Packet Buffer
    u8                       	  	adopted_power_scheme;
    //----- 8-byte boundary ------
    u32                      	  	cfo_est;                      ///< Carrier Frequency Offset Estimate
    u32							  	reserved1;
    //----- 8-byte boundary ------
    phy_rx_details_t         	  	phy_details;                  ///< Details from PHY used in this reception
    //----- 8-byte boundary ------
    u8                       	  	timestamp_frac;               ///< Fractional timestamp beyond usec timestamp for time of reception
    u8                       	  	phy_samp_rate;                ///< PHY Sampling Rate
    u8                       	  	reserved2[2];                 ///< Reserved bytes for alignment
    u32                      	  	additional_info;              ///< Field to hold MAC-specific info, such as a pointer to a station_info
    //----- 8-byte boundary ------
    wlan_mac_low_tx_details_t     	resp_low_tx_details;			///< Tx Low Details for a control resonse (e.g. ACK or CTS)
    u32								reserved3;
    //----- 8-byte boundary ------
    u64                      	  	timestamp;                    ///< MAC timestamp at time of reception
    //----- 8-byte boundary ------
    u32                      	  	channel_est[64];              ///< Rx PHY channel estimates
} rx_frame_info_t;
// The above structure assumes that pkt_buf_state_t is a u8.  However, that is architecture dependent.
// Therefore, the code will check the size of the structure using a compile-time assertion.  This check
// will need to be updated if fields are added to the structure
//
CASSERT(sizeof(rx_frame_info_t) == 344, rx_frame_info_alignment_check);

typedef struct{
	u8    metadata_type;
	u8    reserved[3];
	u32   metadata_ptr;
} tx_queue_metadata_t;


typedef struct{
	tx_queue_metadata_t   metadata;
	u16                   tx_offset;
	u16                   total_len;
	u16                   seqno;
	u8                    flags;
	u8                    other;
	tx_frame_info_t       tx_frame_info;
#ifdef Zynq_Platform
	u8                    phy_hdr_pad[PHY_TX_PKT_BUF_PHY_HDR_SIZE];
#endif
	u8                    frame[0];
} tx_queue_buffer_t;

typedef enum {LinkSt_h1,LinkSt_h2,LinkSt_idle} nbr_link;


typedef struct{
//	u8  packet_class[3]; //0x01_0xaa_0xbb
//	u8  node_id;
	s16 time_jitter[NET_SIZE];
	u8  snr [NET_SIZE];
	u8  rssi[NET_SIZE];
	u8  mcs [NET_SIZE];
	s16 good[NET_SIZE];
	s16 bad [NET_SIZE];
	u8  ucds[NET_SIZE];
	u8  noise[NET_SIZE];
}ob_state_part2;


typedef struct{
//	u8  packet_class[3]; //0x03_0xcc_0x04
//	u8  node_id;
	u8  nbr_list[NET_SIZE];
	u8  slot_list[NET_SIZE*2];
	u8  n_used_l0;
	u8  n_free_hx;
	u8  n_ol0_hx;
	u8  n_free_h1;
	u8  n_ol0_h1;
	u8  n_free_h2;
	u8  n_ol0_h2;
	u8  n_used_l1;
	u8  ctf_live_num;
	u8  tsn_avgload_demand;
	u8  tsn_traffic_demand;
	u8  reserved;
}ob_state_part1;



#define PHY_RX_PKT_BUF_PHY_HDR_OFFSET                     (sizeof(rx_frame_info_t))
#define PHY_TX_PKT_BUF_PHY_HDR_OFFSET                     (sizeof(tx_frame_info_t))

#define PHY_RX_PKT_BUF_MPDU_OFFSET                        (PHY_RX_PKT_BUF_PHY_HDR_SIZE + PHY_RX_PKT_BUF_PHY_HDR_OFFSET)
#define PHY_TX_PKT_BUF_MPDU_OFFSET                        (PHY_TX_PKT_BUF_PHY_HDR_SIZE + PHY_TX_PKT_BUF_PHY_HDR_OFFSET)




struct _rpmsg_eptdev {
	struct device dev;
	struct cdev cdev;
	wait_queue_head_t usr_wait_q;
	struct rpmsg_device *rpdev;
	struct rpmsg_channel_info chinfo;
	struct rpmsg_endpoint *ept;
	spinlock_t queue_lock;
	struct sk_buff_head queue;
	bool is_sk_queue_closed;
	wait_queue_head_t readq;
	char tx_buff[MAX_RPMSG_BUFF_SIZE];
	u32 rpmsg_dst;
	int err_cnt;
	struct work_struct rpmsg_work;
};


typedef struct{
	u8 rx_pkt_buf;
	u8 resv;
	u16 buf_len;
}virt_eth_pkt_ready;

typedef struct{
	struct work_struct worker;
	struct net_device *dev;
}virt_eth_work_tx_poll;

typedef struct{
	struct work_struct worker;
	struct net_device *dev;
	virt_eth_pkt_ready veth_rx_param;
}virt_eth_work_rx_poll;

typedef struct{
	struct delayed_work delayed_work;
	struct net_device *dev;
}virt_eth_work_slot_num;

typedef struct{
	struct delayed_work delayed_work;
	struct net_device *dev;
}virt_eth_work_jgk;

typedef struct{
	struct delayed_work delayed_work;
	struct net_device *dev;
	u8*                pu8Pkt;
	u32                u32PktLen;
}virt_eth_work_iq;

typedef struct{
	u16 tx_in[MAX_QUEUE_NUM];
	u16 tx_qlen[MAX_QUEUE_NUM];
	u32 tx_inall;
	u32 tx_outall;
	u32 tx_in_lose;
	u32 tx_out_lose;
	u32 rx_inall;
	u32 rx_outall;
	u32 rx_in_lose;
	u32 rx_out_lose;
	u32 mac_list_tx_cnt;
	u32 tx_in_cnt;
	u32 phy_tx_done_cnt;
	u32 phy_rx_done_cnt;
	u32 ogm_in;
	u32 ogm_in_len;
	u32 ogm_slot;
	u32 ogm_out_len;
	u32 ping_in;
	u32 ping_in_len;
	u32 ping_slot;
	u32 ping_out_len;
	u32 bcast_in;
	u32 bcast_in_len;
	u32 bcast_slot;
	u32 bcast_out_len;
	u32 ucast_in;
	u32 ucast_in_len;
	u32 ucast_slot;
	u32 ucast_out_len;
}virt_eth_jgk_info;

typedef struct{
	struct sk_buff_head txq;
	int    total_len;
	//tx_queue_buffer_t tx_info;
}virt_eth_tx_queue;

typedef struct{
	u8   mcs_value[MAX_NODE];

	u32  pkt_enq_bytes_tmp[MCS_NUM];
	u32  pkt_enq_bytes[MCS_NUM];
	u32  pkt_outq_bytes[MCS_NUM];
	u32  flow_stat_offset[MCS_NUM];
	u16  TX_AMPDU_LEM_MAX_1[MCS_NUM];
	u16  TX_AMPDU_LEM_MAX_2[MCS_NUM];
	u16  TX_AMPDU_LEM_MAX_3[MCS_NUM];
	u8   q_hffull_flag;
	u16  eth_data_seqno;
	u8   numb;
}virt_eth_traffic_parameter;

typedef struct{
	struct work_struct worker;
	struct net_device *dev;
	u8 manage_array[10];
}virt_eth_work_manage;


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
	u8   veth_version[4];
	u8   agent_version[4];
	u8   ctrl_version[4];
	u32  enqueue_bytes[MCS_NUM];
	u32  outqueue_bytes[MCS_NUM];
	ob_state_part1 mac_information_part1;
	ob_state_part2 mac_information_part2;
	virt_eth_jgk_info traffic_queue_information;
#ifdef	Radio_SWARM_S2
	DEVICE_SC_STATUS_REPORT amp_infomation;     //add by sdg 功放数据结构
#endif
}jgk_report_infor;

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
	u32 trig_mode;
	u32 catch_addr;
	u32 catch_length;
}jgk_set_iq_catch;

typedef struct{
	u32  indication;
	u32  queue_length;
	u8   enqueue_num;
	u8   QoS_Stategy;
	u8   unicast_msc;
	u8   multicast_msc;
	u16   frequency;
	u16   power;
	u16   power_ch[POWER_CHANNEL_NUM];
	u8   bandwidth;
	u8   test_send_mode;
	u8   test_send_mode_mcs;
	u8   res;
//	u8   NET_work_mode;
	Smgmt_net_work_mode work_mode_msg;
	jgk_set_iq_catch iq_catch;
	Smgmt_phy phy_msg;
	u8  u8Slotlen;
	u8 power_level;
	u8 power_attenuation;
	u8 rx_channel_mode;
	u8 resv;
}jgk_set_parameter;
//typedef struct{
//	tx_queue_metadata_t   metadata;
//	u16                   tx_offset;
//	u16                   total_len;
//	u16                   seqno;
//	u8                    flags;
//	u8                    other;
//	tx_frame_info_t       tx_frame_info;
//	u8                    phy_hdr_pad[PHY_TX_PKT_BUF_PHY_HDR_SIZE];
//	u8                    frame[MAX_PKT_SIZE_B];
//} tx_queue_buffer_t;


struct virt_eth_priv{
	u8 addr[MAC_ADDR_LEN];
	struct net_device *soft_dev;
	struct hlist_head station_list;
	spinlock_t station_lock;
	spinlock_t dma_tx_lock;
	spinlock_t dma_rx_lock;
	struct rpmsg_device *rpmsg_dev;
	atomic_t slot_num_time;
	struct delayed_work station_work;
	struct delayed_work queue_work;

	struct workqueue_struct *virt_data_event_workqueue;
	struct workqueue_struct *virt_mgmt_event_workqueue;
	virt_eth_jgk_info v_jgk_info;
	virt_eth_traffic_parameter virt_traffic_param;

    u8  ucds_mode;
	u8  mcs_mode;
	u8  ucast_mcs;
	u8  bcast_mcs;
	u32 neighbor_num;
	virt_eth_tx_queue virt_tx_queue[MAX_QUEUE_NUM];
	virt_eth_tx_queue virt_rx_queue[MAX_QUEUE_NUM];

#ifdef Radio_220
    u8 channel_num;
#endif
//	virt_eth_work_tx_poll v_work_tx;
//	virt_eth_work_rx_poll v_work_rx;
//	virt_eth_work_slot_num v_work_slot_num;
};



#endif /* VIRT_ETH_TYPES_H_ */
