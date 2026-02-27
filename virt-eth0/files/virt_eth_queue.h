/*
 * virt_eth_queue.h
 *
 *  Created on: 2020-4-15
 *      Author: lu
 */

#ifndef VIRT_ETH_QUEUE_H_
#define VIRT_ETH_QUEUE_H_
#include <linux/skbuff.h>
#include <linux/types.h>
#include "virt_eth_types.h"


struct virt_eth_hashtable {
	struct hlist_head *table;   /* the hashtable itself with the buckets */
	spinlock_t *list_locks;     /* spinlock for each hash list entry */
	u32 size;		    /* size of hashtable */
};

#define MCAST_QID                                          0
#define MCAST_TCP_QID                                      1
#define BEACON_QID                                         2
#define MANAGEMENT_QID                                     3
#define STATION_ID_TO_QUEUE_ID(x)                                    ((x+x) + 3)   ///map association ID to Tx queue ID; min AID is 1
#define STATION_ID_TO_TCP_QUEUE_ID(x)                                (STATION_ID_TO_QUEUE_ID(x)-1) //
#define CHECK_UDP_QUEUE(x)                                           ( (x>MANAGEMENT_QID) && ((x-MANAGEMENT_QID)%2 == 0) )
#define CHECK_TCP_QUEUE(x)                                           ( (x>MANAGEMENT_QID) && ((x-MANAGEMENT_QID)%2 == 1) )
#define QUEUE_ID_TO_STATION_ID(x)                                    ( (x>MANAGEMENT_QID)*((x-MANAGEMENT_QID)/2) )

extern u8 VIRT_ETH_BRAM_NUM[];

void virt_eth_queue_hash_init(struct virt_eth_hashtable *hash);
void virt_eth_queue_hash_destroy(struct virt_eth_hashtable *hash);
u8 virt_eth_queue_init(struct net_device *dev);
struct sk_buff* virt_eth_queue_create_skb(u32 len);
int virt_eth_queue_get_id(void);
void virt_eth_queue_del_all(struct virt_eth_priv *veth_priv, u8 txq_id);
struct sk_buff* virt_eth_queue_tx_dequeue(struct virt_eth_priv *veth_priv, u8 txq_id);
void virt_eth_queue_tx_add_head(struct virt_eth_priv *veth_priv, u8 txq_id,struct sk_buff* skb);
u16 virt_eth_queue_get_length(struct virt_eth_priv *veth_priv, u16 id);
u32 virt_eth_queue_get_tx_length(struct virt_eth_priv *veth_priv, u8 nodeid);
//u8 virt_eth_queue_dequeue_transmit(struct net_device *dev,,u8 node_id,u16 txq_id,u8 mcs);
#ifdef Docker_Qualnet
void virt_eth_poll_tx_queue(struct net_device * dev);
#elif defined Zynq_Platform
void virt_eth_poll_tx_queue(struct work_struct *work);
#endif



u8 virt_eth_queue_tail(struct virt_eth_priv *veth_priv, struct sk_buff * skb,u8 reserved,u8 id);
u8 virt_eth_queue_get_rx_length(struct virt_eth_priv *veth_priv, u8 nodeid);
u8 virt_eth_queue_remove_rx_first(struct virt_eth_priv *veth_priv, u8 nodeid);
struct sk_buff* virt_eth_queue_find_by_id(struct virt_eth_priv *veth_priv, u8 nodeid,u16 seqno);
void virt_eth_queue_del_by_id(struct virt_eth_priv *veth_priv, u8 nodeid,u16 seqno);
void virt_eth_add_rx_queue(struct virt_eth_priv *veth_priv, u8 nodeid,struct sk_buff* skb);
u8 virt_eth_queue_dequeue_transmit(struct net_device *dev,u8 node_id,u16 txq_id,u8 mcs);
void virt_eth_rx_queue_timeout(struct work_struct *work);
void virt_eth_rx_queue_schedule(struct virt_eth_priv *veth_priv);
int virt_eth_queue_get_larger_mcs_txqueue_total_length(struct net_device *dev, u8 mcs);
u16 virt_eth_get_cross_queue_id(struct virt_eth_priv *veth_priv,u16 txq_id, u8 mcs, u8 * cross_queue_list);
#endif /* VIRT_ETH_QUEUE_H_ */
