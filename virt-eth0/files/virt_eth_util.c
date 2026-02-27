/*
 * virt_eth_util.c
 *
 *  Created on: 2020-4-15
 *      Author: lu
 */


#include <linux/if_ether.h>
#include <linux/jiffies.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>

#include "virt_eth_types.h"
#include "virt_eth_util.h"
#include "virt_eth_queue.h"
#include "virt_eth_frame.h"
#include "virt_eth_dma.h"
#include "virt_eth_station.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_comm_board.h"
#include "virt_eth_jgk.h"

#define MAX_GROUP_NUM 10

static u8 test_mac[MAC_ADDR_LEN];
const  u8                    def_mac1[MAC_ADDR_LEN] = {0xb8,0x8e,0xdf,0x00,0x01,0x01};
const  u8                    def_multicast_mac1[MAC_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};



static void virt_eth_test_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
    unsigned char *type;
    struct iphdr *ih;
    __be32 *saddr, *daddr, tmp;
    unsigned char    tmp_dev_addr[ETH_ALEN];
    struct ethhdr *ethhdr;
//    int i = 0;
//    char* data;

short port = 0;

//    struct sk_buff *rx_skb;
ipv4_header_t* ihdr;
udp_header_t* uhdr;

//struct sk_buff *rx_skb = dev_alloc_skb(skb->len);
//memcpy(skb_put(rx_skb, skb->len), skb->head, skb->len);
   // 从硬件读出/保存数据
   /* 对调"源/目的"的mac地址 */
    ethhdr = (struct ethhdr *)skb->data;
    memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
    memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
    memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);

    /* 对调"源/目的"的ip地址 */
    ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;


    tmp = *saddr;
    *saddr = *daddr;
    *daddr = tmp;

ihdr = (ipv4_header_t*)(skb->data + sizeof(struct ethhdr));
if(ihdr->protocol == 0x01)
{
    //((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
    //((u8 *)daddr)[2] ^= 1;
    type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
//    printk("tx package type = %02x\n", *type);
    // 修改类型, 原来0x8表示ping
    *type = 0; /* 0表示reply */

}
else if(ihdr->protocol == 0x11)
{
uhdr = (udp_header_t*)((void*)ihdr + sizeof(ipv4_header_t));
port = uhdr->src_port;
uhdr->src_port = uhdr->dest_port;
uhdr->dest_port = 0x1027;
uhdr->checksum = 0;
}
    ih->check = 0;           /* and rebuild the checksum (ip needs it) */
//    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);



    // 构造一个sk_buff
//    rx_skb = dev_alloc_skb(skb->len + 2);
//    skb_reserve(rx_skb, 2); /* align IP on 16B boundary */
//    memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);
//
//    /* Write metadata, and then pass to the receive level */
//    rx_skb->dev = dev;
//    rx_skb->protocol = eth_type_trans(rx_skb, dev);
//    rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
//    dev->stats.rx_packets++;
//    dev->stats.rx_bytes += skb->len;

    // 提交sk_buff
    skb->dev = dev;
    skb->protocol = eth_type_trans(skb, dev);
    skb->ip_summed = CHECKSUM_UNNECESSARY;
//    data = skb->data;
//    for(i = 0; i < skb->len; i ++)
//    {
//    	printk("%x ",*data);
//    	data++;
//    }
//    printk("\n");


    netif_rx_ni(skb);
//    dev_kfree_skb (skb);
}

static void getNewNo(struct virt_eth_priv *veth_priv)
{
	if(veth_priv->virt_traffic_param.eth_data_seqno == 0xffff)
		veth_priv->virt_traffic_param.eth_data_seqno = 0;
	else veth_priv->virt_traffic_param.eth_data_seqno ++;
}

int virt_eth_skb_head_push(struct sk_buff *skb,unsigned int len){
	int result;
	result = skb_cow_head(skb, len);
	if (result < 0)
		return result;

	skb_push(skb, len);
	return 0;
}
//add tx_queue_buffer_t, mac_header_cstdd and llc_header to received packet
u8 virt_eth_util_create_data_frame(struct sk_buff *skb,struct virt_eth_priv *veth_priv, llc_header_t *llc,u8* h_source,u8* h_dest,u16 tx_length){
	int ret = 0;
	int i;
	mac_header_cstdd *data_header;
	tx_queue_buffer_t        * tx_queue_buffer_local;
#ifdef Docker_Qualnet
	u8 ethernet_header_old[sizeof(ethernet_header_t)];
	memcpy(ethernet_header_old,skb->data,sizeof(ethernet_header_t));
#endif
	ret = virt_eth_skb_head_push(skb,ETH_PAYLOAD_OFFSET + sizeof(tx_queue_buffer_t));
	if(ret < 0)
		return 1;
	//put tx queue buffer information to the first in the frame, this information will not send through wireless channel
	tx_queue_buffer_local = (tx_queue_buffer_t*)(void*)skb->data;
#ifdef Docker_Qualnet
	// put the received ethernet header information after tx tx queue buffer information
	// In DQ platform, this is first part sent through wireless channel
	//get ethernet header from old position
	memcpy( skb->data + sizeof(tx_queue_buffer_t),ethernet_header_old,sizeof(ethernet_header_t) );
	//put mac_header_cstdd information after ethernet header
	data_header = (mac_header_cstdd*)((void*)skb->data + sizeof(tx_queue_buffer_t) + sizeof(ethernet_header_t) );
#elif defined Zynq_Platform
	data_header = (mac_header_cstdd*)((void*)skb->data + sizeof(tx_queue_buffer_t));
#endif
	//assign values to the tx_queue_buffer_local
	tx_queue_buffer_local->seqno = 0;
	tx_queue_buffer_local->total_len = 0;
	tx_queue_buffer_local->tx_offset = 0;
	tx_queue_buffer_local->tx_frame_info.length = tx_length;
	tx_queue_buffer_local->tx_frame_info.timestamp_create = jiffies;

	//assign values to the mac_header_cstdd
	data_header->frame_control = MAC_FRAME_CTRL1_SUBTYPE_DATA;
	data_header->dest_id = h_dest[5];
	data_header->src_id = h_source[5];
	data_header->sequence_control = veth_priv->virt_traffic_param.numb;
	if(veth_priv->virt_traffic_param.numb >= 255)
	{
		veth_priv->virt_traffic_param.numb = 0;
	}
	else
		veth_priv->virt_traffic_param.numb ++;
	data_header->length_bytes = 0;
	data_header->rpt_sync_RD = 0;

	//put mac_header_cstdd information after ethernet header
	memcpy(((void*)data_header+sizeof(mac_header_cstdd)),(void*)llc,sizeof(llc_header_t));


//	for(i = 0 ; i < sizeof(ethernet_header_t)+sizeof(mac_header_cstdd)+sizeof(llc_header_t); i ++)
//	{
//		printk("%x ",skb->data[i+sizeof(tx_queue_buffer_t)]);
//	}
//	printk("\n");
	return 0;
}

//Add control header to received packet, then queue the new packet

