/*
 * main.c
 *
 *  Created on: 2020-6-15
 *      Author: lu
 */

#ifndef VIRT_ETH_C_
#define VIRT_ETH_C_

#include "virt_eth.h"

#include <linux/atomic.h>
#include <linux/bug.h>
#include <linux/byteorder/generic.h>
#include <linux/crc32c.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/genetlink.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/printk.h>
#include <linux/rculist.h>
#include <linux/rcupdate.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <net/dsfield.h>
#include <net/rtnetlink.h>

#include "virt_eth_types.h"
#include "virt_eth_interface.h"
#include "virt_eth_hard_iface.h"
#include "virt_eth_system.h"
#include "virt_eth_queue.h"
#include "virt_eth_util.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_station.h"
#include "virt_eth_comm_board.h"
#include "virt_eth_jgk.h"

//struct workqueue_struct *virt_eth_event_workqueue;
const  u8                    def_mac[MAC_ADDR_LEN] = {0xb8,0x8e,0xdf,0x00,0x01,0x01};
const  u8                    def_multicast_mac[MAC_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

int virt_eth_skb_recv(struct sk_buff *skb, struct net_device *dev,
			   struct packet_type *ptype,
			   struct net_device *orig_dev)
{
	//dev = eth0
	u8 * data = (u8*)skb->data; //skb->data is pointed to the header of mac_header_cstdd
	struct virt_eth_priv *veth_priv;
#if VIRT_ETH_TEST
	//tx_frame_info_t *rx_frame_info = (tx_frame_info_t*)data;
#else
	//rx_frame_info_t *rx_frame_info = (rx_frame_info_t*)data;
#endif
	mac_header_cstdd *rxcstdd_hdr;
	llc_header_t*       llc_header;
	void*               mac_payload;
	u16 data_offset = 0;
	u8 dest_mac[MAC_ADDR_LEN];
	u8 src_mac[MAC_ADDR_LEN];
	u8 unicast_to_me,to_multicast;
	u8					pre_llc_offset			 = 0;
	u16 				length				     = 0;
	int i;
	u8*    pkt;
	struct virt_eth_hard_iface *hard_iface;
	struct sk_buff *skb_send;
	struct ethhdr *ethhdr = eth_hdr(skb);
	udp_header_t* uhdr;
	ipv4_header_t* ihdr;

	hard_iface = container_of(ptype, struct virt_eth_hard_iface,
			virt_eth_ptype);

	skb->dev = hard_iface->soft_iface;


	//check if the information is MAC JGK data
	ihdr = (ipv4_header_t*)skb->data;
	//printk("ethhdr->h_proto = 0x%x,skb->len = %d\n",ntohs(ethhdr->h_proto),skb->len);

	if (ihdr->protocol == IPV4_PROT_UDP)
	{
		uhdr = (udp_header_t*)(skb->data+sizeof(ipv4_header_t));
		printk("uhdr->dest_port = %d\n",ntohs(uhdr->dest_port));
		if (uhdr->dest_port == ntohs(7123))
		{

			//update jgk information
			virt_eth_mgmt_jgk_info(hard_iface->soft_iface,skb->data+sizeof(ipv4_header_t)+sizeof(udp_header_t),
				skb->len-sizeof(ipv4_header_t)-sizeof(udp_header_t));

			dev_kfree_skb(skb);
			return 0;
		}
	}


	//	printk("virt rx iface %pM\n",dev->dev_addr);

//	i= skb->len+sizeof(ethernet_header_t)-sizeof(mac_header_cstdd)-sizeof(llc_header_t);
//	skb_send = virt_eth_queue_create_skb( i );
//	pkt = skb_put( skb_send,i );
//    skb_send->dev = hard_iface->soft_iface;
//    memcpy( pkt,(void *)ethhdr,ETH_HLEN );
//    memcpy( pkt+ETH_HLEN,skb->data+sizeof(mac_header_cstdd)+sizeof(llc_header_t),\
//    		skb->len-sizeof(mac_header_cstdd)-sizeof(llc_header_t) );
//    skb_send->protocol = eth_type_trans(skb_send, hard_iface->soft_iface);
//    skb_send->ip_summed = CHECKSUM_UNNECESSARY;
//
//	netif_rx_ni(skb_send);
//	dev_kfree_skb(skb);
//	return NET_RX_SUCCESS;


	veth_priv = netdev_priv(hard_iface->soft_iface); //hard_iface->soft_iface = eth1
//	printk("virt_eth_skb_recv,veth_priv mac address is %x , %x, %x, %x, %x, %x\n",veth_priv->addr[0],veth_priv->addr[1],veth_priv->addr[2],\
//			veth_priv->addr[3],	veth_priv->addr[4],veth_priv->addr[5]);
	//////////////////
#if VIRT_ETH_TEST
	rxcstdd_hdr = (mac_header_cstdd*)(data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
	mac_payload = data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE;
#else
	rxcstdd_hdr = (mac_header_cstdd*)data ;
	mac_payload = data ;
#endif
//	if(skb->len>100)
//	{
//			for(i = 0 ; i < sizeof(mac_header_cstdd)+sizeof(llc_header_t); i ++)
//			{
//				printk("skb->data[%d],%x mac_payload[%d],%x,",i,skb->data[i],i,*((u8*)mac_payload+i) );
//			}
//			printk("skb->len %d\n",skb->len);
//	}

	//////////////////

//	rxcstdd_hdr = (mac_header_cstdd*)((u8*)rx_frame_info + PHY_RX_PKT_BUF_MPDU_OFFSET);
//	mac_payload = (u8*)rx_frame_info + PHY_RX_PKT_BUF_MPDU_OFFSET;

//	if(veth_priv->addr[5] ==2)
//	{
//		printk("node %d rx_process %d %d\n",veth_priv->addr[5],rxcstdd_hdr->dest_id,rxcstdd_hdr->src_id);
//	}

	//get destination node's mac address
	if(rxcstdd_hdr->dest_id == 0xff)
	{
		memcpy(dest_mac,def_multicast_mac,MAC_ADDR_LEN);
	}
	else{
		memcpy(dest_mac,def_mac,MAC_ADDR_LEN);
		dest_mac[5] = rxcstdd_hdr->dest_id;
	}
	//get source node's mac address
	if(rxcstdd_hdr->src_id == 0xff)
	{
		memcpy(src_mac,def_multicast_mac,MAC_ADDR_LEN);
	}
	else{
		memcpy(src_mac,def_mac,MAC_ADDR_LEN);
		src_mac[5] = rxcstdd_hdr->src_id;
	}

	//check if the packet is sent to me
	unicast_to_me = wlan_addr_eq(dest_mac, veth_priv->addr);
	to_multicast  = wlan_addr_eq(dest_mac,def_multicast_mac);
//	printk("virt_eth_skb_recv,dest_mac address is %x , %x, %x, %x, %x, %x\n",dest_mac[0],dest_mac[1],dest_mac[2],\
//			dest_mac[3],	dest_mac[4],dest_mac[5]);
	//////////////
	virt_eth_station_rx_process(src_mac,dev); //?

	//check if the packet's destiantion is me or destination address is broadcast/mulitcat address
	if(unicast_to_me || to_multicast)
	{
#if VIRT_ETH_TEST
		data_offset = rx_frame_info->length;
#else
		//record the remaining unpack length except the ethernet header
		data_offset = skb->len;
#endif
		//

		i=0;
		while(1)
		{
//			i++;
//			if(data_offset > 100 )
//			printk("node %d,do while data_offset %d, cycle %d\n",veth_priv->addr[5],data_offset,i);
			//check if there is packet to unpack
			if(data_offset <= (sizeof(mac_header_cstdd) + sizeof(llc_header_t)))
				break;
			//get llc_header
			llc_header = (llc_header_t*)(mac_payload+sizeof(mac_header_cstdd) + pre_llc_offset);
			//length contain mac_header_cstdd + llc_header+ payoff
			length = llc_header->length;

//			if(data_offset > 100)
//			printk("node %d,llc_header length=%d, data_offset=%d\n",veth_priv->addr[5],length,data_offset);


			//check if length is correct
			if((length < sizeof(mac_header_cstdd)+sizeof(llc_header_t)) || (length > data_offset)/* || ((u32)mac_payload - ((u32)mac_payload_ptr_u8) +  length >= PKT_BUF_SIZE)*/)
			{
//									printk("llc_header---- length-%d %d\n",length,data_offset);
				veth_priv->v_jgk_info.rx_out_lose ++;
				break;
			}

//			if(llc_header->nodeId != dest_mac[5])
//			{
//				printk("error id %d %d\n",llc_header->nodeId , dest_mac[5]);
//				break;
//			}

			//unpack this fragment for this llc



			virt_eth_util_send_data(mac_payload, length, pre_llc_offset,src_mac,dest_mac,hard_iface->soft_iface);
			//left shift sizeof(mac_header_cstdd), due to the following fragment don't have mac_header_cstdd,
			//the operation is to guarantee mac_payload+sizeof(mac_header_cstdd) to get llc header for the next cycle
			length -= sizeof(mac_header_cstdd);
			data_offset -= length;
//								memcpy(mac_payload + length,(u8*)&rx_80211,DEL_DATA_LEN);


//								Xil_DCacheInvalidateRange((INTPTR)((void*)mac_payload + length),DEL_DATA_LEN);
			// right shift the length
			mac_payload += length;

		}

//		while(1){
//			virt_eth_frame_decode();
//
//			virt_eth_util_send_data();
//		}
	}else{
		if(skb->len > 1000){
//			printk("not me\n");
			veth_priv->v_jgk_info.rx_out_lose ++;
		}
	}
		dev_kfree_skb(skb);
return 0;

//	struct virt_eth_priv *veth_priv;
//	struct virt_eth_hard_iface *hard_iface;
//	u8 idx;
//
////	hard_iface = container_of(ptype, struct virt_eth_hard_iface,
////			virt_eth_ptype);
//
//
//	skb = skb_share_check(skb, GFP_ATOMIC);
//
//	/* skb was released by skb_share_check() */
//	if (!skb)
//		goto err_out;
//
//	/* packet should hold at least type and version */
//	if (unlikely(!pskb_may_pull(skb, 2)))
//		goto err_free;
//
//	/* expect a valid ethernet header here. */
//	if (unlikely(skb->mac_len != ETH_HLEN || !skb_mac_header(skb)))
//		goto err_free;
//
////	if(!hard_iface){
////		printk("bat hard_iface error\n");
////		goto err_free;
////	}
////
////	if (!hard_iface->soft_iface)
////		goto err_free;
//
//
////	veth_priv = netdev_priv(hard_iface->soft_iface);
//	hard_iface = virt_eth_hardif_get_by_netdev(dev);
//	skb->dev = hard_iface->soft_iface;
//
////	printk("virt rx iface %pM\n",dev->dev_addr);
//
//	netif_rx_ni(skb);
//	/* return NET_RX_SUCCESS in any case as we
//	 * most probably dropped the packet for
//	 * routing-logical reasons.
//	 */
//	return NET_RX_SUCCESS;
//
//err_free:
//	kfree_skb(skb);
//err_out:
//	return NET_RX_DROP;
}

static int __init virt_eth_init(void)
{
	int ret,i = 0;


	//virt_eth_hardif_remove_interfaces();
//	virt_eth_event_workqueue = create_singlethread_workqueue("veth_events");
//	if (!virt_eth_event_workqueue)
//		goto err_create_wq;

//	register_netdevice_notifier(&virt_eth_hard_if_notifier);
	rtnl_link_register(&virt_eth_link_ops);

//	batadv_netlink_register();



	return 0;

err_create_wq:

	return -ENOMEM;
}

static void __exit virt_eth_exit(void)
{
	rtnl_link_unregister(&virt_eth_link_ops);
//	unregister_netdevice_notifier(&virt_eth_hard_if_notifier);

//	rm virt_eth_hardif_list

//	flush_workqueue(virt_eth_event_workqueue);
//	destroy_workqueue(virt_eth_event_workqueue);
//	virt_eth_event_workqueue = NULL;

//	rcu_barrier();

}

module_init(virt_eth_init);
module_exit(virt_eth_exit);

//MODULE_DESCRIPTION("Sample driver to exposes rpmsg svcs to userspace via a char device");
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("virteth");

#endif /* MAIN_C_ */
