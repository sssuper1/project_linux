/*
 * virt_eth_system.h
 *
 *  Created on: 2020-4-20
 *      Author: lu
 */

#ifndef VIRT_ETH_SYSTEM_H_
#define VIRT_ETH_SYSTEM_H_

#include <linux/types.h>
#include <linux/netdevice.h>
#include "virt_eth_types.h"


u8 virt_eth_system_init(struct net_device *dev);
#ifdef Docker_Qualnet
void virt_eth_system_set_priv(struct net_device *dev);
#elif defined Zynq_Platform
void virt_eth_system_set_priv(struct net_device *dev,struct rpmsg_device *rpmsg_dev);
#endif
u8 virt_eth_system_rx_data_process(struct net_device *dev,u8* data,u32 len);
void virt_eth_system_tx_done(struct net_device *dev);
void virt_eth_system_param_set_report(void);

#endif /* VIRT_ETH_SYSTEM_H_ */
