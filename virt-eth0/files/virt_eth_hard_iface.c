/*
 * virt_eth_hard_iface.c
 *
 *  Created on: Jun 18, 2020
 *      Author: slb
 */

#include "virt_eth.h"
#include "virt_eth_types.h"
#include "virt_eth_hard_iface.h"
#include "virt_eth_interface.h"



#include <linux/atomic.h>
#include <linux/bug.h>
#include <linux/byteorder/generic.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/rculist.h>
#include <linux/rtnetlink.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <net/net_namespace.h>
#include <net/rtnetlink.h>

static bool virt_eth_is_valid_iface(const struct net_device *net_dev)
{
	if (net_dev->flags & IFF_LOOPBACK)
		return false;

	if (net_dev->type != ARPHRD_ETHER)
		return false;

	if (net_dev->addr_len != ETH_ALEN)
		return false;

	return true;
}

void virt_eth_hardif_remove_interfaces(struct net_device *soft_iface)
{
	struct virt_eth_hard_iface *hard_iface, *hard_iface_tmp;
	struct virt_eth_priv *veth_priv = netdev_priv(soft_iface);

	rtnl_lock();
	list_for_each_entry_safe(hard_iface, hard_iface_tmp,
				 &veth_priv->virt_eth_hardif_list, list) {
		list_del_rcu(&hard_iface->list);
		kfree(hard_iface);
	}
	rtnl_unlock();
}

static int virt_eth_master_del_slave(struct virt_eth_hard_iface *slave,
				   struct net_device *master)
{
	int ret;

	if (!master)
		return 0;

	ret = -EBUSY;
	if (master->netdev_ops->ndo_del_slave)
		ret = master->netdev_ops->ndo_del_slave(master, slave->net_dev);

	return ret;
}

struct virt_eth_hard_iface *
virt_eth_hardif_get_netdev(struct net_device *soft_iface)
{
	struct virt_eth_hard_iface *hard_iface;
	struct virt_eth_priv *veth_priv = netdev_priv(soft_iface);

	rcu_read_lock();
	list_for_each_entry_rcu(hard_iface, &veth_priv->virt_eth_hardif_list, list) {
		if (hard_iface->net_dev != NULL && hard_iface->if_status == 1 && hard_iface->soft_iface == soft_iface)
			goto out;
	}

	hard_iface = NULL;

out:
	rcu_read_unlock();
	return hard_iface;
}

struct virt_eth_hard_iface *
virt_eth_hardif_get_by_netdev(struct net_device *soft_interface,const struct net_device *net_dev)
{
	struct virt_eth_hard_iface *hard_iface;
	struct virt_eth_priv *veth_priv = netdev_priv(soft_interface);
	if(veth_priv == NULL){
		printk("virt_eth_hardif_get_by_netdev error\n");
		return NULL;
	}

	rcu_read_lock();
	list_for_each_entry_rcu(hard_iface, &veth_priv->virt_eth_hardif_list, list) {
		if (hard_iface->net_dev == net_dev)
			goto out;
	}

	hard_iface = NULL;

out:
	rcu_read_unlock();
	return hard_iface;
}

int virt_eth_hardif_enable_interface(struct virt_eth_hard_iface *hard_iface,
				   struct net *net, const char *iface_name)
{
	struct virt_eth_priv *veth_priv;
	struct net_device *soft_iface, *master;
	__be16 ethertype = htons(ETH_P_BATMAN);
	int ret;



	soft_iface = dev_get_by_name(net, iface_name);

	if (!soft_iface) {
		soft_iface = virt_eth_softif_create(net, iface_name);

		if (!soft_iface) {
			ret = -ENOMEM;
			goto err;
		}

		/* dev_get_by_name() increases the reference counter for us */
		dev_hold(soft_iface);
	}



//	printk("bat name %s %pM\n",iface_name,soft_iface->dev_addr);
	/* check if the interface is enslaved in another virtual one and
	 * in that case unlink it first
	 */
	master = netdev_master_upper_dev_get(hard_iface->net_dev);
	ret = virt_eth_master_del_slave(hard_iface, master);
	if (ret)
		goto err_dev;

	hard_iface->soft_iface = soft_iface;
	veth_priv = netdev_priv(hard_iface->soft_iface);

	ret = netdev_master_upper_dev_link(hard_iface->net_dev,
					   soft_iface, NULL, NULL);
	if (ret)
		goto err_dev;


	hard_iface->virt_eth_ptype.type = ethertype;
	hard_iface->virt_eth_ptype.func = virt_eth_skb_recv;
	hard_iface->virt_eth_ptype.dev = hard_iface->net_dev;
	dev_add_pack(&hard_iface->virt_eth_ptype);
	hard_iface->if_status = 1;




out:
	return 0;

err_dev:
	hard_iface->soft_iface = NULL;
	dev_put(soft_iface);
err:
	return ret;
}

struct virt_eth_hard_iface *
virt_eth_hardif_add_interface(struct net_device *soft_interface,struct net_device *net_dev)
{
	struct virt_eth_hard_iface *hard_iface;
	struct virt_eth_priv *veth_priv = netdev_priv(soft_interface);
	int ret;

	ASSERT_RTNL();

	if (!virt_eth_is_valid_iface(net_dev))
		goto out;

	dev_hold(net_dev);

	hard_iface = kzalloc(sizeof(*hard_iface), GFP_ATOMIC);
	if (!hard_iface)
		goto release_dev;

	hard_iface->net_dev = net_dev;
	hard_iface->soft_iface = NULL;
	hard_iface->if_status = 0;
//	printk("virt add iface %pM\n",net_dev->dev_addr);


	INIT_LIST_HEAD(&hard_iface->list);
	list_add_tail_rcu(&hard_iface->list, &veth_priv->virt_eth_hardif_list);

	return hard_iface;

release_dev:
	dev_put(net_dev);
out:
	return NULL;
}

static int virt_eth_hard_if_event(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	struct net_device *net_dev = netdev_notifier_info_to_dev(ptr);
	struct virt_eth_hard_iface *hard_iface;



//	hard_iface = virt_eth_hardif_get_by_netdev(net_dev);
//	if (!hard_iface && (event == NETDEV_REGISTER ||
//			    event == NETDEV_POST_TYPE_CHANGE))
//		hard_iface = virt_eth_hardif_add_interface(net_dev);
//
//	if (!hard_iface)
//		goto out;



out:
	return NOTIFY_DONE;
}

struct notifier_block virt_eth_hard_if_notifier = {
	.notifier_call = virt_eth_hard_if_event,
};
