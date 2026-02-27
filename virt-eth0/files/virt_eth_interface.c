/*
 * virt_eth_interface.c
 *
 *  Created on: 2020-6-15
 *      Author: lu
 */

#include "virt_eth_interface.h"

#include <linux/atomic.h>
#include <linux/bug.h>
#include <linux/byteorder/generic.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/fs.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/rculist.h>
#include <linux/rcupdate.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>
#include <linux/workqueue.h>

#include "virt_eth_hard_iface.h"
#include "virt_eth_system.h"
#include "virt_eth_queue.h"
#include "virt_eth_util.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_station.h"
#include "virt_eth_comm_board.h"
#include "virt_eth_jgk.h"

#define NET_MAX_MTU 1560

static int virt_eth_softif_init_late(struct net_device *dev)
{

	return 0;
}

static int virt_eth_interface_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

static int virt_eth_interface_release(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static struct net_device_stats *virt_eth_interface_stats(struct net_device *dev)
{
//	struct batadv_priv *bat_priv = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;
//
//	stats->tx_packets = batadv_sum_counter(bat_priv, BATADV_CNT_TX);
//	stats->tx_bytes = batadv_sum_counter(bat_priv, BATADV_CNT_TX_BYTES);
//	stats->tx_dropped = batadv_sum_counter(bat_priv, BATADV_CNT_TX_DROPPED);
//	stats->rx_packets = batadv_sum_counter(bat_priv, BATADV_CNT_RX);
//	stats->rx_bytes = batadv_sum_counter(bat_priv, BATADV_CNT_RX_BYTES);
	return stats;
}

static int virt_eth_interface_add_vid(struct net_device *dev, __be16 proto,
				    unsigned short vid)
{
	return 0;
}

static int virt_eth_interface_kill_vid(struct net_device *dev, __be16 proto,
				     unsigned short vid)
{
	return 0;
}

static int virt_eth_interface_set_mac_addr(struct net_device *dev, void *p)
{
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	ether_addr_copy(dev->dev_addr, addr->sa_data);
	ether_addr_copy(veth_priv->addr, addr->sa_data);


	virt_eth_jgk_init(veth_priv);
	//set schedule timer 
	virt_eth_system_set_priv(dev);
	/* only modify transtable if it has been initialized before */
	return 0;
}

static int virt_eth_interface_change_mtu(struct net_device *dev, int new_mtu)
{
	if ((new_mtu < 68) || (new_mtu >= NET_MAX_MTU))
		return -EINVAL;

	dev->mtu = new_mtu;

	return 0;
}

static void virt_eth_interface_set_rx_mode(struct net_device *dev)
{
}

static int virt_eth_interface_tx(struct sk_buff *skb,
			       struct net_device *soft_iface)
{

//	u8 ret = 0;
	//printk("virt tx iface %pM\n",skb->dev->dev_addr);
//	ret = virt_eth_util_encap(skb,soft_iface);
//	if(ret != 0)
//		dev_kfree_skb (skb);


	struct virt_eth_priv *veth_priv = netdev_priv(soft_iface);
	struct virt_eth_hard_iface *hard_iface;
	int ret;
	//struct net_device *soft_iface2 = dev_get_by_name(&init_net,"eth1");
	//struct net_device *soft_iface2;
	
//	if(veth_priv->slave_dev != NULL)
//		skb->dev = veth_priv->slave_dev;
//	else
//		goto send_error;
	hard_iface = virt_eth_hardif_get_netdev(soft_iface);
	if(hard_iface != NULL){
		skb->dev = hard_iface->net_dev;
	}
	else
		goto send_error;

//	printk("virt tx iface %pM\n",skb->dev->dev_addr);

	if (jgk_information_data[veth_priv->addr[5]-1].mac_information_part1.node_id != veth_priv->addr[5])
	{
		jgk_information_data[veth_priv->addr[5]-1].mac_information_part1.node_id = veth_priv->addr[5];
		printk("set node_id =%d,veth_priv->addr[5] = %d\n ",jgk_information_data[veth_priv->addr[5]-1].mac_information_part1.node_id,veth_priv->addr[5]);
	}
	//soft_iface2 = dev_get_by_index(&init_net,ifindex);

	//printk("ifindex %d get by name soft_iface  %x, soft_iface %x\n",ifindex,(int)soft_iface2,(int)soft_iface);

     ret = virt_eth_util_encap(skb,soft_iface);
     if(ret == 0)
    	 return 0;

//	ret = dev_queue_xmit(skb);
//	return net_xmit_eval(ret);
//    return 0;

//
send_error:
	kfree_skb(skb);
	return 0;

}

static int virt_eth_softif_slave_add(struct net_device *dev,
				   struct net_device *slave_dev)
{
	struct virt_eth_hard_iface *hard_iface;
	struct net *net = dev_net(dev);
	int ret = -EINVAL;
	printk("virt_eth_softif_slave_add \n");
	if(dev == NULL){
		printk("virt_eth_softif_slave_add error\n");
		goto out;
	}
	hard_iface = virt_eth_hardif_get_by_netdev(dev,slave_dev);
	if (!hard_iface || hard_iface->soft_iface){
		if(!hard_iface){
			hard_iface = virt_eth_hardif_add_interface(dev,slave_dev);
			if(!hard_iface)
				goto out;
		}
		else
			goto out;
	}
	ret = virt_eth_hardif_enable_interface(hard_iface, net, dev->name);
	//printk("get ifindex\n");
	//ifindex = dev_get_iflink(dev);
	//printk("ifindex =%d\n",ifindex);

out:
	return ret;
}

static int virt_eth_softif_slave_del(struct net_device *dev,
				   struct net_device *slave_dev)
{
	return 0;
}
//
static const struct net_device_ops virt_eth_netdev_ops = {
	.ndo_init = virt_eth_softif_init_late,
	.ndo_open = virt_eth_interface_open,
	.ndo_stop = virt_eth_interface_release,
	.ndo_get_stats = virt_eth_interface_stats,
	.ndo_vlan_rx_add_vid = virt_eth_interface_add_vid,
	.ndo_vlan_rx_kill_vid = virt_eth_interface_kill_vid,
	.ndo_set_mac_address = virt_eth_interface_set_mac_addr,
	.ndo_change_mtu = virt_eth_interface_change_mtu,
	.ndo_set_rx_mode = virt_eth_interface_set_rx_mode,
	.ndo_start_xmit = virt_eth_interface_tx,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_add_slave = virt_eth_softif_slave_add,
	.ndo_del_slave = virt_eth_softif_slave_del,
};

static void virt_eth_softif_free(struct net_device *dev)
{
//	batadv_debugfs_del_meshif(dev);
//	batadv_mesh_free(dev);

	/* some scheduled RCU callbacks need the bat_priv struct to accomplish
	 * their tasks. Wait for them all to be finished before freeing the
	 * netdev and its private data (bat_priv)
	 */
	rcu_barrier();
}

static void virt_eth_softif_init_early(struct net_device *dev)
{
	int ret;
	ether_setup(dev);

	dev->netdev_ops = &virt_eth_netdev_ops;
	dev->needs_free_netdev = true;
	dev->priv_destructor = virt_eth_softif_free;
	dev->features |= NETIF_F_HW_VLAN_CTAG_FILTER | NETIF_F_NETNS_LOCAL;
	dev->priv_flags |= IFF_NO_QUEUE;

	/* can't call min_mtu, because the needed variables
	 * have not been initialized yet
	 */
	dev->mtu = 10000;

	/* generate random address */
	eth_hw_addr_random(dev);

	printk("virt_eth_softif_init_early \n");
	//initial the jgk, station, queue and mcs
	ret = virt_eth_system_init(dev);
	if(ret != 0)
		printk("virt_eth_system_init error \n");
	ret = virt_eth_station_init(dev);
	if(ret != 0)
		printk("virt_eth_station_init error \n");
	ret = virt_eth_queue_init(dev);
	if(ret != 0)
		printk("virt_eth_queue_init error \n");
	ret = virt_eth_mgmt_init(dev);
	if(ret != 0)
		printk("virt_eth_mgmt_init error \n");



//	dev->ethtool_ops = &batadv_ethtool_ops;
}


struct net_device *virt_eth_softif_create(struct net *net, const char *name)
{
	struct net_device *soft_iface;
	int ret;
		
	soft_iface = alloc_netdev(sizeof(struct virt_eth_priv), name,
				  NET_NAME_UNKNOWN, virt_eth_softif_init_early);
	if (!soft_iface)
		return NULL;


	dev_net_set(soft_iface, net);

	soft_iface->rtnl_link_ops = &virt_eth_link_ops;

	ret = register_netdevice(soft_iface);
	if (ret < 0) {
		pr_err("Unable to register the batman interface '%s': %i\n",
		       name, ret);
		free_netdev(soft_iface);
		return NULL;
	}

	return soft_iface;
}

static void virt_eth_softif_destroy_netlink(struct net_device *soft_iface,
					  struct list_head *head)
{
//	struct virt_eth_priv *bat_priv = netdev_priv(soft_iface);
//	struct batadv_hard_iface *hard_iface;
//	struct batadv_softif_vlan *vlan;
//
//	list_for_each_entry(hard_iface, &batadv_hardif_list, list) {
//		if (hard_iface->soft_iface == soft_iface)
//			batadv_hardif_disable_interface(hard_iface,
//							BATADV_IF_CLEANUP_KEEP);
//	}
//
//	/* destroy the "untagged" VLAN */
//	vlan = batadv_softif_vlan_get(bat_priv, BATADV_NO_FLAGS);
//	if (vlan) {
//		batadv_softif_destroy_vlan(bat_priv, vlan);
//		batadv_softif_vlan_put(vlan);
//	}
//
//	batadv_sysfs_del_meshif(soft_iface);
//	unregister_netdevice_queue(soft_iface, head);
}


struct rtnl_link_ops virt_eth_link_ops __read_mostly = {
	.kind		= "virteth",
	.priv_size	= sizeof(struct virt_eth_priv),
	.setup		= virt_eth_softif_init_early,
	.dellink	= virt_eth_softif_destroy_netlink,
};

