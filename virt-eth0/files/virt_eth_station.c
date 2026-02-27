/*
 * virt_eth_station.c
 *
 *  Created on: 2020-4-17
 *      Author: lu
 */


#include "virt_eth_station.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_queue.h"

#define STATION_TIME_OUT 5000
#define STATION_SCHEDULE 2000

u8 virt_eth_station_init(struct net_device *dev){

	return 0;
}





u8 virt_eth_station_add(u8 id,u8* mac_addr,u8 mcs,struct virt_eth_priv *veth_priv){
	virt_station_info * vstation = NULL;

	if(veth_priv == NULL)
		return 1;

	if(CHECK_ID(id))
		return 1;

	spin_lock_bh(&veth_priv->station_lock);

	vstation = kmalloc(sizeof(*vstation), GFP_ATOMIC);
	vstation->id = id;
#if VIRT_ETH_TEST
	vstation->mcs = UNICAST_MCS_DEFAULT;
#else
	vstation->mcs = mcs;
#endif
	if(vstation->mcs < MCS_NUM){
		vstation->state = STATION_ONLINE;
		vstation->last_seen = jiffies;
	}
	else
		vstation->state = STATION_OFFLINE;
	vstation->veth_priv = veth_priv;
	memcpy(vstation->addr,mac_addr,MAC_ADDR_LEN);
	hlist_add_head_rcu(&vstation->list, &veth_priv->station_list);
	spin_unlock_bh(&veth_priv->station_lock);

	return 0;
}

u8 virt_eth_station_del(virt_station_info * vstation){
	if(vstation == NULL)
		return 1;

	spin_lock_bh(&vstation->veth_priv->station_lock);
	hlist_del_rcu(&vstation->list);
	spin_unlock_bh(&vstation->veth_priv->station_lock);
	return 0;
}

virt_station_info* virt_eth_station_find_id(u8 id,struct virt_eth_priv *veth_priv){

	virt_station_info *pos, *station = NULL;

	rcu_read_lock();
	hlist_for_each_entry_rcu(pos, &veth_priv->station_list, list) {
		if(pos->id == id){
			station = pos;
			break;
		}
	}
	rcu_read_unlock();

	return station;


}

u8 virt_eth_station_set_mcs(u8 mcs,virt_station_info * vstation){
	if(vstation == NULL)
		return 1;

	vstation->mcs = mcs;

	return 0;
}

u8 virt_eth_station_rx_process(u8* mac_addr,struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	virt_station_info* station_info;
	station_info = virt_eth_station_find_id(mac_addr[5],veth_priv);
	if(station_info == NULL){


		//////
		return 0;
	}

	return 0;
}



u8 virt_eth_station_reset(void){
	return 0;
}


void virt_eth_station_timeout(struct work_struct *work)
{
	struct virt_eth_priv *veth_priv;
	struct delayed_work *delayed_work;
	virt_station_info * vstation = NULL;

	delayed_work = to_delayed_work(work);
	veth_priv = container_of(delayed_work, struct virt_eth_priv,
			station_work);

	rcu_read_lock();
	hlist_for_each_entry_rcu(vstation, &veth_priv->station_list, list) {
		if(vstation == NULL)
		{
			break;
		}
		if(CHECK_ID(vstation->id)){
			virt_eth_station_del(vstation);
			continue;
		}
		if(vstation->state == STATION_OFFLINE){
			if(time_is_before_jiffies(vstation->last_seen + msecs_to_jiffies(5000))){
				virt_eth_queue_del_all(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(vstation->id));
				virt_eth_queue_del_all(veth_priv,STATION_ID_TO_QUEUE_ID(vstation->id));
			}
		}
	}
	rcu_read_unlock();

	virt_eth_station_schedule(veth_priv);
}

void virt_eth_station_schedule(struct virt_eth_priv *veth_priv){
	unsigned long time_delay = msecs_to_jiffies(STATION_SCHEDULE);

	queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			   &veth_priv->station_work,
			   time_delay);
}