u8 virt_eth_util_encap(struct sk_buff *skb,struct net_device *dev/*,  u32 eth_rx_len*/){
	struct virt_eth_priv *veth_priv = netdev_priv(dev); //dev = soft_interface, eth1
	struct ethhdr            * ethhdr;
	ipv4_header_t            * ip_hdr;
	icmp_header_t            * icmp_hdr;
	arp_ipv4_packet_t        * arp;
	udp_header_t             * udp;
	dhcp_packet              * dhcp;
	llc_header_t              llc_hdr;
	bat_unicast_header_t     * bat_unicast_hdr;
	bat_bcast_header_t       * bat_bcast_hdr;
	struct ethhdr            * bat_eth_hdr;
	tcp_header_t             * tcp_hdr;

	u32                        mpdu_tx_len;
//	u8                       * udp_payload;

//	tx_queue_buffer_t        * tx_queue_buffer_local;
	u8                       flag = 1;
	u8* mpdu_start_ptr;
	u8* eth_start_ptr;
	u8  reserved = IPV4_PROT_UDP;
	u8  eth_dest[MAC_ADDR_LEN];
	u8  eth_src[MAC_ADDR_LEN];
	u8 ret = 0;
	u32 i;
	u8 node_msc;
	virt_eth_work_tx_poll* v_work_tx;
	virt_station_info* vstation_info = NULL;
	virt_eth_work_manage* v_work_manage;


	ethhdr = eth_hdr(skb);
	memcpy(eth_src, ethhdr->h_source, 6);
	memcpy(eth_dest, ethhdr->h_dest, 6);

#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
	if(ethhdr->h_dest[5] != 0xff)
		memcpy(test_mac, ethhdr->h_source, 6);
#endif

	mpdu_start_ptr = (u8*)(skb->data);
	eth_start_ptr = (u8*)(skb->data + ETH_PAYLOAD_OFFSET);
#ifdef Docker_Qualnet
	mpdu_tx_len = skb->len + sizeof(llc_header_t) + sizeof(mac_header_cstdd);
#elif defined Zynq_Platform
	mpdu_tx_len = skb->len - sizeof(ethernet_header_t) + sizeof(llc_header_t) + sizeof(mac_header_cstdd);
#endif
//  printk("virt proto %d,received length %d,mpdu_tx_len %d\n",ethhdr->h_proto,skb->len,mpdu_tx_len);
//	printk("encap,veth_priv mac address is %x , %x, %x, %x, %x, %x\n",veth_priv->addr[0],veth_priv->addr[1],veth_priv->addr[2],\
//			veth_priv->addr[3],	veth_priv->addr[4],veth_priv->addr[5]);
//			for(i = 0 ; i < sizeof(ethernet_header_t)+sizeof(mac_header_cstdd)+sizeof(llc_header_t); i ++)
//			{
//				printk("%x ",skb->data[i]);
//			}
//			printk("\n");

	//1. find the inset queue id accord to the packet type, then assign this value to "reserved"
	//2.assign llc_hdr.type accord to the packet type
	switch(ethhdr->h_proto) {
		case ETH_TYPE_CTRL:

			return 0;
		break;
		case ETH_TYPE_ARP:
			llc_hdr.type = LLC_TYPE_ARP;

			// Overwrite ARP request source MAC address field with the station's wireless MAC address.
			arp = (arp_ipv4_packet_t*)((void*)ethhdr + sizeof(ethernet_header_t));
			memcpy(arp->sender_haddr, eth_src, 6);
			if(ether_addr_equal_unaligned(eth_src,veth_priv->soft_dev->dev_addr))
			{
				return 0;
			}
			memcpy(ethhdr->h_source,veth_priv->soft_dev->dev_addr,6);
			printk("virt arp src mac %pM\n",ethhdr->h_source);
			//raise the priority of ARP
			reserved = MCAST_TCP_QID;
		break;
		case ETH_TYPE_BAT:
//			if(eth_src[5] ==1)
//			printk("ETH_TYPE_BAT\n");
			llc_hdr.type = LLC_TYPE_BAT;
			bat_unicast_hdr = (bat_unicast_header_t*)((void*)ethhdr + sizeof(struct ethhdr));
			if(bat_unicast_hdr->type == BATADV_BCAST){
					reserved = MCAST_TCP_QID;
					//bat_bcast_hdr = (bat_bcast_header_t*)((void*)ethhdr + sizeof(struct ethhdr));
					//bat_eth_hdr = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_bcast_header_t));
					//if (bat_eth_hdr->h_proto == ETH_TYPE_ARP)
					//{
					//	arp = (arp_ipv4_packet_t*)((void*)bat_eth_hdr + sizeof(ethernet_header_t));
					//	printk("Veth: Recieved Boradcast ARP packect from ethernet, operation = %d, dest node id = %d\n",ntohs(arp->oper),arp->target_paddr[3]);

					//}
				}
			else if(bat_unicast_hdr->type == BATADV_UNICAST)
			{

				bat_eth_hdr = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
//				printk("BATADV_UNICAST %x %d\n",bat_unicast_hdr->type,ntohs(bat_eth_hdr->h_proto));
				switch(bat_eth_hdr->h_proto)
				{
				case ETH_TYPE_ARP:
				{
//					if(eth_src[5] !=2)
//					printk("ETH_TYPE_ARP %x\n",bat_eth_hdr->h_proto);
					//arp = (arp_ipv4_packet_t*)((void*)bat_eth_hdr + sizeof(ethernet_header_t));
					//printk("Veth: Recieved Uicast ARP packect from ethernet, operation = %d, dest node id = %d\n",ntohs(arp->oper),arp->target_paddr[3]);


					
					reserved = MCAST_TCP_QID;



	//							tx_queue_buffer_local->tx_frame_info.reserved = MANAGEMENT;
					break;
				}
				case ETH_TYPE_IP:
				{

					ip_hdr = (ipv4_header_t*)((void*)bat_eth_hdr + sizeof(struct ethhdr));

//					printk("ETH_TYPE_IP %x %d\n",bat_eth_hdr->h_proto,ip_hdr->protocol);

					switch(ip_hdr->protocol)
					{
					case IPV4_PROT_TCP:
         
//						printk("IPV4_PROT_TCP %x, \n",ip_hdr->protocol);
						reserved = IPV4_PROT_TCP;

//						tcp_hdr =  (tcp_header_t*)((void*)ip_hdr + sizeof(ipv4_header_t));
//						if(ntohs(ip_hdr->total_length) == 64)
//						{
//							start_tcp_seq_no = ntohl(tcp_hdr->seq_no);
//							printk("tcp start seq %u",start_tcp_seq_no);
//						}
////						printk("src port %d, dest port %d, seq %u \n",
////							ntohs(tcp_hdr->src_port),ntohs(tcp_hdr->dst_port),ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1);
//
//	//								tx_queue_buffer_local->tx_frame_info.reserved = MANAGEMENT;
//
//						printk("rec from BATMAN: tcp seq num = %u (%u),  ip_len = %d, dest ip addr %d.%d.%d.%d\n",
//							ntohl(tcp_hdr->seq_no),ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1,ntohs(ip_hdr->total_length),NIPQUAD(ip_hdr->dest_ip_addr));
					
//						if((ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1) ==TARTET_TCP_ACK_NO)
//						{
//							printk("rec: tcp seq num = %d, skb_len = %d, mpdu_tx_len =%d ip_len = %d, dest ip addr %d.%d.%d.%d, reserved = %d\n",
//								ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1,skb->len,mpdu_tx_len,ntohs(ip_hdr->total_length),NIPQUAD(ip_hdr->dest_ip_addr),reserved);
//						}

						break;
					case IPV4_PROT_UDP:
//						printk("IPV4_PROT_UDP %x\n",ip_hdr->protocol);
						reserved = IPV4_PROT_UDP;
						udp = (udp_header_t*)((void*)ip_hdr + 4*((u8)(ip_hdr->version_ihl) & 0xF));
						if((ntohs(udp->dest_port)) == UDP_PORT_54321 || (ntohs(udp->dest_port)) == UDP_PORT_54322){
							//veth_priv->v_jgk_info.tx_in_lose ++;
						}
						break;
					case IPV4_PROT_ICMP:


						reserved = MANAGEMENT_QID;

						//printk("IPV4_PROT_ICMP %x %d\n",ip_hdr->protocol,reserved);
						//icmp_hdr = (icmp_header_t *)((void*)ip_hdr + sizeof(ipv4_header_t));
						//printk("Veth: Recieved ICMP packect from ethernet, type = %d, seq_number= %d\n",icmp_hdr->icmp_type,ntohs(icmp_hdr->icmp_sequence));

						////bat_eth_hdr = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
						//printk("src mac address: %d-%d-%d-%d\n",bat_eth_hdr->h_source[2],bat_eth_hdr->h_source[3],
						//	bat_eth_hdr->h_source[4],bat_eth_hdr->h_source[5]);
						//printk("dest mac address:%d-%d-%d-%d\n",bat_eth_hdr->h_dest[2],bat_eth_hdr->h_dest[3],
						//	bat_eth_hdr->h_dest[4],bat_eth_hdr->h_dest[5]);
//						if(mpdu_tx_len > virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,MULTICAST_MCS_DEFAULT))
//						{
//							if(virt_eth_mgmt_get_mcs(veth_priv,eth_dest[5]) == 0x0f){
//								ret = 1;
//								goto err;
//							}

							if(veth_priv->mcs_mode == FIX_MCS_MODE){
								veth_priv->virt_traffic_param.pkt_enq_bytes[veth_priv->ucast_mcs] += \
										(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd));
							}else{
								node_msc = virt_eth_mgmt_get_mcs(veth_priv,eth_dest[5]);
								if(node_msc<8)
								veth_priv->virt_traffic_param.pkt_enq_bytes[node_msc] += \
										(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd)); //update the flow statistics
							}
							veth_priv->v_jgk_info.ping_in ++;
							veth_priv->v_jgk_info.ping_in_len += mpdu_tx_len;
//						}
//						else
//						{
//							veth_priv->virt_traffic_param.pkt_enq_bytes[virt_eth_mgmt_get_mcs(veth_priv,eth_dest[5])] += \
//									(virt_eth_mgmt_get_mcs_len_by_id(veth_priv,eth_dest[5])+WLAN_PHY_FCS_NBYTES); //update the flow statistics
//						}
						flag = 0;
						break;
					default:
						break;
					}
					break;
				}
				default:
	//							tx_queue_buffer_local->tx_frame_info.reserved = MANAGEMENT;
					reserved = IPV4_PROT_UDP;
					break;
				}
			}
			else if(bat_unicast_hdr->type == BATADV_IV_OGM)
			{
             	


				
				reserved = MCAST_QID;

//				printk(" REC BATADV_IV_OGM, reserved = %d %x\n",bat_unicast_hdr->type,reserved);

				flag = 0;
				veth_priv->v_jgk_info.ogm_in ++;
				veth_priv->v_jgk_info.ogm_in_len += mpdu_tx_len;
				////////////////////////////////////////////////////////////////
//				jgk_data1.r_pc_ogm ++;
//				jgk_data1.r_pc_ogm_flow += mpdu_tx_len - WLAN_PHY_FCS_NBYTES;
			}



		break;

		case ETH_TYPE_IP:
	//                	llc_hdr->type = LLC_TYPE_BAT;

			llc_hdr.type = LLC_TYPE_IP;

			// Check if IPv4 packet is a DHCP Discover in a UDP frame
			ip_hdr = (ipv4_header_t*)((void*)ethhdr + sizeof(ethernet_header_t));
			printk("ETH_TYPE_IP %x\n",ip_hdr->protocol);

			//---------------------
			if(ip_hdr->protocol == IPV4_PROT_ICMP)
			{
				reserved = MANAGEMENT_QID;
//				printk("IPV4_PROT_ICMP pk len=%d,mpdu_tx_len = %d\n",skb->len,mpdu_tx_len);;
			}
			else if (ip_hdr->protocol == IPV4_PROT_TCP)
			{
				//tx_queue_buffer_local->metadata.reserved[0] = IPV4_PROT_TCP;
				reserved = IPV4_PROT_TCP;

				
				
				
			}
			else if (ip_hdr->protocol == IPV4_PROT_UDP) {
			reserved = IPV4_PROT_UDP;

			udp = (udp_header_t*)((void*)ip_hdr + 4*((u8)(ip_hdr->version_ihl) & 0xF));
			if( (ntohs(udp->dest_port)) == UDP_DST_PORT_MANAGEMENT)
			{
				v_work_manage = kmalloc(sizeof(*v_work_manage), GFP_KERNEL);
				v_work_manage->dev = dev;
				memcpy(v_work_manage->manage_array,(u8*)udp+8,10);

				INIT_WORK(&(v_work_manage->worker), virt_eth_mgmt_manage);
				schedule_work(&(v_work_manage->worker));
//					queue_work(virt_data_event_workqueue,&(v_work_manage->worker));
				ret = 1;
				goto err;
			}

				if((ntohs(udp->dest_port)) == UDP_PORT_54321 || (ntohs(udp->dest_port)) == UDP_PORT_54322){
//                        	eth_cnt_port_54321 ++;

//					rpmsg_data_send(eth_hdr,eth_rx_len);
				}


				// Check if this is a DHCP Discover packet, which contains the source hardware
				// address deep inside the packet (in addition to its usual location in the Eth
				// header). For STA encapsulation, we need to overwrite this address with the
				// MAC addr of the wireless station.
				if ((ntohs(udp->src_port) == UDP_SRC_PORT_BOOTPC) ||
					(ntohs(udp->src_port) == UDP_SRC_PORT_BOOTPS)) {

					// Disable the checksum since this will change the bytes in the packet
					udp->checksum = 0;

					dhcp = (dhcp_packet*)((void*)udp + sizeof(udp_header_t));

					if (ntohl(dhcp->magic_cookie) == DHCP_MAGIC_COOKIE) {
						// Assert the DHCP Discover's BROADCAST flag; this signals to any DHCP
						// severs that their responses should be sent to the broadcast address.
						// This is necessary for the DHCP response to propagate back through the
						// wired-wireless portal at the AP, through the STA Rx MAC filters, back
						// out the wireless-wired portal at the STA, and finally into the DHCP
						// listener at the wired device
						dhcp->flags = htonl(DHCP_BOOTP_FLAGS_BROADCAST);

					} // END is DHCP valid
				} // END is DHCP
			} // END is UDP
		break;


		case htons(ETH_P_CTL):
