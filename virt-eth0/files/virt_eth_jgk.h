/*
 * virt_eth_jgk.h
 *
 *  Created on: 2020-5-16
 *      Author: lu
 */

#ifndef VIRT_ETH_JGK_H_
#define VIRT_ETH_JGK_H_

#include "virt_eth_types.h"

#ifdef Docker_Qualnet
extern u8  jgk_set_parameter_flag[NET_SIZE];
extern jgk_report_infor jgk_information_data[NET_SIZE];
extern jgk_comm_board_parameter jgk_comm_board_parameter_data[NET_SIZE];
#elif defined Zynq_Platform
extern u8  jgk_set_parameter_flag;
extern jgk_report_infor jgk_information_data;
extern jgk_comm_board_parameter jgk_comm_board_parameter_data;
extern u32 start_tcp_seq_no;
extern u8  g_u8Slotlen;

#endif
//extern u32 ifindex; 
void virt_eth_jgk_init(struct net_device *dev);
void virt_eth_jgk_schedule(struct virt_eth_priv *veth_priv);
void virt_eth_jgk_iq_schedule(struct virt_eth_priv *veth_priv,u8* pu8Pkt,u32 u32PktLen);

#endif /* VIRT_ETH_JGK_H_ */
