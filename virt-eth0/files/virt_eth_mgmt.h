/*
 * virt_eth_mgmt.h
 *
 *  Created on: 2020-4-14
 *      Author: lu
 */

#ifndef VIRT_ETH_MGMT_H_
#define VIRT_ETH_MGMT_H_

#include <linux/types.h>
#include "virt_eth_types.h"

#define VIRT_ETH_MGMT_RX_READY_LEN 4

#define BATMAN_ADV_ROUTE_INFO_TYPE 0x06
#define BATMAN_ADV_VERSION 0x0f

typedef enum{
	MGMT_MAC_SETTING = 0x01,
	MGMT_MAC_SETTING_REPLY,
	MGMT_PARAM_QUERY,
	MGMT_PARAM_REPORT,
    	MGMT_TX_READY,
    	MGMT_RX_READY,
    	MGMT_TX_DONE,
    	MGMT_SLOT_NUM,
    	MGMT_IQR_START,
//	MGMT_TX_DATA,
//	MGMT_RX_DATA
}E_MGMT_MSG;

typedef enum{
	soft_id = 0x01,
	freq,
	bw,
	power,
	mcs_mode,
	mac_mode,
	route,
	jgk,
	rx_dma,
	phy_msg,
	freq_hop,
	iq_catch,
	slotlen,
	power_level,
	power_attenuation,
	rx_channel_mode
}E_MGMT_PARAM_MSG;

typedef enum{
	MCS_MODE = 0x00,
	PHYDLY_SLOT,
	TRX_FREQ,
	CHANNEL_BW,
	TXFIFO_DLY,
	RTS_SW,
	TX_ATTEN,
	PRINT_EN,
	ACK_MODE,
	TEST_MODE
}E_MGMT_MANAGE;

typedef struct{
	u8 type;
	u8 len;
}S_PARAM_MSG;

typedef struct{
	S_PARAM_MSG msg;
	u8 id;
	u8 resv;
}S_PARAM_SOFT_ID;

typedef struct{
	S_PARAM_MSG msg;
	u16 freq;
}S_PARAM_FREQ;

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	u16 tx1a_power;
	u16 tx1b_power;
	u16 tx2a_power;
	u16 tx2b_power;
}S_PARAM_POWER;

typedef struct{
	S_PARAM_MSG msg;
	u8 bw;
	u8 resv;
}S_PARAM_BW;

typedef struct{
	S_PARAM_MSG msg;
	u8 mac_mode;
	u8 resv;
}S_PARAM_MAC_MODE;

typedef struct{
	S_PARAM_MSG msg;
	Smgmt_phy phy_msg;
}S_PARAM_PHY_MSG;

typedef struct{
	S_PARAM_MSG msg;
	u8 mcs_mode;
	u8 resv;
}S_PARAM_MCS_MODE;

typedef struct{
#if TCP_LOOPBACK_TEST
	dma_addr_t rx_dma_addr[NUM_RX_PKT_BUF-1];
#else
	dma_addr_t rx_dma_addr[NUM_RX_PKT_BUF];
#endif
}S_PARAM_RX_DMA;



typedef struct{
	u8 forword_link_quality[MAX_NODE];
	u8 reply_broadcast_enable[MAX_NODE];
	u8 ucds_state;
	u8 resv[3];
}S_ROUTE_INFO;

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	S_ROUTE_INFO route_info;
}S_PARAM_ROUTE;

typedef struct __attribute__ ((__packed__)){
	S_ROUTE_INFO route_info;
	u32 mac_list_tx_cnt;
	u32 tx_in_cnt;
	u32 phy_tx_done_cnt;
	u32 phy_rx_done_cnt;
}S_TEST_INFO;


typedef struct{
	u8 slot_num;
	u8 resv[3];
}S_PARAM_SLOT_NUM;

typedef struct{
	u8  type;
	u8  version;
	u8  param_num;
	u8  len;
}S_MGMT_MSG;

typedef struct{
	u8           type;
	u8           version;
	u8           ttl;
	u8           flags;
	u32          len;
	S_ROUTE_INFO route_info;
}S_BATMAN_ADV_ROUTE_INFO;

#ifdef Radio_SWARM_k1 
typedef enum{
	bw20m = 0,
	bw10m,
	bw5m,
	bw3m,
	bw_1p2m
}E_BW_NUM;
#elif defined Radio_220
typedef enum{
	bw20m = 0,
	bw10m,
	bw5m,
	bw3m,
	bw_1p2m
}E_BW_NUM;
#elif defined Radio_SWARM_S2
typedef enum{
	bw20m = 0,
	bw10m,
	bw5m,
	bw3m,
	bw_1p2m
}E_BW_NUM;
#elif defined Radio_7800
typedef enum{
	bw40m = 0,
	bw20m,
	bw10m,
	bw5m,
	bw3m,
	bw_1p2m
}E_BW_NUM;
#else
	typedef enum{
	bw20m = 0,
	bw10m,
	bw5m,
	bw3m,
	bw_1p2m
}E_BW_NUM;
#endif

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	u8 fh_mode;
	u8 fh_len;
	u32 hop_freq_tb[32];
}S_PARAM_FREQ_HOP;

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	u8 trig_mode;
	u8 resv;
	u32 catch_addr;
	u32 catch_length;
}S_PARAM_IQ_CATCH;

typedef struct c{
	S_PARAM_MSG msg;
	u8 u8Slotlen;
	u8 resv;
}S_PARAM_SLOT_LEN;

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	u8 level;
	u8 resv;
}S_PARAM_POWER_LEVEL;

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	u8 attenuation;
	u8 resv;
}S_PARAM_POWER_ATTENUATION;

typedef struct __attribute__ ((__packed__)){
	S_PARAM_MSG msg;
	u8 mode;
	u8 resv;
}S_PARAM_RX_CHANNEL_MODE;





void set_TX_AMPDU_LEM_MAX_1(struct virt_eth_priv *veth_priv,int mode);
void set_TX_AMPDU_LEM_MAX_2(struct virt_eth_priv *veth_priv,u8 u8Bw,u8 u8Slotlen);
u8 virt_eth_mgmt_init(struct net_device *dev);
u8 virt_eth_mgmt_get_mcs(struct virt_eth_priv *veth_priv, u8 node_id);
u16 virt_eth_mgmt_get_mcs_len_by_id(struct virt_eth_priv *veth_priv, u8 node_id);
u16 virt_eth_mgmt_get_mcs_len_by_mcs(struct virt_eth_priv *veth_priv,u8 mcs);
void virt_eth_mgmt_send_msg(struct net_device *dev,E_MGMT_MSG type,void* data,u8 len);
u8 virt_eth_mgmt_get_buffer(void);
int virt_eth_mgmt_recv(struct net_device *dev,u8* data,u32 len);
void virt_eth_mgmt_slot_schedule(struct virt_eth_priv *veth_priv);
void virt_eth_mgmt_manage(struct work_struct *work);
void virt_eth_mgmt_jgk_info(struct net_device *dev,void* data,u32 len);
#endif /* VIRT_ETH_MGMT_H_ */
