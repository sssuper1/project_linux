/*
 * virt_eth_comm_board.h
 *
 *  Created on: 2020-5-15
 *      Author: lu
 */

#ifndef VIRT_ETH_COMM_BOARD_H_
#define VIRT_ETH_COMM_BOARD_H_

#include <linux/types.h>
#include <linux/if_ether.h>

#define ETH_P_CTL       0x9000
#define ETH_P_CTL_ROUTE 0x9002

//Q:query
//A:answer
//S:set
//G:get
//R:report
#define PACKED __attribute__((packed))





enum CTL_TYPE{
	Q_CHANNEL_CONFIG_INFO = 0x01,
	A_CHANNEL_CONFIG_INFO = 0x02,
	S_CHANNEL_CONFIG_INFO = 0x03,
	G_CHANNEL_CONFIG_INFO = 0x04,
	Q_CHANNEL_QUALITY_INFO = 0x05,
	A_CHANNEL_QUALITY_INFO = 0x06,
	R_QOS_QUEUE_OCCUPY = 0x07,
	Q_QOS_QUEUE_INFO = 0x08,
	A_QOS_QUEUE_INFO = 0x09,
	Q_CHANNEL_LDLE_RETIO_MSG = 0x0a,
	A_CHANNEL_LDLE_RETIO_MSG = 0x0b,
	Q_PRIORITY_R_S_PKG_NUM = 0x0c,
	A_PRIORITY_R_S_PKG_NUM = 0x0d,
	Q_PRIORITY_OVERFLOW_PKG_NUM = 0x0e,
	A_PRIORITY_OVERFLOW_PKG_NUM = 0x0f,
	R_DEVICE_ADDR_CONFLICT_MSG = 0x10,
	A_DEVICE_ADDR_CONFLICT_MSG = 0x11,
	R_DEVICE_EXCEPTION_MSG = 0x12,
	A_DEVICE_EXCEPTION_MSG = 0x13,
	Q_ROUTE_MAP_MSG = 0x14,
	A_ROUTE_MAP_MSG = 0x15,
	S_GATEWAY_YES_OR_NO = 0x16,
	Q_GATEWAY_STATUS_MSG = 0x17,
	A_GATEWAY_STATUS_MSG = 0x18,
	S_IP_ADDR = 0x19,
	R_GATEWAY_MSG = 0x1a,
};



//struct gateway_msg{
//	u8 status;
//};

struct route_msg{
	__be32 ip;
	__be16 mask_len;
	__be16 metrics;
};

struct route_map_msg{
//	u8     packet_type;
//	__be16 packet_len;
	u8     seqno;
	u8     type;
	__be16 num;

};

struct device_exception_msg{
	u8     packet_type;
	__be16 packet_len;
	u8     exception_msg[];
};

struct __attribute__ ((__packed__)) priority_overflow_pkg_num{
//	u8     packet_type;
//	__be16 packet_len;
	__be32 queue1_overflow_num;
	__be32 queue2_overflow_num;
	__be32 queue3_overflow_num;
};

struct __attribute__ ((__packed__)) priority_r_s_pkg_num{
//	u8     packet_type;
//	__be16 packet_len;
	__be32 queue1_s_num;
	__be32 queue1_r_num;
	__be32 queue2_s_num;
	__be32 queue2_r_num;
	__be32 queue3_s_num;
	__be32 queue3_r_num;
};

struct __attribute__ ((__packed__)) channel_ldle_retio_msg{
//	u8     packet_type;
//	__be16 packet_len;
	__be16 channel_ldle_retio;
};

struct __attribute__ ((__packed__)) channel_config_info{
	u8     packet_type;
	__be16 packet_len;
	u8     device_mac[ETH_ALEN];
	u8     channel_type;
	__be32 channel_rate;
	__be16 device_mtu;
	__be32 channel_freq;
	__be32 channel_bw;
	u8     mode;
};

struct __attribute__ ((__packed__)) channel_quality_info{
	u8     packet_type;
	__be16 packet_len;
	__be16 node_num;
	u8     node_name[ETH_ALEN];
	u8     node_quality;
	u8     node_quality_n[7];
};

struct __attribute__ ((__packed__)) qos_queue_occupy{
//	u8     packet_type;
//	__be16 packet_len;
	__be16 queue1_occupy;
	__be16 queue2_occupy;
	__be16 queue3_occupy;
};

struct __attribute__ ((__packed__)) qos_queue_info{
//	u8     packet_type;
//	__be16 packet_len;
	__be16 queue1_occupy;
	__be16 queue2_occupy;
	__be16 queue3_occupy;
};

struct __attribute__ ((__packed__)) report_no_msg{
	u8     packet_type;
	__be16 packet_len;
};

struct batadv_comm_msg{
	u8     packet_type;
	u8     version;
	u8     ttl;
	u8     flags;
	__be32 len;
};

typedef struct{
	struct delayed_work delayed_work;
	enum CTL_TYPE type;
	struct net_device *dev;
}virt_eth_comm_work;

u8 virt_eth_comm_queue_warning(struct net_device *dev);
u8 virt_eth_do_comm_board(struct net_device *dev,struct sk_buff *skb,u32 len);

#endif /* VIRT_ETH_COMM_BOARD_H_ */
