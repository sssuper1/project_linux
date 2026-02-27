/*
 * virt_eth_station.h
 *
 *  Created on: 2020-4-17
 *      Author: lu
 */

#ifndef VIRT_ETH_STATION_H_
#define VIRT_ETH_STATION_H_

#include <linux/types.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/netdevice.h>

#include "virt_eth_types.h"

#define STATION_ONLINE  1
#define STATION_OFFLINE 0

typedef struct{
	struct hlist_node list;
	struct virt_eth_priv *veth_priv;
	unsigned long last_seen;
	u8 addr[MAC_ADDR_LEN];
	u8 id;
	u8 mcs;
	u8 state;


}virt_station_info;

u8 virt_eth_station_init(struct net_device *dev);
u8 virt_eth_station_add(u8 id,u8* mac_addr,u8 mcs,struct virt_eth_priv *veth_priv);
u8 virt_eth_station_del(virt_station_info * vstation);
virt_station_info* virt_eth_station_find_id(u8 id,struct virt_eth_priv *veth_priv);
u8 virt_eth_station_set_mcs(u8 mcs,virt_station_info * vstation);
u8 virt_eth_station_rx_process(u8* mac_addr,struct net_device *dev);
u8 virt_eth_station_reset(void);
void virt_eth_station_timeout(struct work_struct *work);
void virt_eth_station_schedule(struct virt_eth_priv *veth_priv);

#endif /* VIRT_ETH_STATION_H_ */