//		printk("ETH_P_CTL\n");
				ret = virt_eth_do_comm_board(dev,skb,skb->len);
				return ret;

		default:
			// Unknown/unsupported EtherType; don't process the Eth frame
			return 0;
		break;
	} //END switch(pkt type)
	if(wlan_addr_eq(eth_dest,def_multicast_mac1) || is_multicast_ether_addr(eth_dest))
	{
//		veth_priv->virt_traffic_param.pkt_enq_bytes[2] += (mpdu_tx_len+WLAN_PHY_FCS_NBYTES); //update the flow statistics
		eth_dest[5] = MULTICAST_ADDR;
		if(reserved == MCAST_QID){
			if(veth_priv->mcs_mode == FIX_MCS_MODE){
				veth_priv->virt_traffic_param.pkt_enq_bytes[veth_priv->bcast_mcs] += \
						(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd));
			}else{
				veth_priv->virt_traffic_param.pkt_enq_bytes[MULTICAST_MCS_DEFAULT_2] += \
						(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd));
			}
		}else{
			if(veth_priv->mcs_mode == FIX_MCS_MODE){
				veth_priv->virt_traffic_param.pkt_enq_bytes[veth_priv->bcast_mcs] += \
						(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd));
			}else{
				veth_priv->virt_traffic_param.pkt_enq_bytes[MULTICAST_MCS_DEFAULT] += \
						(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd));
			}
		}
		flag = 0;

		if(reserved == IPV4_PROT_UDP)
			reserved = MCAST_TCP_QID;


		if(reserved != MCAST_QID){
			veth_priv->v_jgk_info.bcast_in ++;
			veth_priv->v_jgk_info.bcast_in_len += mpdu_tx_len;
		}
	}
	if(flag)
	{
		if(eth_dest[5] >= MAX_NODE){
			ret = 1;
			goto err;
		}

		if(veth_priv->mcs_mode == FIX_MCS_MODE){
			veth_priv->virt_traffic_param.pkt_enq_bytes[veth_priv->ucast_mcs] += \
					(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd));
		}else{
			node_msc = virt_eth_mgmt_get_mcs(veth_priv,eth_dest[5]);
			if(node_msc<8)
			veth_priv->virt_traffic_param.pkt_enq_bytes[node_msc] += \
					(mpdu_tx_len-sizeof(llc_header_t) - sizeof(mac_header_cstdd)); //update the flow statistics
		}

		veth_priv->v_jgk_info.ucast_in ++;
		veth_priv->v_jgk_info.ucast_in_len += mpdu_tx_len;
	}


	//	if(veth_priv->addr[5]==1)
	//		printk("node %d llc sequence number is %d",veth_priv->addr[5],veth_priv->virt_traffic_param.eth_data_seqno);
		llc_hdr.seqno = veth_priv->virt_traffic_param.eth_data_seqno;
		llc_hdr.length = mpdu_tx_len ;
		getNewNo(veth_priv);
		llc_hdr.fragment_offset = 0;
		llc_hdr.src_nodeId = veth_priv->addr[5];
		llc_hdr.dest_nodeId = eth_dest[5];
#ifdef Radio_220
		llc_hdr.corss_q_ind = veth_priv->channel_num;
#else
		llc_hdr.corss_q_ind = CONDITION_FALSE;
#endif
	
		//printk("eth_dest[5] = %d\n",eth_dest[5]);
		//check whether the destination node is new_'
		if(eth_dest[5] != MULTICAST_ADDR){
			vstation_info = virt_eth_station_find_id(eth_dest[5],veth_priv);
			if(vstation_info == NULL){
				//add station information for the new node
				ret = virt_eth_station_add(eth_dest[5],eth_dest,virt_eth_mgmt_get_mcs(veth_priv,eth_dest[5]),veth_priv);
				if(ret != 0)
				{
	//				printk(" ret = %d,virt_eth_station_add go to error\n",ret);
					goto err;
				}
			}
	
		}
	
		//******************************************************************************
		// test
	
	//	ret = virt_eth_queue_tail(veth_priv,skb,MANAGEMENT_QID,eth_dest[5]);
	//	v_work_tx = kmalloc(sizeof(*v_work_tx), GFP_KERNEL);
	//	v_work_tx->dev = dev;
	//	//poll tx queue, send the packet
	//	INIT_WORK(&(v_work_tx->worker), virt_eth_poll_tx_queue);
	//	queue_work(veth_priv->virt_mgmt_event_workqueue,&(v_work_tx->worker));
	//	skb = virt_eth_queue_tx_dequeue(veth_priv,MULTICAST_MCS_DEFAULT + 1);
	//	ret = dev_queue_xmit(skb);
	//	return 0;
	
		//******************************************************************************
	
		//create frame, the struct of the new frame is tx_queue_buffer_t (will not send by phy) + ethernet header
		//+ mac_header_ctsdd + llc_header + payoff,orderly
	
	
		ret = virt_eth_util_create_data_frame(skb,veth_priv,&llc_hdr,eth_src,eth_dest,mpdu_tx_len);
		if(ret != 0)
		{
			goto err;
		}
	
	
		//enqueue
		//test
	//	printk("reserved %d,eth_dest[5] %d\n",reserved,eth_dest[5]);
	//	printk("reserved %d\n",reserved);
		//insert the packet in the queue tail
	
	//	skb_pull(skb,sizeof(tx_queue_buffer_t));
	//	ret = dev_queue_xmit(skb);
	//
	//		for(i = 0 ; i < sizeof(ethernet_header_t)+sizeof(mac_header_cstdd)+sizeof(llc_header_t); i ++)
	//		{
	//			printk("%x ",skb->data[i]);
	//		}
	//		printk("\n");
	
//		if (reserved == MANAGEMENT_QID)
//		{
//			printk("virt_eth_queue_tail,reserved = %d,eth_dest[5] %d\n",reserved,eth_dest[5]);
//		}
	
		ret = virt_eth_queue_tail(veth_priv,skb,reserved,eth_dest[5]);
		//printk("node id = %d, virt_tx_queue = 0x%p,virt_rx_queue = 0x%p\n",veth_priv->addr[5],\
		//	(void *)veth_priv->virt_tx_queue,(void *)veth_priv->virt_rx_queue);
		
//		if (bat_unicast_hdr->type == BATADV_UNICAST){
//	       bat_eth_hdr = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
//	       if(bat_eth_hdr->h_proto == ETH_TYPE_IP)
//	   	   {
//	         
//	       }
//
//	   }
		if(ret == 1)
		{
			goto err;
		}
		/////
	//	if(veth_priv->addr[5]== 1)
	//		printk("node id = %d,queue id = %d queue_len %d\n",veth_priv->addr[5],reserved,virt_eth_queue_get_length(veth_priv,reserved));
		//		printk(" queue_len %d\n",virt_eth_queue_get_tx_length(eth_dest[5]));
	
		//******************************************************************************
		// test
	
		//******************************************************************************
		
#ifdef Docker_Qualnet
		//poll tx queue, send the packet
		i=0;
		while(1)
		{
			i++;
			virt_eth_poll_tx_queue(dev);
			if(i==7)
				break;
		}
	
#elif defined Zynq_Platform
		v_work_tx = kmalloc(sizeof(*v_work_tx), GFP_ATOMIC);
		if (v_work_tx == NULL)
		{
	//		printk("v_work_tx == NULL");
			return 1;
		}
		v_work_tx->dev = dev;
		//poll tx queue, send the packet
		INIT_WORK(&(v_work_tx->worker), virt_eth_poll_tx_queue);
	//	printk("INIT_WORK done\n");
	//	printk("virt_mgmt_event_workqueue=0x%p,v_work_tx->worker=0x%p",veth_priv->virt_mgmt_event_workqueue,v_work_tx->worker);
		queue_work(veth_priv->virt_data_event_workqueue,&(v_work_tx->worker));
	//	printk("queue_work done\n");
#endif //end for #ifdef Docker_Qualnet


	return 0;

err:
//	dev_kfree_skb (skb);
//
//    v_jgk_info.tx_in_lose ++;

	return ret;
}

