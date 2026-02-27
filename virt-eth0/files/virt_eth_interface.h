/*
 * virt_eth_interface.h
 *
 *  Created on: 2020-6-15
 *      Author: lu
 */

#ifndef VIRT_ETH_INTERFACE_H_
#define VIRT_ETH_INTERFACE_H_

#include "virt_eth_types.h"
#include <net/rtnetlink.h>
#include "virt_eth_util.h"

extern struct rtnl_link_ops virt_eth_link_ops;
struct net_device *virt_eth_softif_create(struct net *net, const char *name);


#endif /* VIRT_ETH_INTERFACE_H_ */
