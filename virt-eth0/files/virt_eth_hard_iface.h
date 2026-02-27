/*
 * virt_eth_hard_iface.h
 *
 *  Created on: Jun 18, 2020
 *      Author: slb
 */

#ifndef VIRT_ETH_HARD_IFACE_H_
#define VIRT_ETH_HARD_IFACE_H_


extern struct notifier_block virt_eth_hard_if_notifier;
void virt_eth_hardif_remove_interfaces(struct net_device *soft_iface);
int virt_eth_hardif_enable_interface(struct virt_eth_hard_iface *hard_iface,
				   struct net *net, const char *iface_name);
struct virt_eth_hard_iface *virt_eth_hardif_get_by_netdev(struct net_device *soft_interface,const struct net_device *net_dev);
struct virt_eth_hard_iface *virt_eth_hardif_get_netdev(struct net_device *soft_iface);
struct virt_eth_hard_iface *virt_eth_hardif_add_interface(struct net_device *soft_interface,struct net_device *net_dev);
#endif /* VIRT_ETH_HARD_IFACE_H_ */