u8 virt_eth_util_send_data(void* mpdu, u16 length, u8 pre_llc_offset,u8* src_mac,u8* dst_mac,struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
    int                      status = 1;
    u8                     * eth_mid_ptr;
#ifdef Zynq_Platform
    rx_frame_info_t        * rx_frame_info;
#endif

//    mac_header_80211         mac80211_hdr;
//    mac_header_cstdd       * rxcstdd_hdr;
    mac_header_cstdd       * rx80211_hdr;
    llc_header_t           * llc_hdr;

    ethernet_header_t        * eth_hdr;
    ipv4_header_t            * ip_hdr;
    udp_header_t             * udp;

    arp_ipv4_packet_t        * arp;
    dhcp_packet            * dhcp;
	
	struct ethhdr            * bat_eth_hdr;
	tcp_header_t             * tcp_hdr;

    u8                       continue_loop;
    u8                       is_dhcp_req         = 0;

    struct sk_buff*			group_data = NULL;
    struct sk_buff*			send_data = NULL;

//    struct sk_buff*			skb_test = NULL;

    struct sk_buff*			entry_data = NULL;

    tx_queue_buffer_t *tx_group = NULL;

//    tx_queue_buffer_t *mpdu_data = NULL;
    u8                     group_flag = 0;

    u8                       addr_cache[6];
    u32                      len_to_send;

    u32                      packet_header_length = sizeof(mac_header_cstdd) + sizeof(llc_header_t);

//    u8                       is_not_in_group = 0;

    u16                      offset = 0;
    u16                      end = 0;
//    u32 tmp;
//    int i;
//    u32 flag;
	bat_unicast_header_t     * bat_unicast_hdr;


    if(length < (packet_header_length + pre_llc_offset)){
//        printk("Error in wlan_mpdu_eth_send: length of %d is too small... must be at least %d\n", length, min_pkt_len);
        return -1;
    }

//    printk("virt_eth_util_send_data len %d\n",length);

//    rx80211_hdr = (mac_header_cstdd*)((void *)h80211);
    llc_hdr     = (llc_header_t*)((void *)mpdu + sizeof(mac_header_cstdd) + pre_llc_offset);
    eth_hdr     = (ethernet_header_t*)((void *)mpdu + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset - sizeof(ethernet_header_t));
    len_to_send = length - packet_header_length - pre_llc_offset + ETH_HLEN;

            switch(llc_hdr->type){
                case LLC_TYPE_ARP:
			send_data = virt_eth_queue_create_skb(len_to_send);
            		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
            		eth_hdr = (ethernet_header_t*)send_data->data;

                    memcpy(addr_cache, src_mac, 6);

                    // Insert the Eth source, from the 802.11 header address 2 field
                    memcpy(eth_hdr->src_mac_addr, addr_cache, 6);
                    memcpy(eth_hdr->dest_mac_addr, dst_mac, 6);
                    eth_hdr->ethertype = ETH_TYPE_ARP;

                    // If the ARP packet is addressed to this STA wireless address, replace the ARP dest address
                    // with the connected wired device's MAC address
                    arp = (arp_ipv4_packet_t*)((void*)eth_hdr + sizeof(ethernet_header_t));
//                    if (wlan_addr_eq(arp->target_haddr, get_mac_hw_addr_wlan())) {
                        //memcpy(arp->target_haddr, eth_sta_mac_addr, 6);
//                    }
                break;

                case LLC_TYPE_IP:
                	printk("rx llc_hdr->type = %d\n",LLC_TYPE_IP);

                 	if(llc_hdr->fragment_offset & ntohs(IP_DF))//分片
    					{

                 		    //check if the number of packet in received queue is reach max value
                 		   //if so, remove the first received packet in the queue
    						if(virt_eth_queue_get_rx_length(veth_priv,llc_hdr->src_nodeId) >= MAX_GROUP_NUM)
    						{

    							virt_eth_queue_remove_rx_first(veth_priv,llc_hdr->src_nodeId);
    							veth_priv->v_jgk_info.rx_out_lose ++;
    						}
    						//find packet from received queue according node id and sequence number
    						if(llc_hdr == NULL || veth_priv == NULL)
    							printk("error\n");
    						group_data = virt_eth_queue_find_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    						if(group_data){
    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							if(time_is_before_jiffies(tx_group->tx_frame_info.timestamp_create + msecs_to_jiffies(5000)))
    							{
    								virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
//    								dev_kfree_skb (group_data);
    								group_data = NULL;
    							}
    						}
    						//if cannot find packet, then creat a new skb for the sequence number
    						if(group_data == NULL)
    						{
    							//create a new skb struct to store fragment for the certain sequence number
    							printk("find id %d\n",llc_hdr->seqno);
    							group_data = virt_eth_queue_create_skb(MAC_PKT_BUF);
    							if(group_data!=NULL)
    							{
    								//initial the skb struct
    								virt_eth_add_rx_queue(veth_priv,llc_hdr->src_nodeId,group_data);
    								skb_put(group_data,MAC_PKT_BUF);

    								//initial the tx_queue_buffer_t for record the fragment information
    								tx_group = (tx_queue_buffer_t*)(group_data->data);
    								tx_group->total_len = 0;
    								tx_group->tx_offset = 0;
    								tx_group->seqno = 0;
    								tx_group->flags = 0;

    							}
    						}
    						//if find the packet from the receive queue or create a new skb struct
    						if(group_data != NULL)
    						{
    							//offset is used to record the tail position of payoff for the packet
    							offset = ntohs(llc_hdr->fragment_offset);
    							//get the last 13 bits of llc_hdr->fragment_offset for the offset
    							offset &= IP_OFFSET;
#ifdef Docker_Qualnet
    							if( offset > sizeof(ethernet_header_t) )
    							{
    								// adjust the start position for sender and receiver
    								//the start position of the sender is at the ethernet header
    								//the start position of the receiver is at the mac_header_cstdd
    								offset -= sizeof(ethernet_header_t);
    							}
#endif

    							printk("offset %d %d\n",offset,llc_hdr->fragment_offset);

    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							tx_group->seqno = llc_hdr->seqno;
    							tx_group->tx_frame_info.timestamp_create = jiffies;

//    							skb_test = virt_eth_queue_find_by_id(llc_hdr->nodeId,llc_hdr->seqno);
//    							if(skb_test == NULL)
//    								printk("--------------------\n");

    							//check if this received fragment is last one
    							if(llc_hdr->fragment_offset & ntohs(IP_LF))
    							{
    								//set the last fragment flag is true (the penultimate bit is set to 1)
    								tx_group->flags |= INET_LAST_IN;
    								//the end is used to record tail position of packet for current reached fragments
    								//add the payoff length for the current received fragment
    								end = offset + llc_hdr->length - packet_header_length;
    								//tx_group->tx_offset record the total length of received fragment
    								tx_group->tx_offset += (length - packet_header_length);
    								printk("ip 1111 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    								//end should be always larger than tx_group->total_len
    								if(end < tx_group->total_len)
    								{
    									goto err;
    								}
    								tx_group->total_len = end;
    							}
    							//this fragment is not the last one
    							else
    							{
    								//check if the fragment is first one
    								if(offset == 0)
    								{
    									//the recorded length should contain the length of header
    									end = offset + llc_hdr->length;
    									tx_group->tx_offset += llc_hdr->length;
    								}
    								//the fragment is not the first one
    								else
    								{
    									//add the payoff length for the current received fragment(should subtract the header length )
    									end = offset + llc_hdr->length - packet_header_length;

    									tx_group->tx_offset += (llc_hdr->length - packet_header_length);
    									printk("tx_group->tx_offset %d\n",tx_group->tx_offset);
    								}
    								//check if the recorded tail position (end) of packet is updated
    								if(end > tx_group->total_len)
    									tx_group->total_len = end;

    								printk("ip 2222 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    							}
    							//copy the received fragment to the skb struct in the received queue
    							//check if the fragment is first one
    							if(offset == 0)
    							{
    								//set the first fragment flag is true (the last bit is set to 1)
    								tx_group->flags |= INET_FIRST_IN;
    								//copy the whole fragment to the skb struct in the received queue
    								memcpy((void*)tx_group->frame,(void*)mpdu, length);
    							}
    							//the fragment is not the first one
    							else
    							{
    								//copy the payoff to the skb struct in the received queue
    								memcpy((void*)tx_group->frame+offset, \
    									(void *)mpdu + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset, \
    									length - packet_header_length - pre_llc_offset);
    							}
    							//check if all fragments have reached, the condition is that
    							//the first and last fragment have been receiveth_hdred,
    							//meanwhile the tail of packet(tx_group->total_len) is equal to the
    							// total length of the received fragments
    							if (tx_group->flags == (INET_FIRST_IN | INET_LAST_IN) &&
    								tx_group->total_len == tx_group->tx_offset)
    							{
    								group_flag = 1;
    								//create ethernet header
    								eth_hdr     = (ethernet_header_t*)((void *)tx_group->frame + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset - sizeof(ethernet_header_t));
    								//the ethernet header length
    								len_to_send = tx_group->tx_offset - sizeof(mac_header_cstdd) - sizeof(llc_header_t) - pre_llc_offset + sizeof(ethernet_header_t);
    								printk("send_data %d\n",len_to_send);
    								//create new send skb struct
//    								if(len_to_send > 1600)
//    									printk("send_data %d\n",len_to_send);
    								send_data = virt_eth_queue_create_skb(len_to_send);
    								//copy the received packed packet the send skb
    		                		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
    		                		eth_hdr = (ethernet_header_t*)send_data->data;
    		                		//delete the packet from the node's received queue
    		                		virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    		                		group_data = NULL;
//    		                		dev_kfree_skb (group_data);
    							}
    							//the all fragments have not reached, then return, waiting for the others fragment reaching
    							else
    							{
    								return 0;
    							}
    						}
    					}
                 	//the received packet is a whole one, donot fragment
                 	//then copy the packet to the new created skb, send it to batman-adv
                    else{

                    	    printk("not fragment pk,len_to_send = %d\n",len_to_send);
                    		send_data = virt_eth_queue_create_skb(len_to_send);
                    		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
                    		eth_hdr = (ethernet_header_t*)send_data->data;

                    	}


                 	//vaule the ethernet header
                    memcpy(addr_cache, src_mac, 6);
    				memcpy(eth_hdr->dest_mac_addr, dst_mac, 6);
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
    				memcpy(eth_hdr->src_mac_addr, test_mac, 6);
#else
                    memcpy(eth_hdr->src_mac_addr, addr_cache, 6);
#endif
					eth_hdr->ethertype = ETH_TYPE_IP;
                break;

                case LLC_TYPE_BAT:
                	//check if the received packet is fragment
                  	if(llc_hdr->fragment_offset & ntohs(IP_DF))//分片
    					{
                  		  //check if the number of packet in received queue is reach max value
                  		   //if so, remove the first received packet in the queue
    						if(virt_eth_queue_get_rx_length(veth_priv,llc_hdr->src_nodeId) >= MAX_GROUP_NUM)
    						{
    							virt_eth_queue_remove_rx_first(veth_priv,llc_hdr->src_nodeId);
    							veth_priv->v_jgk_info.rx_out_lose ++;
//    							printk("MAX_GROUP_NUM\n");
    						}
    						//find packet from received queue according node id and sequence number
    						group_data = virt_eth_queue_find_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    						//if cannot find packet, then creat a new skb for the sequence number
    						if(group_data){
    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							if(time_is_before_jiffies(tx_group->tx_frame_info.timestamp_create + msecs_to_jiffies(5000)))
    							{
    								virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
//    								dev_kfree_skb (group_data);
    								group_data = NULL;
    							}
    						}
    						if(group_data == NULL)
    						{
//    							printk("find id %d\n",llc_hdr->seqno);
    							//create a new skb struct to store fragment for the certain sequence number
    							group_data = virt_eth_queue_create_skb(MAC_PKT_BUF);
    							if(group_data!=NULL)
    							{
    								//initial the skb struct
    								virt_eth_add_rx_queue(veth_priv,llc_hdr->src_nodeId,group_data);
    								skb_put(group_data,MAC_PKT_BUF);
    								//initial the tx_queue_buffer_t for record the fragment information
    								tx_group = (tx_queue_buffer_t*)(group_data->data);
    								tx_group->total_len = 0;
    								tx_group->tx_offset = 0;
    								tx_group->seqno = 0;
    								tx_group->flags = 0;
    							}
    						}
    						//if find the packet from the receive queue or create a new skb struct
    						if(group_data != NULL)
    						{
    							//offset is used to record the tail position of payoff for the packet
    							//get the last 13 bits of llc_hdr->fragment_offset for the offset
    							offset = ntohs(llc_hdr->fragment_offset);
    							offset &= IP_OFFSET;
#ifdef Docker_Qualnet
    							if( offset > sizeof(ethernet_header_t) )
    							{
    								// adjust the start position for sender and receiver
    								//the start position of the sender is at the ethernet header
    								//the start position of the received is at the mac_header_cstdd
    								offset -= sizeof(ethernet_header_t);
    							}
#endif
//    							if( length>100)
//    							printk("node %d,offset %d,llc_hdr->fragment_offset %d,llc_hdr->length %d,llc_hdr->seqno %d\n",\
//    									veth_priv->addr[5],offset,ntohs(llc_hdr->fragment_offset),llc_hdr->length,llc_hdr->seqno);

    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							tx_group->seqno = llc_hdr->seqno;
    							tx_group->tx_frame_info.timestamp_create = jiffies;

//    							skb_test = virt_eth_queue_find_by_id(llc_hdr->nodeId,llc_hdr->seqno);
//    							if(skb_test == NULL)
//    								printk("--------------------\n");
    							//check if this received fragment is last one
    							if(llc_hdr->fragment_offset & ntohs(IP_LF))
    							{
    								//set the last fragment flag is true (the penultimate bit is set to 1)
    								tx_group->flags |= INET_LAST_IN;
    								//the "end" is used to record tail position of packet for current reached fragments
    								//add the payoff length for the current received fragment
    								end = offset + llc_hdr->length - packet_header_length;
    								//tx_group->tx_offset record the total length of received fragment
    								tx_group->tx_offset += (length - packet_header_length);
//    								printk("ip 1111 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    								//tx_group->total_len should be equal to the value of "end"
    								//tx_group->total_len should be update when the "end" is changed
    								if(end < tx_group->total_len){
    									veth_priv->v_jgk_info.rx_out_lose ++;
    									goto err;
    								}
    								tx_group->total_len = end;
    							}
    							//this fragment is not the last one
    							else
    							{
    								//check if the fragment is first one
    								if(offset == 0)
    								{
    									//the recorded length should contain the length of header
    									end = offset + llc_hdr->length;
    									tx_group->tx_offset += llc_hdr->length;
    								}
    								//the fragment is not the first one
    								else
    								{
    									//add the payoff length for the current received fragment(should subtract the header length )
    									end = offset + llc_hdr->length - packet_header_length;
    									tx_group->tx_offset += llc_hdr->length - packet_header_length;
//    									printk("tx_group->tx_offset %d\n",tx_group->tx_offset);
    								}
    								//check if the recorded tail position (end) of packet is updated
    								if(end > tx_group->total_len)
    									tx_group->total_len = end;

//    								printk("ip 2222 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    							}
    							//copy the received fragment to the skb struct in the received queue
    							//check if the fragment is first one
    							if(offset == 0)
    							{
    								//set the first fragment flag is true (the last bit is set to 1)
    								tx_group->flags |= INET_FIRST_IN;
    								//copy the whole fragment to the skb struct in the received queue
    								memcpy((void*)tx_group->frame,(void*)mpdu, length);
    							}
    							//the fragment is not the first one
    							else
    							{
    								//copy the payoff to the skb struct in the received queue
    								memcpy((void*)tx_group->frame+offset, \
    									(void *)mpdu + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset, \
    									length - packet_header_length - pre_llc_offset);
    							}
    							//check if all fragments have reached, the condition is that
    							//the first and last fragment have been received,
    							//meanwhile the tail of packet(tx_group->total_len) is equal to the
    							// total length of the received fragments (tx_group->tx_offset)
    							if (tx_group->flags == (INET_FIRST_IN | INET_LAST_IN) &&
    								tx_group->total_len == tx_group->tx_offset)
    							{

    								group_flag = 1;
    								//create ethernet header
    								eth_hdr     = (ethernet_header_t*)((void *)tx_group->frame + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + \
    										pre_llc_offset - sizeof(ethernet_header_t));
    								//the ethernet header length
    								len_to_send = tx_group->tx_offset - sizeof(mac_header_cstdd) - sizeof(llc_header_t) - pre_llc_offset \
    										+ sizeof(ethernet_header_t);
//    								printk("send_data %d\n",len_to_send);
    								//create new send skb struct

    								send_data = virt_eth_queue_create_skb(len_to_send);
    								//copy the received packed packet the send skb
    		                		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
    		                		eth_hdr = (ethernet_header_t*)send_data->data;
//    								if(length>100)
//    									printk("llc_hdr->seqno %d all fragments have reached,len_to_send %d",llc_hdr->seqno,len_to_send);


    		                		//delete the packet from the node's received queue
    		                		virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    		                		group_data = NULL;
//    		                		dev_kfree_skb (group_data);
    							}
    							//the all fragments have not reached, then return, waiting for the others fragment reaching
    							else
    							{
    								return 0;
    							}
    						}
    						//the created new skb struct failed
    						else
    						{
//    							printk("group err\n");
    							veth_priv->v_jgk_info.rx_out_lose ++;
    						}
    				}
                  	//the received packet is a whole one, donot fragment
                  	//then copy the packet to the new created skb, send it to batman-adv
                  	else
                    	{
                    		send_data = virt_eth_queue_create_skb(len_to_send);
                    		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
                    		eth_hdr = (ethernet_header_t*)send_data->data;
                    	}
                  	//vaule the ethernet header
                  	memcpy(addr_cache, src_mac, 6);
                  	memcpy(eth_hdr->dest_mac_addr, dst_mac, 6);
                  	memcpy(eth_hdr->src_mac_addr, addr_cache, 6);

					eth_hdr->ethertype = ETH_TYPE_BAT;
					bat_unicast_hdr = (bat_unicast_header_t*)((void*)eth_hdr + sizeof(ethernet_header_t));
#if TCP_LOOPBACK_TEST					
					bat_eth_hdr = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
                    if(bat_eth_hdr->h_proto == ETH_TYPE_IP)
                    {
                    	ip_hdr = (ipv4_header_t*)((void*)bat_eth_hdr + sizeof(struct ethhdr));
						if(ip_hdr->protocol == IPV4_PROT_TCP)
						{
							tcp_hdr =  (tcp_header_t*)((void*)ip_hdr + sizeof(ipv4_header_t));
							if(ntohs(ip_hdr->total_length) == 64)
							{
								start_tcp_seq_no = ntohl(tcp_hdr->seq_no);
							}
							printk(" rec from MAC: tcp seq num = %u (%u), skb_len = %d ip_len = %d, dest ip addr %d.%d.%d.%d\n",
								ntohl(tcp_hdr->seq_no),ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1,send_data->len,ntohs(ip_hdr->total_length),NIPQUAD(ip_hdr->dest_ip_addr)); //ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1
						}
                    }

#endif


//					if(bat_unicast_hdr->type == BATADV_IV_OGM){
//					printk("Virt-eth0: Rx ogm done, send to batman-adv, src = %d, dest = %d\n",src_mac[5],dst_mac[5]);
//
//					}
					

#ifdef Radio_220
					bat_unicast_hdr = (bat_unicast_header_t*)((void*)eth_hdr + sizeof(ethernet_header_t));
                    if(bat_unicast_hdr->type == BATADV_UNICAST||
						bat_unicast_hdr->type ==BATADV_BCAST){
						if(llc_hdr->corss_q_ind != veth_priv->channel_num){
							printk("voice channel is not match,dest channel =%d, owner channel =%d\n",llc_hdr->corss_q_ind,veth_priv->channel_num);
							dev_kfree_skb (send_data);
							return -1;
						}


						}

#endif
                break;

                default:
//                	queue_checkin(entry_data);
                    // Invalid or unsupported Eth type; give up and return
                	veth_priv->v_jgk_info.rx_out_lose ++;
                    return -1;
                break;
            }

            if(len_to_send > 1400){
            	udp = (udp_header_t*)((void*)eth_hdr+58);
            	if((ntohs(udp->dest_port)) == UDP_PORT_54321 || (ntohs(udp->dest_port)) == UDP_PORT_54322){
            		veth_priv->v_jgk_info.rx_in_lose ++;
				}
            }

#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST 
            printk("do virt_eth_test_rx_packet\n");
            virt_eth_test_rx_packet(send_data,dev);
//            dev_kfree_skb (send_data);

#else
            //value the skb sturct of send_date

                send_data->dev = dev;
                send_data->protocol = eth_type_trans(send_data, dev);

            send_data->ip_summed = CHECKSUM_UNNECESSARY;
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += len_to_send;
			//send the send_date to batman-adv

            netif_rx_ni(send_data);
#endif

            veth_priv->v_jgk_info.rx_outall ++;

//            if(len_to_send>100)
//            printk("send_data  ok, len_to_send =%d\n",len_to_send);
err:
	return 0;
}


u8 virt_eth_util_send_data_test(void* mpdu, u16 length, u8 pre_llc_offset,u8* src_mac,u8* dst_mac,struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
    int                      status = 1;
    u8                     * eth_mid_ptr;
#ifdef Zynq_Platform
    rx_frame_info_t        * rx_frame_info;
#endif

//    mac_header_80211         mac80211_hdr;
//    mac_header_cstdd       * rxcstdd_hdr;
    mac_header_cstdd       * rx80211_hdr;
    llc_header_t           * llc_hdr;

    ethernet_header_t        * eth_hdr;
    ipv4_header_t            * ip_hdr;
    udp_header_t             * udp;

    arp_ipv4_packet_t        * arp;
    dhcp_packet            * dhcp;
	
	struct ethhdr            * bat_eth_hdr;
	tcp_header_t             * tcp_hdr;

    u8                       continue_loop;
    u8                       is_dhcp_req         = 0;

    struct sk_buff*			group_data = NULL;
    struct sk_buff*			send_data = NULL;

//    struct sk_buff*			skb_test = NULL;

    struct sk_buff*			entry_data = NULL;

    tx_queue_buffer_t *tx_group = NULL;

//    tx_queue_buffer_t *mpdu_data = NULL;
    u8                     group_flag = 0;

    u8                       addr_cache[6];
    u32                      len_to_send;

    u32                      packet_header_length = sizeof(mac_header_cstdd) + sizeof(llc_header_t);

//    u8                       is_not_in_group = 0;

    u16                      offset = 0;
    u16                      end = 0;
//    u32 tmp;
//    int i;
//    u32 flag;
	bat_unicast_header_t     * bat_unicast_hdr;


    if(length < (packet_header_length + pre_llc_offset)){
//        printk("Error in wlan_mpdu_eth_send: length of %d is too small... must be at least %d\n", length, min_pkt_len);
        return -1;
    }

//    printk("virt_eth_util_send_data len %d\n",length);

//    rx80211_hdr = (mac_header_cstdd*)((void *)h80211);
    llc_hdr     = (llc_header_t*)((void *)mpdu + sizeof(mac_header_cstdd) + pre_llc_offset);
    eth_hdr     = (ethernet_header_t*)((void *)mpdu + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset - sizeof(ethernet_header_t));
    len_to_send = length - packet_header_length - pre_llc_offset + ETH_HLEN;

            switch(llc_hdr->type){
                case LLC_TYPE_ARP:

					send_data = virt_eth_queue_create_skb(len_to_send);
            		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
            		eth_hdr = (ethernet_header_t*)send_data->data;

                    memcpy(addr_cache, src_mac, 6);

                    // Insert the Eth source, from the 802.11 header address 2 field
                    memcpy(eth_hdr->src_mac_addr, addr_cache, 6);
                    memcpy(eth_hdr->dest_mac_addr, dst_mac, 6);
                    eth_hdr->ethertype = ETH_TYPE_ARP;

                    // If the ARP packet is addressed to this STA wireless address, replace the ARP dest address
                    // with the connected wired device's MAC address
                    arp = (arp_ipv4_packet_t*)((void*)eth_hdr + sizeof(ethernet_header_t));
//                    if (wlan_addr_eq(arp->target_haddr, get_mac_hw_addr_wlan())) {
                        //memcpy(arp->target_haddr, eth_sta_mac_addr, 6);
//                    }
                break;

                case LLC_TYPE_IP:
//                	printk("rx llc_hdr->type = %d\n",LLC_TYPE_IP);

                 	if(llc_hdr->fragment_offset & ntohs(IP_DF))//分片
    					{

                 		    //check if the number of packet in received queue is reach max value
                 		   //if so, remove the first received packet in the queue
    						if(virt_eth_queue_get_rx_length(veth_priv,llc_hdr->src_nodeId) >= MAX_GROUP_NUM)
    						{

    							virt_eth_queue_remove_rx_first(veth_priv,llc_hdr->src_nodeId);
    							veth_priv->v_jgk_info.rx_out_lose ++;
    						}
    						//find packet from received queue according node id and sequence number
    						if(llc_hdr == NULL || veth_priv == NULL)
    							printk("error\n");
    						group_data = virt_eth_queue_find_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    						//if cannot find packet, then creat a new skb for the sequence number
    						if(group_data == NULL)
    						{
    							//create a new skb struct to store fragment for the certain sequence number
//    							printk("find id %d\n",llc_hdr->seqno);
    							group_data = virt_eth_queue_create_skb(MAC_PKT_BUF);
    							if(group_data!=NULL)
    							{
    								//initial the skb struct
    								virt_eth_add_rx_queue(veth_priv,llc_hdr->src_nodeId,group_data);
    								skb_put(group_data,MAC_PKT_BUF);

    								//initial the tx_queue_buffer_t for record the fragment information
    								tx_group = (tx_queue_buffer_t*)(group_data->data);
    								tx_group->total_len = 0;
    								tx_group->tx_offset = 0;
    								tx_group->seqno = 0;
    								tx_group->flags = 0;

    							}
    						}
    						//if find the packet from the receive queue or create a new skb struct
    						if(group_data != NULL)
    						{
    							//offset is used to record the tail position of payoff for the packet
    							offset = ntohs(llc_hdr->fragment_offset);
    							//get the last 13 bits of llc_hdr->fragment_offset for the offset
    							offset &= IP_OFFSET;
#ifdef Docker_Qualnet
    							if( offset > sizeof(ethernet_header_t) )
    							{
    								// adjust the start position for sender and receiver
    								//the start position of the sender is at the ethernet header
    								//the start position of the receiver is at the mac_header_cstdd
    								offset -= sizeof(ethernet_header_t);
    							}
#endif

//    							printk("offset %d %d\n",offset,llc_hdr->fragment_offset);

    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							tx_group->seqno = llc_hdr->seqno;
    							tx_group->tx_frame_info.timestamp_create = jiffies;

//    							skb_test = virt_eth_queue_find_by_id(llc_hdr->nodeId,llc_hdr->seqno);
//    							if(skb_test == NULL)
//    								printk("--------------------\n");

    							//check if this received fragment is last one
    							if(llc_hdr->fragment_offset & ntohs(IP_LF))
    							{
    								//set the last fragment flag is true (the penultimate bit is set to 1)
    								tx_group->flags |= INET_LAST_IN;
    								//the end is used to record tail position of packet for current reached fragments
    								//add the payoff length for the current received fragment
    								end = offset + llc_hdr->length - packet_header_length;
    								//tx_group->tx_offset record the total length of received fragment
    								tx_group->tx_offset += (length - packet_header_length);
//    								printk("ip 1111 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    								//end should be always larger than tx_group->total_len
    								if(end < tx_group->total_len)
    								{
    									goto err;
    								}
    								tx_group->total_len = end;
    							}
    							//this fragment is not the last one
    							else
    							{
    								//check if the fragment is first one
    								if(offset == 0)
    								{
    									//the recorded length should contain the length of header
    									end = offset + llc_hdr->length;
    									tx_group->tx_offset += llc_hdr->length;
    								}
    								//the fragment is not the first one
    								else
    								{
    									//add the payoff length for the current received fragment(should subtract the header length )
    									end = offset + llc_hdr->length - packet_header_length;

    									tx_group->tx_offset += (llc_hdr->length - packet_header_length);
//    									printk("tx_group->tx_offset %d\n",tx_group->tx_offset);
    								}
    								//check if the recorded tail position (end) of packet is updated
    								if(end > tx_group->total_len)
    									tx_group->total_len = end;

//    								printk("ip 2222 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    							}
    							//copy the received fragment to the skb struct in the received queue
    							//check if the fragment is first one
    							if(offset == 0)
    							{
    								//set the first fragment flag is true (the last bit is set to 1)
    								tx_group->flags |= INET_FIRST_IN;
    								//copy the whole fragment to the skb struct in the received queue
    								memcpy((void*)tx_group->frame+offset,(void*)mpdu, length);
    							}
    							//the fragment is not the first one
    							else
    							{
    								//copy the payoff to the skb struct in the received queue
    								memcpy((void*)tx_group->frame+offset, \
    									(void *)mpdu + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset, \
    									length - packet_header_length - pre_llc_offset);
    							}
    							//check if all fragments have reached, the condition is that
    							//the first and last fragment have been receiveth_hdred,
    							//meanwhile the tail of packet(tx_group->total_len) is equal to the
    							// total length of the received fragments
    							if (tx_group->flags == (INET_FIRST_IN | INET_LAST_IN) &&
    								tx_group->total_len == tx_group->tx_offset)
    							{
    								group_flag = 1;
    								//create ethernet header
    								eth_hdr     = (ethernet_header_t*)((void *)tx_group->frame + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset - sizeof(ethernet_header_t));
    								//the ethernet header length
    								len_to_send = tx_group->tx_offset - sizeof(mac_header_cstdd) - sizeof(llc_header_t) - pre_llc_offset + sizeof(ethernet_header_t);
//    								printk("send_data %d\n",len_to_send);
    								//create new send skb struct
//    								if(len_to_send > 1600)
//    									printk("send_data %d\n",len_to_send);
    								send_data = virt_eth_queue_create_skb(len_to_send);
    								//copy the received packed packet the send skb
    		                		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
    		                		eth_hdr = (ethernet_header_t*)send_data->data;
    		                		//delete the packet from the node's received queue
    		                		virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    		                		group_data = NULL;
//    		                		dev_kfree_skb (group_data);
    							}
    							//the all fragments have not reached, then return, waiting for the others fragment reaching
    							else
    							{
    								return 0;
    							}
    						}
    					}
                 	//the received packet is a whole one, donot fragment
                 	//then copy the packet to the new created skb, send it to batman-adv
                    else{

//                    	    printk("not fragment pk,len_to_send = %d\n",len_to_send);
                    		send_data = virt_eth_queue_create_skb(len_to_send);
                    		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
                    		eth_hdr = (ethernet_header_t*)send_data->data;
                    	}


                 	//vaule the ethernet header
                    memcpy(addr_cache, src_mac, 6);
    				memcpy(eth_hdr->dest_mac_addr, dst_mac, 6);
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
    				memcpy(eth_hdr->src_mac_addr, test_mac, 6);
#else
                    memcpy(eth_hdr->src_mac_addr, addr_cache, 6);
#endif
					eth_hdr->ethertype = ETH_TYPE_IP;
                break;

                case LLC_TYPE_BAT:
                	//check if the received packet is fragment
                  	if(llc_hdr->fragment_offset & ntohs(IP_DF))//分片
    					{
                  		  //check if the number of packet in received queue is reach max value
                  		   //if so, remove the first received packet in the queue
    						if(virt_eth_queue_get_rx_length(veth_priv,llc_hdr->src_nodeId) >= MAX_GROUP_NUM)
    						{
    							virt_eth_queue_remove_rx_first(veth_priv,llc_hdr->src_nodeId);
    							veth_priv->v_jgk_info.rx_out_lose ++;
//    							printk("MAX_GROUP_NUM\n");
    						}
    						//find packet from received queue according node id and sequence number
    						group_data = virt_eth_queue_find_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    						//if cannot find packet, then creat a new skb for the sequence number
    						if(group_data){
    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							if(time_is_before_jiffies(tx_group->tx_frame_info.timestamp_create + msecs_to_jiffies(5000)))
    							{
    								virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
//    								dev_kfree_skb (group_data);
    								group_data = NULL;
    							}
    						}
    						if(group_data == NULL)
    						{
//    							printk("find id %d\n",llc_hdr->seqno);
    							//create a new skb struct to store fragment for the certain sequence number
    							group_data = virt_eth_queue_create_skb(MAC_PKT_BUF);
    							if(group_data!=NULL)
    							{
    								//initial the skb struct
    								virt_eth_add_rx_queue(veth_priv,llc_hdr->src_nodeId,group_data);
    								skb_put(group_data,MAC_PKT_BUF);
    								//initial the tx_queue_buffer_t for record the fragment information
    								tx_group = (tx_queue_buffer_t*)(group_data->data);
    								tx_group->total_len = 0;
    								tx_group->tx_offset = 0;
    								tx_group->seqno = 0;
    								tx_group->flags = 0;
    							}
    						}
    						//if find the packet from the receive queue or create a new skb struct
    						if(group_data != NULL)
    						{
    							//offset is used to record the tail position of payoff for the packet
    							//get the last 13 bits of llc_hdr->fragment_offset for the offset
    							offset = ntohs(llc_hdr->fragment_offset);
    							offset &= IP_OFFSET;
#ifdef Docker_Qualnet
    							if( offset > sizeof(ethernet_header_t) )
    							{
    								// adjust the start position for sender and receiver
    								//the start position of the sender is at the ethernet header
    								//the start position of the received is at the mac_header_cstdd
    								offset -= sizeof(ethernet_header_t);
    							}
#endif
//    							if( length>100)
//    							printk("node %d,offset %d,llc_hdr->fragment_offset %d,llc_hdr->length %d,llc_hdr->seqno %d\n",\
//    									veth_priv->addr[5],offset,ntohs(llc_hdr->fragment_offset),llc_hdr->length,llc_hdr->seqno);

    							tx_group = (tx_queue_buffer_t*)(group_data->data);
    							tx_group->seqno = llc_hdr->seqno;
    							tx_group->tx_frame_info.timestamp_create = jiffies;

//    							skb_test = virt_eth_queue_find_by_id(llc_hdr->nodeId,llc_hdr->seqno);
//    							if(skb_test == NULL)
//    								printk("--------------------\n");
    							//check if this received fragment is last one
    							if(llc_hdr->fragment_offset & ntohs(IP_LF))
    							{
    								//set the last fragment flag is true (the penultimate bit is set to 1)
    								tx_group->flags |= INET_LAST_IN;
    								//the "end" is used to record tail position of packet for current reached fragments
    								//add the payoff length for the current received fragment
    								end = offset + llc_hdr->length - packet_header_length;
    								//tx_group->tx_offset record the total length of received fragment
    								tx_group->tx_offset += (length - packet_header_length);
//    								printk("ip 1111 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    								//tx_group->total_len should be equal to the value of "end"
    								//tx_group->total_len should be update when the "end" is changed
    								if(end < tx_group->total_len){
    									veth_priv->v_jgk_info.rx_out_lose ++;
    									goto err;
    								}
    								tx_group->total_len = end;
    							}
    							//this fragment is not the last one
    							else
    							{
    								//check if the fragment is first one
    								if(offset == 0)
    								{
    									//the recorded length should contain the length of header
    									end = offset + llc_hdr->length;
    									tx_group->tx_offset += llc_hdr->length;
    								}
    								//the fragment is not the first one
    								else
    								{
    									//add the payoff length for the current received fragment(should subtract the header length )
    									end = offset + llc_hdr->length - packet_header_length;
    									tx_group->tx_offset += llc_hdr->length - packet_header_length;
//    									printk("tx_group->tx_offset %d\n",tx_group->tx_offset);
    								}
    								//check if the recorded tail position (end) of packet is updated
    								if(end > tx_group->total_len)
    									tx_group->total_len = end;

//    								printk("ip 2222 %d %d %d %d %d %d %d\n",llc_hdr->seqno,llc_hdr->fragment_offset,offset,llc_hdr->length,end,tx_group->tx_offset,tx_group->total_len);
    							}
    							//copy the received fragment to the skb struct in the received queue
    							//check if the fragment is first one
    							if(offset == 0)
    							{
    								//set the first fragment flag is true (the last bit is set to 1)
    								tx_group->flags |= INET_FIRST_IN;
    								//copy the whole fragment to the skb struct in the received queue
    								memcpy((void*)tx_group->frame,(void*)mpdu, length);
    							}
    							//the fragment is not the first one
    							else
    							{
    								//copy the payoff to the skb struct in the received queue
    								memcpy((void*)tx_group->frame+offset, \
    									(void *)mpdu + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + pre_llc_offset, \
    									length - packet_header_length - pre_llc_offset);
    							}
    							//check if all fragments have reached, the condition is that
    							//the first and last fragment have been received,
    							//meanwhile the tail of packet(tx_group->total_len) is equal to the
    							// total length of the received fragments (tx_group->tx_offset)
    							if (tx_group->flags == (INET_FIRST_IN | INET_LAST_IN) &&
    								tx_group->total_len == tx_group->tx_offset)
    							{

    								group_flag = 1;
    								//create ethernet header
    								eth_hdr     = (ethernet_header_t*)((void *)tx_group->frame + sizeof(mac_header_cstdd) + sizeof(llc_header_t) + \
    										pre_llc_offset - sizeof(ethernet_header_t));
    								//the ethernet header length
    								len_to_send = tx_group->tx_offset - sizeof(mac_header_cstdd) - sizeof(llc_header_t) - pre_llc_offset \
    										+ sizeof(ethernet_header_t);
//    								printk("send_data %d\n",len_to_send);
    								//create new send skb struct

    								send_data = virt_eth_queue_create_skb(len_to_send);
    								//copy the received packed packet the send skb
    		                		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
    		                		eth_hdr = (ethernet_header_t*)send_data->data;
//    								if(length>100)
//    									printk("llc_hdr->seqno %d all fragments have reached,len_to_send %d",llc_hdr->seqno,len_to_send);


    		                		//delete the packet from the node's received queue
    		                		virt_eth_queue_del_by_id(veth_priv,llc_hdr->src_nodeId,llc_hdr->seqno);
    		                		group_data = NULL;
//    		                		dev_kfree_skb (group_data);
    							}
    							//the all fragments have not reached, then return, waiting for the others fragment reaching
    							else
    							{
    								return 0;
    							}
    						}
    						//the created new skb struct failed
    						else
    						{
//    							printk("group err\n");
    							veth_priv->v_jgk_info.rx_out_lose ++;
    						}
    				}
                  	//the received packet is a whole one, donot fragment
                  	//then copy the packet to the new created skb, send it to batman-adv
                  	else
                    	{
                    		send_data = virt_eth_queue_create_skb(len_to_send);
                    		memcpy(skb_put(send_data,len_to_send), (void*)eth_hdr, len_to_send);
                    		eth_hdr = (ethernet_header_t*)send_data->data;
                    	}
                  	//vaule the ethernet header
                  	memcpy(addr_cache, src_mac, 6);
                  	memcpy(eth_hdr->dest_mac_addr, dst_mac, 6);
                  	memcpy(eth_hdr->src_mac_addr, addr_cache, 6);

					eth_hdr->ethertype = ETH_TYPE_BAT;
					bat_unicast_hdr = (bat_unicast_header_t*)((void*)eth_hdr + sizeof(ethernet_header_t));
#if TCP_LOOPBACK_TEST					
					bat_eth_hdr = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
                    if(bat_eth_hdr->h_proto == ETH_TYPE_IP)
                    {
                    	ip_hdr = (ipv4_header_t*)((void*)bat_eth_hdr + sizeof(struct ethhdr));
						if(ip_hdr->protocol == IPV4_PROT_TCP)
						{
							tcp_hdr =  (tcp_header_t*)((void*)ip_hdr + sizeof(ipv4_header_t));
							printk("CDMA LOOPBACK rec: tcp seq num = %u (%u), skb_len = %d ip_len = %d, dest ip addr %d.%d.%d.%d\n",
								ntohl(tcp_hdr->seq_no),ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1,send_data->len,ntohs(ip_hdr->total_length),NIPQUAD(ip_hdr->dest_ip_addr)); //ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1
						}
                    }

#endif

//					if(bat_unicast_hdr->type == BATADV_IV_OGM){
//					printk("Virt-eth0: Rx ogm done, send to batman-adv, src = %d, dest = %d\n",src_mac[5],dst_mac[5]);
//
//					}
					

#ifdef Radio_220
					bat_unicast_hdr = (bat_unicast_header_t*)((void*)eth_hdr + sizeof(ethernet_header_t));
                    if(bat_unicast_hdr->type == BATADV_UNICAST||
						bat_unicast_hdr->type ==BATADV_BCAST){
						if(llc_hdr->corss_q_ind != veth_priv->channel_num){
							printk("voice channel is not match,dest channel =%d, owner channel =%d\n",llc_hdr->corss_q_ind,veth_priv->channel_num);
							dev_kfree_skb (send_data);
							return -1;
						}


						}

#endif
                break;

                default:
//                	queue_checkin(entry_data);
                    // Invalid or unsupported Eth type; give up and return
                	veth_priv->v_jgk_info.rx_out_lose ++;
                    return -1;
                break;
            }

            if(len_to_send > 1400){
            	udp = (udp_header_t*)((void*)eth_hdr+58);
            	if((ntohs(udp->dest_port)) == UDP_PORT_54321 || (ntohs(udp->dest_port)) == UDP_PORT_54322){
            		veth_priv->v_jgk_info.rx_in_lose ++;
				}
            }

#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST ||TCP_LOOPBACK_TEST
//            printk("do kfree packet\n");
//            virt_eth_test_rx_packet(send_data,dev);
            dev_kfree_skb (send_data);

#else
            //value the skb sturct of send_date

                send_data->dev = dev;
                send_data->protocol = eth_type_trans(send_data, dev);

            send_data->ip_summed = CHECKSUM_UNNECESSARY;
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += len_to_send;
			//send the send_date to batman-adv

            netif_rx_ni(send_data);
#endif

            veth_priv->v_jgk_info.rx_outall ++;

//            if(len_to_send>100)
//            printk("send_data  ok, len_to_send =%d\n",len_to_send);
err:
	return 0;
}

u8 virt_eth_util_rx_process(struct net_device *dev,u8* data){

	struct virt_eth_priv *veth_priv = netdev_priv(dev);
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
	tx_frame_info_t *rx_frame_info = (tx_frame_info_t*)data;
#else
	rx_frame_info_t *rx_frame_info = (rx_frame_info_t*)data;
#endif
	mac_header_cstdd *rxcstdd_hdr;
	llc_header_t*       llc_header;
	llc_header_t*        llc_header_tmp;
	void*               mac_payload;
	u16 data_offset = 0;
	u8 dest_mac[MAC_ADDR_LEN];
	u8 src_mac[MAC_ADDR_LEN];
	u8 unicast_to_me,to_multicast;
	u8 cross_queue_packet=0;
	u8					pre_llc_offset			 = 0;
	u16 				length				     = 0;
    int  i;
	//////////////////
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
	rxcstdd_hdr = (mac_header_cstdd*)(data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
	mac_payload = data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE;
	llc_header_tmp = (llc_header_t*)(data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd));
	printk("Receive cdma loopback data from MAC\n");

#else
	rxcstdd_hdr = (mac_header_cstdd*)(data + sizeof(rx_frame_info_t) + PHY_RX_PKT_BUF_PHY_HDR_SIZE);
	mac_payload = data + sizeof(rx_frame_info_t) + PHY_RX_PKT_BUF_PHY_HDR_SIZE;
	llc_header_tmp = (llc_header_t*)(mac_payload+sizeof(mac_header_cstdd));
#endif

	//////////////////

//	rxcstdd_hdr = (mac_header_cstdd*)((u8*)rx_frame_info + PHY_RX_PKT_BUF_MPDU_OFFSET);
//	mac_payload = (u8*)rx_frame_info + PHY_RX_PKT_BUF_MPDU_OFFSET;


//    printk("Virt-eth0: mac header src id= %d, dest id = %d, llc src_nodeId = %d, dest_nodeId = %d\n",
//    rxcstdd_hdr->src_id,rxcstdd_hdr->dest_id,llc_header_tmp->src_nodeId,llc_header_tmp->dest_nodeId);
	//get destination node's mac address
	if(rxcstdd_hdr->dest_id == 0xff)
	{
		memcpy(dest_mac,def_multicast_mac1,MAC_ADDR_LEN);
	}
	else{
		memcpy(dest_mac,def_mac1,MAC_ADDR_LEN);
		dest_mac[5] = rxcstdd_hdr->dest_id;
	}
	//get source node's mac address
	if(rxcstdd_hdr->src_id == 0xff)
	{
		memcpy(src_mac,def_multicast_mac1,MAC_ADDR_LEN);
	}
	else{
		memcpy(src_mac,def_mac1,MAC_ADDR_LEN);
		src_mac[5] = rxcstdd_hdr->src_id;
	}
	//check if the packet is sent to me
	unicast_to_me = wlan_addr_eq(dest_mac, veth_priv->addr);
	to_multicast  = wlan_addr_eq(dest_mac,def_multicast_mac1);
	cross_queue_packet =(llc_header_tmp->corss_q_ind==CONDITION_TURE);
#if VIRT_ETH_TEST
	unicast_to_me = 1;
#endif

	//////////////
	virt_eth_station_rx_process(src_mac,dev);
	//check if the packet's destiantion is me or destination address is broadcast/mulitcat address
	if(1/*unicast_to_me || to_multicast||cross_queue_packet*/){
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
		data_offset = rx_frame_info->length;
#else
		//record the remaining unpack length
		data_offset = rx_frame_info->phy_details.length;
#endif
//		if(llc_header_tmp->corss_q_ind==CONDITION_TURE)
//		printk("corss queue pk rx_process,dest= %d,src= %d,length=%d\n",\
//				rxcstdd_hdr->dest_id,rxcstdd_hdr->src_id,data_offset);
//		data_offset = 748;
//		if(data_offset > sizeof(mac_header_cstdd)){
//			memcpy((u8*)rx_80211,rx_80211_header,sizeof(mac_header_80211));
//		}
//		else
//			break;
//		printk("unicast_to_me %d\n",(sizeof(mac_header_cstdd) + sizeof(llc_header_t) + WLAN_PHY_FCS_NBYTES));
		i = 0;
		while(1)
		{
			i++;
//			if(cross_queue_packet)
//			printk("node %d,do while data_offset %d, cycle %d\n",veth_priv->addr[5],data_offset,i);
//			printk("do while date_offset %d, cycle %d\n",data_offset,i);
			//check if there is packet to unpack
			if(data_offset <= (sizeof(mac_header_cstdd) + sizeof(llc_header_t) ))
				break;
			//get llc_header
			llc_header = (llc_header_t*)(mac_payload+sizeof(mac_header_cstdd) + pre_llc_offset);
			//length contain mac_header_cstdd + llc_header+ payoff
			length = llc_header->length;
//			length = llc_header_1.length +WLAN_PHY_FCS_NBYTES;
//								printk("llc_header length-%d %d\n",length,data_offset);
//			if(cross_queue_packet)
//			printk("node %d,llc_header length=%d, data_offset=%d\n",veth_priv->addr[5],length,data_offset);
			//check if length is correct
			if((length < sizeof(mac_header_cstdd)+sizeof(llc_header_t)) || (length > data_offset)/* || ((u32)mac_payload - ((u32)mac_payload_ptr_u8) +  length >= PKT_BUF_SIZE)*/)
			{
//									printk("llc_header---- length-%d %d\n",length,data_offset);
				veth_priv->v_jgk_info.rx_out_lose ++;
				break;
			}
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
			virt_eth_util_send_data(mac_payload, length, pre_llc_offset,src_mac,dest_mac,dev);
#else
			if(llc_header->dest_nodeId == veth_priv->addr[5] || llc_header->dest_nodeId == 0xff )
			{
				//unpack this fragment for this llc
			//	dest_mac[5] = llc_header->dest_nodeId;

				//get destination node's mac address
				if(llc_header->dest_nodeId == 0xff)
				{
					memcpy(dest_mac,def_multicast_mac1,MAC_ADDR_LEN);
				}
				else{
					memcpy(dest_mac,def_mac1,MAC_ADDR_LEN);
					dest_mac[5] = llc_header->dest_nodeId;
				}


				virt_eth_util_send_data(mac_payload, length, pre_llc_offset,src_mac,dest_mac,dev);
			}
#endif
			//left shift sizeof(mac_header_cstdd), due to the following fragment don't have mac_header_cstdd,
			//the operation is to guarantee mac_payload+sizeof(mac_header_cstdd) to get llc header for the next cycle
			length -= sizeof(mac_header_cstdd) ;
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
#if VIRT_ETH_TEST || CDMA_AMP_LOOPBACK_TEST
#else
		if(rx_frame_info->phy_details.length > 1000){
//			printk("not me\n");
			veth_priv->v_jgk_info.rx_out_lose ++;
		}
#endif
	}
return 0;
}

u8 virt_eth_util_rx_process_test(struct net_device *dev,u8* data){

	struct virt_eth_priv *veth_priv = netdev_priv(dev);
#if TCP_LOOPBACK_TEST
	tx_frame_info_t *rx_frame_info = (tx_frame_info_t*)data;
#else
	rx_frame_info_t *rx_frame_info = (rx_frame_info_t*)data;
#endif
	mac_header_cstdd *rxcstdd_hdr;
	llc_header_t*       llc_header;
	llc_header_t*        llc_header_tmp;
	void*               mac_payload;
	u16 data_offset = 0;
	u8 dest_mac[MAC_ADDR_LEN];
	u8 src_mac[MAC_ADDR_LEN];
	u8 unicast_to_me,to_multicast;
	u8 cross_queue_packet=0;
	u8					pre_llc_offset			 = 0;
	u16 				length				     = 0;
    int  i;
	u8 dest_id_temp;
	u8 src_id_temp;
	//////////////////
#if TCP_LOOPBACK_TEST


	rxcstdd_hdr = (mac_header_cstdd*)(data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
	mac_payload = data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE;
	llc_header_tmp = (llc_header_t*)(data + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd));

	dest_id_temp = rxcstdd_hdr->dest_id;
	src_id_temp = rxcstdd_hdr->src_id;
	if(dest_id_temp != 0xff)
	{
		rxcstdd_hdr->dest_id = src_id_temp;
		rxcstdd_hdr->src_id  = dest_id_temp;
	}
	else{
        return 0;
	}

//	printk("Receive cdma loopback data dest id %d, src id %d\n",rxcstdd_hdr->dest_id,rxcstdd_hdr->src_id);

#else
	rxcstdd_hdr = (mac_header_cstdd*)(data + sizeof(rx_frame_info_t) + PHY_RX_PKT_BUF_PHY_HDR_SIZE);
	mac_payload = data + sizeof(rx_frame_info_t) + PHY_RX_PKT_BUF_PHY_HDR_SIZE;
	llc_header_tmp = (llc_header_t*)(mac_payload+sizeof(mac_header_cstdd));
#endif

	//////////////////

//	rxcstdd_hdr = (mac_header_cstdd*)((u8*)rx_frame_info + PHY_RX_PKT_BUF_MPDU_OFFSET);
//	mac_payload = (u8*)rx_frame_info + PHY_RX_PKT_BUF_MPDU_OFFSET;


//    printk("Virt-eth0: mac header src id= %d, dest id = %d, llc src_nodeId = %d, dest_nodeId = %d\n",
//    rxcstdd_hdr->src_id,rxcstdd_hdr->dest_id,llc_header_tmp->src_nodeId,llc_header_tmp->dest_nodeId);
	//get destination node's mac address
	if(rxcstdd_hdr->dest_id == 0xff)
	{
		memcpy(dest_mac,def_multicast_mac1,MAC_ADDR_LEN);
	}
	else{
		memcpy(dest_mac,def_mac1,MAC_ADDR_LEN);
		dest_mac[5] = rxcstdd_hdr->dest_id;
	}
	//get source node's mac address
	if(rxcstdd_hdr->src_id == 0xff)
	{
		memcpy(src_mac,def_multicast_mac1,MAC_ADDR_LEN);
	}
	else{
		memcpy(src_mac,def_mac1,MAC_ADDR_LEN);
		src_mac[5] = rxcstdd_hdr->src_id;
	}
	//check if the packet is sent to me
	unicast_to_me = wlan_addr_eq(dest_mac, veth_priv->addr);
	to_multicast  = wlan_addr_eq(dest_mac,def_multicast_mac1);
	cross_queue_packet =(llc_header_tmp->corss_q_ind==CONDITION_TURE);
#if TCP_LOOPBACK_TEST
	unicast_to_me = 1;
#endif

	//////////////
	virt_eth_station_rx_process(src_mac,dev);
	//check if the packet's destiantion is me or destination address is broadcast/mulitcat address
	if(1/*unicast_to_me || to_multicast||cross_queue_packet*/){
#if TCP_LOOPBACK_TEST
		data_offset = rx_frame_info->length;
#else
		//record the remaining unpack length
		data_offset = rx_frame_info->phy_details.length;
#endif
//		if(llc_header_tmp->corss_q_ind==CONDITION_TURE)
//		printk("corss queue pk rx_process,dest= %d,src= %d,length=%d\n",\
//				rxcstdd_hdr->dest_id,rxcstdd_hdr->src_id,data_offset);
//		data_offset = 748;
//		if(data_offset > sizeof(mac_header_cstdd)){
//			memcpy((u8*)rx_80211,rx_80211_header,sizeof(mac_header_80211));
//		}
//		else
//			break;
//		printk("unicast_to_me %d\n",(sizeof(mac_header_cstdd) + sizeof(llc_header_t) + WLAN_PHY_FCS_NBYTES));
		i = 0;
		while(1)
		{
			i++;
//			if(cross_queue_packet)
//			printk("node %d,do while data_offset %d, cycle %d\n",veth_priv->addr[5],data_offset,i);
//			printk("do while date_offset %d, cycle %d\n",data_offset,i);
			//check if there is packet to unpack
			if(data_offset <= (sizeof(mac_header_cstdd) + sizeof(llc_header_t) ))
				break;
			//get llc_header
			llc_header = (llc_header_t*)(mac_payload+sizeof(mac_header_cstdd) + pre_llc_offset);
			//length contain mac_header_cstdd + llc_header+ payoff
			length = llc_header->length;
//			length = llc_header_1.length +WLAN_PHY_FCS_NBYTES;
//								printk("llc_header length-%d %d\n",length,data_offset);
//			if(cross_queue_packet)
//			printk("node %d,llc_header length=%d, data_offset=%d\n",veth_priv->addr[5],length,data_offset);
			//check if length is correct
			if((length < sizeof(mac_header_cstdd)+sizeof(llc_header_t)) || (length > data_offset)/* || ((u32)mac_payload - ((u32)mac_payload_ptr_u8) +  length >= PKT_BUF_SIZE)*/)
			{
//									printk("llc_header---- length-%d %d\n",length,data_offset);
				veth_priv->v_jgk_info.rx_out_lose ++;
				break;
			}
#if TCP_LOOPBACK_TEST
            dest_id_temp = llc_header->dest_nodeId;
            src_id_temp  = llc_header->src_nodeId;
			if(dest_id_temp != 0xff)
			{
				llc_header->dest_nodeId = src_id_temp;
				llc_header->src_nodeId  = dest_id_temp;
				virt_eth_util_send_data_test(mac_payload, length, pre_llc_offset,src_mac,dest_mac,dev);
			}
            
			
#else
			if(llc_header->dest_nodeId == veth_priv->addr[5] || llc_header->dest_nodeId == 0xff )
			{
				//unpack this fragment for this llc
			//	dest_mac[5] = llc_header->dest_nodeId;

				//get destination node's mac address
				if(llc_header->dest_nodeId == 0xff)
				{
					memcpy(dest_mac,def_multicast_mac1,MAC_ADDR_LEN);
				}
				else{
					memcpy(dest_mac,def_mac1,MAC_ADDR_LEN);
					dest_mac[5] = llc_header->dest_nodeId;
				}


				virt_eth_util_send_data(mac_payload, length, pre_llc_offset,src_mac,dest_mac,dev);
			}
#endif
			//left shift sizeof(mac_header_cstdd), due to the following fragment don't have mac_header_cstdd,
			//the operation is to guarantee mac_payload+sizeof(mac_header_cstdd) to get llc header for the next cycle
			length -= sizeof(mac_header_cstdd) ;
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
#if TCP_LOOPBACK_TEST
#else
		if(rx_frame_info->phy_details.length > 1000){
//			printk("not me\n");
			veth_priv->v_jgk_info.rx_out_lose ++;
		}
#endif
	}
return 0;
}

