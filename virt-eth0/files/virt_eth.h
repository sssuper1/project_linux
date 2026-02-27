/*
 * main.h
 *
 *  Created on: 2020-6-15
 *      Author: lu
 */

#ifndef VIRT_ETH_H_
#define VIRT_ETH_H_

#include "virt_eth_types.h"


int virt_eth_skb_recv(struct sk_buff *skb, struct net_device *dev,
			   struct packet_type *ptype,
			   struct net_device *orig_dev);

#endif /* MAIN_H_ */
