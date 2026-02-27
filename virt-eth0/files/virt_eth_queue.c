/*
 * virt_eth_queue.c
 *
 *  Created on: 2020-4-15
 *      Author: lu
 */

#include <linux/io.h>
#include <linux/slab.h>

#include "virt_eth_queue.h"
#include "virt_eth_util.h"
#include "virt_eth_station.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_dma.h"
#include "virt_eth_frame.h"
#include "virt_eth_system.h"
#include "virt_eth_jgk.h"
#include "virt_eth_dma.h"


//u8 select_bcast_queue = 1;
//1231
//u8 VIRT_ETH_BRAM_NUM[7];

u16 taget_tcp_llc_no;

void virt_eth_queue_hash_init(struct virt_eth_hashtable *hash)
{
	u32 i;

	for (i = 0; i < hash->size; i++) {
		INIT_HLIST_HEAD(&hash->table[i]);
		spin_lock_init(&hash->list_locks[i]);
	}
//1231
//	for(i = 0; i < 7; i ++){
//		VIRT_ETH_BRAM_NUM[i] = 1;
//	}
}

/* free only the hashtable and the hash itself. */
void virt_eth_queue_hash_destroy(struct virt_eth_hashtable *hash)
{
	kfree(hash->list_locks);
	kfree(hash->table);
	kfree(hash);
}


u8 virt_eth_queue_init(struct net_device *dev){

	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	int i = 0;
	//veth_priv->virt_tx_queue =  (virt_eth_tx_queue *)kmalloc(MAX_QUEUE_NUM*sizeof(virt_eth_tx_queue),GFP_KERNEL);
	//veth_priv->virt_rx_queue =  (virt_eth_tx_queue *)kmalloc(MAX_QUEUE_NUM*sizeof(virt_eth_tx_queue),GFP_KERNEL);

	for(i=0;i < MAX_QUEUE_NUM; i ++){
		skb_queue_head_init(&veth_priv->virt_tx_queue[i].txq);
		veth_priv->virt_tx_queue[i].total_len = 0;
		skb_queue_head_init(&veth_priv->virt_rx_queue[i].txq);
	}
//	printk("queue initial complete virt_tx_queue \n");
//	printk("virt_tx_queue = 0x%p,virt_rx_queue = 0x%p\n",(void *)veth_priv->virt_tx_queue,(void *)veth_priv->virt_rx_queue);
	taget_tcp_llc_no = -1;

	return 0;
}

struct sk_buff* virt_eth_queue_create_skb(u32 len){
	struct sk_buff* skbuf = NULL;
	skbuf = dev_alloc_skb(len);

	return skbuf;
}

void virt_eth_queue_del_all(struct virt_eth_priv *veth_priv, u8 txq_id){
	skb_queue_purge(&veth_priv->virt_tx_queue[txq_id].txq);
}



int virt_eth_queue_get_id(void){
	int id = 0;

	return id;
}

struct sk_buff* virt_eth_queue_tx_dequeue(struct virt_eth_priv *veth_priv, u8 txq_id){
	return skb_dequeue(&veth_priv->virt_tx_queue[txq_id].txq);
}

void virt_eth_queue_tx_add_head(struct virt_eth_priv *veth_priv, u8 txq_id,struct sk_buff* skb){
	skb_queue_head(&veth_priv->virt_tx_queue[txq_id].txq,skb);
}

u16 virt_eth_queue_get_length(struct virt_eth_priv *veth_priv, u16 id){
	if(id >= MAX_QUEUE_NUM)
		return 0;
	return veth_priv->virt_tx_queue[id].txq.qlen;
}

u32 virt_eth_queue_get_tx_length(struct virt_eth_priv *veth_priv, u8 nodeid){
	return veth_priv->virt_tx_queue[STATION_ID_TO_TCP_QUEUE_ID(nodeid)].txq.qlen + veth_priv->virt_tx_queue[STATION_ID_TO_QUEUE_ID(nodeid)].txq.qlen;
}

static u8 virt_eth_get_tx_baram_addr(u8 type){
	u8 i = NUM_TX_PKT_BUF;
	tx_frame_info_t *tx_frame_info;
	u8* data;

//	printk("virt_eth_get_tx_baram_addr, type = %d\n",type);
	if(type == UNICAST_TYPE){
		for(i = 0; i < UNICAST_TX_PKT_BUF; i ++){
			data = ioremap(TX_PKT_BUF(i),sizeof(tx_frame_info_t));

			tx_frame_info = (tx_frame_info_t*)data;

			if(tx_frame_info->tx_pkt_buf_state == TX_PKT_BUF_h_u){
				iounmap(data);
				break;
			}
			iounmap(data);
		}
		if(i == UNICAST_TX_PKT_BUF)
			i = NUM_TX_PKT_BUF;
		goto ret;
	}else if(type == MULTICAST_TYPE){
		for(i = UNICAST_TX_PKT_BUF; i < MULTICAST_TX_PKT_BUF; i ++){
			data = ioremap(TX_PKT_BUF(i),sizeof(tx_frame_info_t));

			tx_frame_info = (tx_frame_info_t*)data;

			if(tx_frame_info->tx_pkt_buf_state == TX_PKT_BUF_h_b){
				iounmap(data);
				break;
			}
			iounmap(data);
		}
		if(i == MULTICAST_TX_PKT_BUF)
			i = NUM_TX_PKT_BUF;
		goto ret;
	}
ret:
	return i;
	//return i;
}
//1231
//static u8 virt_eth_get_tx_baram_addr(u8 type){
//	u8 i = NUM_TX_PKT_BUF;
//	tx_frame_info_t *tx_frame_info;
//	u8* data;
//
////	if(VIRT_ETH_BRAM_NUM[0] == 1){
////		return 0;
////	}
////	else
////		return NUM_TX_PKT_BUF;
//
////	printk("virt_eth_get_tx_baram_addr, type = %d\n",type);
//	if(type == UNICAST_TYPE){
//		for(i = 0; i < UNICAST_TX_PKT_BUF; i ++){
//			if(VIRT_ETH_BRAM_NUM[i] == 1){
//				break;
//			}
//		}
//		if(i == UNICAST_TX_PKT_BUF)
//			i = NUM_TX_PKT_BUF;
//		goto ret;
//	}else if(type == MULTICAST_TYPE){
//		for(i = UNICAST_TX_PKT_BUF; i < MULTICAST_TX_PKT_BUF; i ++){
//			if(VIRT_ETH_BRAM_NUM[i] == 1){
//				break;
//			}
//		}
//		if(i == MULTICAST_TX_PKT_BUF)
//			i = NUM_TX_PKT_BUF;
//		goto ret;
//	}
//ret:
//	return i;
//	//return i;
//}


u8 virt_eth_queue_dequeue_transmit(struct net_device *dev,u8 node_id,u16 txq_id,u8 mcs){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	bool first_frame = true;
	u8   ret = 0;
	u32 mcs_len = 0;
	struct sk_buff* skb;
#ifdef Docker_Qualnet
	struct sk_buff* skb_packed;
	struct sk_buff* skb_send;
#endif
	tx_frame_info_t* 	tx_frame_info = NULL;
	tx_queue_buffer_t* 	tx_frame_info_tmp = NULL;
	u8   OGM_DEQUEUE_CONDITION=0;
	u8   UDP_DEQUEUE_CONDITION=0;
//	dma_addr_t txaddr;
	u8* txbuf = NULL;
	u8* data = NULL;
	u8 tx_pkt_buf = 0;
	u16 q_len = 0;
	u8 type = UNICAST_TYPE;
	virt_eth_pkt_ready tx_pkt_ready;
	int i;
	int mcs_queue_length;
	u8 cross_queue_list[MAX_NODE];
	u8 cross_q_node_mcs;
	u16 txq_id2;
	u16 txq_tmp;
	icmp_header_t            * icmp_hdr;
	ipv4_header_t            * ip_hdr;
	struct ethhdr            * bat_eth_hdr;
	arp_ipv4_packet_t        * arp;
	bat_unicast_header_t     * bat_unicast_hdr;
	bat_bcast_header_t       * bat_bcast_hdr;


	void __iomem *reg_addr;
    u32 read_slot_id;
	int packed_pk_cnt;


#if VIRT_ETH_TEST || TCP_LOOPBACK_TEST
	u8* rxbuf = NULL;
#endif


	
	for(i=0;i<MAX_NODE;i++)
	{
		cross_queue_list[i]=0;
	}
	q_len = virt_eth_queue_get_length(veth_priv,txq_id);
//	if(CHECK_UDP_QUEUE(txq_id)){
//		if(q_len < 2000)
//			goto err;
//	}

	if(q_len == 0)
		goto err;

	if(node_id == MAX_NODE)
	{
		if(veth_priv->mcs_mode == FIX_MCS_MODE)
			mcs = veth_priv->bcast_mcs;
		mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);
	}
	else
	{
		if(veth_priv->mcs_mode == FIX_MCS_MODE)
			mcs = veth_priv->ucast_mcs;

		mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);
	}
//	if(txq_id == MCAST_QID)
//		printk("OGM virt_eth_queue_dequeue_transmit start,txq_id=%d,q_len=%d,mcs = %d\n",txq_id,q_len,mcs);
	//OGM packet
	if(txq_id == MCAST_QID)
	{
		//队首的包
		skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);
		if(skb==NULL)
			goto err;
		tx_frame_info_tmp = ((tx_queue_buffer_t*)skb->data);
//		printk("waiting time condition =  %d",\
//				time_is_before_jiffies(tx_frame_info_tmp->tx_frame_info.timestamp_create + msecs_to_jiffies(MAX_WAITING_TIME_OGM)));

		//处于队首的包在队列中等待的时间大于MAX_WAITING_TIME_OGM(ms)or 队列中包个数大于11
		if( time_is_before_jiffies(tx_frame_info_tmp->tx_frame_info.timestamp_create + msecs_to_jiffies(MAX_WAITING_TIME_OGM)) || q_len>MAC_OGM_NUM_WAIT)
		{
			OGM_DEQUEUE_CONDITION = 1;
//			printk("OGM_DEQUEUE_CONDITION = %d\n",OGM_DEQUEUE_CONDITION);
		}
		//enqueue the packet
		virt_eth_queue_tx_add_head(veth_priv,txq_id,skb);
		if(OGM_DEQUEUE_CONDITION == 0)
		{
			goto err;
		}
	}

//	if(q_len < 5 && txq_id>MANAGEMENT_QID)
	//UDP queue packet should packed by force
//	printk("txq_id = %d,CHECK_UDP_QUEUE result = %d, ",txq_id,CHECK_UDP_QUEUE(txq_id));
	if(CHECK_UDP_QUEUE(txq_id))
	{

		//get the total packet length of same or lager mcs queue
		mcs_queue_length =virt_eth_queue_get_larger_mcs_txqueue_total_length(dev,mcs);
		//get the mcs length
		mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);
		//队首的包
		skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);
		if(skb==NULL)
			goto err;
		tx_frame_info_tmp = ((tx_queue_buffer_t*)skb->data);
		//处于队首的包在队列中等待的时间大于MAX_WAITING_TIME_UDP(ms) ormcs_queue_length larger than mcs_len
		if( time_is_before_jiffies(tx_frame_info_tmp->tx_frame_info.timestamp_create + msecs_to_jiffies(MAX_WAITING_TIME_UDP)) || mcs_queue_length>=mcs_len)
		{
			UDP_DEQUEUE_CONDITION = 1;
		}
		//enqueue the packet
		virt_eth_queue_tx_add_head(veth_priv,txq_id,skb);
		if(UDP_DEQUEUE_CONDITION == 0)
		{
			goto err;
		}
	}




#ifdef Zynq_Platform

#ifdef Radio_7800
	type = MULTICAST_TYPE;
#else
	if(txq_id <= MANAGEMENT_QID){
		type = MULTICAST_TYPE;
	}
#endif

	//if(txq_id == MANAGEMENT_QID){
	//	type = UNICAST_TYPE;
	//}

//	//UNICAST_TYPE 1022
//	type = UNICAST_TYPE;

	tx_pkt_buf = virt_eth_get_tx_baram_addr(type);
//	if(txq_id == MCAST_QID)
//        printk("done virt_eth_get_tx_baram_addr,tx_pkt_buf = %d",tx_pkt_buf);
	if(tx_pkt_buf == NUM_TX_PKT_BUF)
	{
//		if(txq_id == MCAST_QID)
//		printk("There are max number packet in baram, no room left for other pk\n");
		ret = 1;
		goto err;
	}
#endif


#if VIRT_ETH_TEST
//	mcs = 4;
//	mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);

#else

#endif
//	printk("get mcs_len =%d, mcs = %d",mcs_len,mcs);

	if(mcs_len == 0 || mcs == 0x0f){
		goto setstate;
	}
	else{

		//******************************************************************************
		// test
//		skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);
//		ret = dev_queue_xmit(skb);
//		return 1;
		//******************************************************************************


		//printk("mcs_len %d\n",mcs_len + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
//		printk("node %d, mcs_len %d, txq_id %d q_len %d,\n",veth_priv->addr[5],mcs_len,txq_id,q_len);

//		spin_lock_bh(&veth_priv->dma_tx_lock);
#ifdef Docker_Qualnet
		skb_packed = virt_eth_queue_create_skb(mcs_len+ sizeof(tx_frame_info_t));
        txbuf = skb_put(skb_packed,mcs_len + sizeof(tx_frame_info_t));
#elif defined Zynq_Platform
		txbuf = virt_eth_dma_get_tx_buffer((int)tx_pkt_buf);
#endif
		//printk("virt_eth_queue_dequeue_transmit 0x%p\n",(void*)txbuf);

		//txbuf = virt_eth_dma_create_buf(&txaddr,mcs_len + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
		if(txbuf == NULL){
			ret = 1;
			goto setstate;
		}

		data = txbuf;
//		printk("src dma_addr_t 0x%p\n",(void*)txbuf);
		//if(txbuf )
		i = 0;
		cross_q_node_mcs = mcs;
		txq_tmp = txq_id;
		packed_pk_cnt = 0;
		while(ret == 0){
			//get packet from queue

			i++;

			//printk("virt_eth_queue_tx_dequeue veth_priv->virt_tx_queue[%d]\n",txq_id);
			skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);

			//printk("Done virt_eth_queue_tx_dequeue\n");
			if(skb == NULL)
			{
				//printk("skb == NULL\n");
				//there is room for packed
				// get cross queue id
//				if(txq_id == MANAGEMENT_QID)
//				{
//					printk("txq_id=%d is empty,mcs_len =%d, mcs = %d go to cross queue packed\n",txq_id,mcs_len,mcs,q_len);
//					txq_id = 23;
//					q_len = virt_eth_queue_get_length(veth_priv,txq_id);

//					break;
//				}
//				if(txq_id == 23)
//					break;
//				txq_id2 = txq_id;
//				txq_id = 23;


				txq_id = virt_eth_get_cross_queue_id(veth_priv,txq_id, mcs, cross_queue_list);
				q_len = virt_eth_queue_get_length(veth_priv,txq_id);
				if(q_len>0 && txq_id!=0xffff)
				{
					//get the queue's mcs
					cross_q_node_mcs = virt_eth_mgmt_get_mcs(veth_priv,QUEUE_ID_TO_STATION_ID(txq_id));

					//record the cross queue packed in the first LLC header
#ifndef Radio_220
					((llc_header_t*)(txbuf+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind = CONDITION_TURE;
//					printk("txq_id=%d is empty, cross_queue_id = %d,q_len = %d,cross_q_node_mcs=%d,cross_ind = %d\n",\
							txq_id2,txq_id,q_len,cross_q_node_mcs,((llc_header_t*)(data+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind);
#endif
					continue;
				}


//				printk("get skb error\n");
				//if there no more packet for packed, packed/unpacked is over
				break;
			}

			veth_priv->virt_tx_queue[txq_id].total_len -= skb->len;
			//printk("node %d get the %d st packet,go to virt_eth_frame_aggre_ampdu_transmit\n",veth_priv->addr[5],i);

#ifdef Docker_Qualnet
			skb_packed->dev = skb->dev;
#endif
			ret = virt_eth_frame_aggre_ampdu_transmit(veth_priv,first_frame,mcs_len,skb,data,mcs,cross_q_node_mcs);
//            if(ret == Fragment_Initial){
//				packed_pk_cnt++;
//				if(packed_pk_cnt>=4){
//					ret = Fragment_Finish;
//
//				}
//
//			} 
			first_frame = false;
//			printk("ret = %d\n",ret);


			if(ret == Original_PK_Remaining)
			{
//				//printk("readd skb\n");
				//there is part of packet can not be send in this slot
				//add this packet to the queue for the next transmission
				veth_priv->virt_tx_queue[txq_id].total_len += (skb->len - \
						((tx_queue_buffer_t*)(skb->data))->tx_offset - sizeof(llc_header_t) - sizeof(mac_header_cstdd) );
				virt_eth_queue_tx_add_head(veth_priv,txq_id,skb);
				break;
			}
			else
				dev_kfree_skb(skb);

			if(ret == Fragment_Error)
				veth_priv->v_jgk_info.tx_out_lose ++;
		}
#if VIRT_ETH_TEST
//		printk("start\n");
		tx_frame_info = (tx_frame_info_t*)txbuf;
		if(type==UNICAST_TYPE)
		{
			tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_h_u;
		}
		else if(type==MULTICAST_TYPE)
		{
			tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_h_b;
		}

		if(tx_frame_info->length == 0){
			printk("tx_frame_info->length error\n");
		}
//		printk("tx_frame_info->tx_pkt_buf_state= %d\n",tx_frame_info->tx_pkt_buf_state);
//		virt_eth_util_rx_process(dev,txbuf);
//		if(((llc_header_t*)(txbuf+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind == CONDITION_TURE)
//		printk("cross queue pk, dma send data length %d\n",tx_frame_info->length);
		ret = virt_eth_dma_tx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)tx_pkt_buf,tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
		if(ret != 0)
			printk("dma tx error\n");
		//		rx_pkt.rx_pkt_buf = tx_pkt_buf;
//		rx_pkt.buf_len = tx_frame_info->length;
//		rx_pkt.resv = 0;
//		virt_eth_system_rx_data_process(dev,(u8*)&rx_pkt,VIRT_ETH_MGMT_RX_READY_LEN);
//		printk("start 1\n");
		rxbuf = virt_eth_dma_get_rx_buffer(tx_pkt_buf);
		ret = virt_eth_dma_rx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)tx_pkt_buf,tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);

		if(ret != 0)
			printk("dma rx error\n");
//		tx_pkt_ready.rx_pkt_buf = 0;
//		tx_pkt_ready.resv = 2;
//		tx_pkt_ready.buf_len = 0;
//		virt_eth_mgmt_send_msg(dev,MGMT_TX_READY,(u8*)&tx_pkt_ready,sizeof(virt_eth_pkt_ready));
//		printk("start 2\n");
		virt_eth_util_rx_process(dev,rxbuf);

		memset(txbuf,0,PKG_SIZE);
		memset(rxbuf,0,PKG_SIZE);
                return 1;
#endif
		//offset = 
		tx_frame_info = (tx_frame_info_t*)txbuf;
#ifdef Docker_Qualnet
		//check if enough space is available for pulling
		if(skb_packed->len < sizeof(tx_frame_info_t))
		{
			return 0;
		}


		if(tx_frame_info->length < sizeof(ethernet_header_t))
		{
			return 0;
		}
		//create a sent skb_buff
		skb_send = virt_eth_queue_create_skb(tx_frame_info->length);

        skb_send->dev = skb_packed->dev;
        //copy the packed date to send_skb
        memcpy(skb_put(skb_send,tx_frame_info->length),skb_packed->data+sizeof(tx_frame_info_t),tx_frame_info->length);
		//if(tx_frame_info->length >1000)
			//printk("send packet to eth0 length = %d\n",tx_frame_info->length);
    	dev_kfree_skb(skb_packed);
//    	dev_kfree_skb(skb_send);
		ret = dev_queue_xmit(skb_send);
#elif defined Zynq_Platform

		if(type == MULTICAST_TYPE)
			tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_l_b;
		else if(type == UNICAST_TYPE)
			tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_l_u;
//1231
//		VIRT_ETH_BRAM_NUM[tx_pkt_buf] = 0;

//		spin_unlock_bh(&veth_priv->dma_tx_lock);
//		virt_eth_dma_tx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)tx_pkt_buf, sizeof(tx_frame_info_t));
//		ret = virt_eth_dma_tx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)tx_pkt_buf,tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
		//use cdma to send date
		//printk("virt_eth_dma_tx start \n");
		ret = virt_eth_dma_tx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)tx_pkt_buf,/*PKG_SIZE*/tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
//		virt_eth_dma_free_buf(txbuf,txaddr,mcs_len + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
//		if(((llc_header_t*)(data+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind == CONDITION_TURE)
//				printk("cross queue pk dma send data ok, payoff_length = %d, ret= %d",tx_frame_info->length,ret);

#if TCP_LOOPBACK_TEST
		if(CHECK_TCP_QUEUE(txq_id))
		{
			rxbuf = virt_eth_dma_get_rx_buffer(NUM_RX_PKT_BUF-1);
			ret = virt_eth_dma_rx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)(NUM_RX_PKT_BUF-1),tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);
		
				if(ret != 0)
					printk("dma rx error\n");
		//		tx_pkt_ready.rx_pkt_buf = 0;
		//		tx_pkt_ready.resv = 2;
		//		tx_pkt_ready.buf_len = 0;
		//		virt_eth_mgmt_send_msg(dev,MGMT_TX_READY,(u8*)&tx_pkt_ready,sizeof(virt_eth_pkt_ready));
		//		printk("start 2\n");
			virt_eth_util_rx_process_test(dev,rxbuf);
		
			memset(txbuf,0,PKG_SIZE);
			memset(rxbuf,0,PKG_SIZE);
		}

#endif //end for #ifdef TCP_LOOPBACK_TEST


#endif  // end for #elif defined Zynq_Platform
		if(ret == 0){
				
//	        reg_addr =	ioremap(INSTR_BASE+SREG0_OFFSET,4);
//	        read_slot_id = readl(reg_addr);
//			printk("Virt-eth0:dma send sucessfully, payoff_length = %d, ret= %d,slot =%d ",tx_frame_info->length,ret,read_slot_id);
//            iounmap(reg_addr);
			if(node_id != MAX_NODE && txq_id%2 == 1 && tx_frame_info->length > 1000)
				veth_priv->v_jgk_info.tx_outall ++;
			
		}
		else{
			veth_priv->v_jgk_info.tx_out_lose ++;
		}
		////////////////////////////////////////
		//amp send tx buf
#ifdef Zynq_Platform
		tx_pkt_ready.rx_pkt_buf = tx_pkt_buf;
		tx_pkt_ready.resv = type;
		tx_pkt_ready.buf_len = tx_frame_info->length;
		virt_eth_mgmt_send_msg(dev,MGMT_TX_READY,(u8*)&tx_pkt_ready,sizeof(virt_eth_pkt_ready));

//		if(CHECK_TCP_QUEUE(txq_id))
//		{
//			u8*				mac_payload;
//			llc_header_t             * llc_hdr;
//			llc_header_t             *llc_header_tmp;
//			tcp_header_t             * tcp_hdr;
//			int left_pk_len;
//			int data_offset;
//			int data_length;
//			int printf_len;
//			int ii;
//
//			data_offset =  tx_frame_info->length;
//
//			while(1){
//
//				if(data_offset <= (sizeof(mac_header_cstdd) + sizeof(llc_header_t) ))
//					break;
//				mac_payload = txbuf + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE;
//				llc_hdr = (llc_header_t*)(mac_payload+sizeof(mac_header_cstdd) );
//				data_length = llc_hdr->length;
//
//				if(llc_hdr->seqno == taget_tcp_llc_no)
//				{
//					if(ntohs(llc_hdr->fragment_offset)==0)
//					{
//						ip_hdr = (ipv4_header_t*)((void*)txbuf +  sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE + 
//							+ sizeof(mac_header_cstdd) + sizeof(llc_header_t) + sizeof(bat_unicast_header_t)+ sizeof(struct ethhdr));
//						if(ip_hdr->protocol == IPV4_PROT_TCP)
//						{
//							tcp_hdr =  (tcp_header_t*)((void*)ip_hdr + 4*((u8)(ip_hdr->version_ihl) & 0xF));
//							if((ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1) ==TARTET_TCP_ACK_NO)
//							{
//								printk("Veth: send tcp pk seq %d  llc seq %d dest ip addr %d.%d.%d.%d\n",
//									ntohl(tcp_hdr->seq_no),taget_tcp_llc_no,NIPQUAD(ip_hdr->dest_ip_addr));
//							}
//						}
//					
//					}
//					else{
//						
//						if(data_length >= sizeof(mac_header_cstdd) + sizeof(llc_header_t) + 
//							sizeof(bat_unicast_header_t)+ sizeof(struct ethhdr)+sizeof(ipv4_header_t))
//						{
//							printf_len = sizeof(mac_header_cstdd) + sizeof(llc_header_t) + 
//							sizeof(bat_unicast_header_t)+ sizeof(struct ethhdr)+sizeof(ipv4_header_t);
//						}
//						else{
//							printf_len = data_length;
//							
//
//						}
//						
//						for(ii=0;ii<printf_len;ii++)
//						{
//							printk("0x%x ",mac_payload[ii]);
//						}
//
//
//					}
//				}
//				data_length -= sizeof(mac_header_cstdd) ;
//				data_offset -= data_length;
//				mac_payload += data_length;
//
//
//			}
//
//
//
//		}

		//if (txq_tmp == MANAGEMENT_QID)
		//{
		//	ip_hdr = (ipv4_header_t*)((void*)txbuf +  sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE + 
		//		+ sizeof(mac_header_cstdd) + sizeof(llc_header_t) + sizeof(bat_unicast_header_t)+ sizeof(struct ethhdr));
		//	if (ip_hdr->protocol == IPV4_PROT_ICMP)
		//	{
		//		icmp_hdr = (icmp_header_t *)((void*)ip_hdr + sizeof(ipv4_header_t));
		//		printk("Veth: Send ICMP packect to MAC, type = %d, seq_number= %d\n",icmp_hdr->icmp_type,ntohs(icmp_hdr->icmp_sequence));
		//	}
		//	
		//}
		//if (txq_tmp == MCAST_TCP_QID)
		//{
		//	bat_unicast_hdr = (bat_unicast_header_t*)((void*)txbuf +  sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE + 
		//		+ sizeof(mac_header_cstdd) + sizeof(llc_header_t));
		//	if (bat_unicast_hdr->type == BATADV_BCAST)
		//	{
		//		bat_eth_hdr  = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_bcast_header_t));
		//		if (bat_eth_hdr->h_proto == ETH_TYPE_ARP)
		//		{
		//			arp = (arp_ipv4_packet_t*)((void*)bat_eth_hdr + sizeof(ethernet_header_t));
		//			printk("Veth: Send Broadcast ARP packect to mac, bank buffer id = %d, dest node id = %d\n",tx_pkt_buf,arp->target_paddr[3]);
		//		}
		//	}
		//	else if (bat_unicast_hdr->type == BATADV_UNICAST)
		//	{
		//		bat_eth_hdr  = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
		//		if (bat_eth_hdr->h_proto == ETH_TYPE_ARP)
		//		{
		//			arp = (arp_ipv4_packet_t*)((void*)bat_eth_hdr + sizeof(ethernet_header_t));
		//			printk("Veth: Send Unicast ARP packect to mac, bank buffer id = %d, dest node id = %d\n",tx_pkt_buf,arp->target_paddr[3]);
		//		}
		//	}
		//}

		memset(txbuf,0,tx_frame_info->length + sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE);

		switch(txq_tmp){
		case MANAGEMENT_QID:{
			veth_priv->v_jgk_info.ping_slot ++;
			veth_priv->v_jgk_info.ping_out_len += tx_pkt_ready.buf_len;
			//printk("Veth CDMA ping packet out, bank id = %d, ping_slot = %d\n",tx_pkt_buf,veth_priv->v_jgk_info.ping_slot);
			break;
		}
		case MCAST_QID:{
			veth_priv->v_jgk_info.ogm_slot ++;
			veth_priv->v_jgk_info.ogm_out_len += tx_pkt_ready.buf_len;
		    //printk("Veth OGM  packet out, bank id = %d, ogm_slot = %d\n",tx_pkt_buf,veth_priv->v_jgk_info.ogm_slot);
			break;
		}
		case MCAST_TCP_QID:{
			veth_priv->v_jgk_info.bcast_slot ++;
			veth_priv->v_jgk_info.bcast_out_len += tx_pkt_ready.buf_len;
			break;
		}
		default:{
			veth_priv->v_jgk_info.ucast_slot ++;
			veth_priv->v_jgk_info.ucast_out_len += tx_pkt_ready.buf_len;
			break;
		}
		}



#endif //for VIRT_ETH_TEST
//		printk("dma send ok mcs_len buf %d %d %d\n",tx_pkt_buf,mcs_len,tx_frame_info->length);
	}
	return 1;

setstate:
err:
	return ret;

}

u8 virt_eth_queue_dequeue_transmit_1(struct net_device *dev,u8 node_id,u16 txq_id,u8 mcs){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	bool first_frame = true;
	u8   ret = 0;
	u32 mcs_len = 0;
	struct sk_buff* skb;
#ifdef Docker_Qualnet
	struct sk_buff* skb_packed;
	struct sk_buff* skb_send;
#endif
	tx_frame_info_t* 	tx_frame_info = NULL;
	tx_queue_buffer_t* 	tx_frame_info_tmp = NULL;
	u8   OGM_DEQUEUE_CONDITION=0;
	u8   UDP_DEQUEUE_CONDITION=0;
	u8   au8TxBuf[MAC_PKT_BUF] = {0};
	u8* txbuf = NULL;
	u8* data = NULL;
	u8 tx_pkt_buf = 0;
	u16 q_len = 0;
	u8 type = UNICAST_TYPE;
	virt_eth_pkt_ready tx_pkt_ready;
	int i;
	int mcs_queue_length;
	u8 cross_queue_list[MAX_NODE];
	u8 cross_q_node_mcs;
	u16 txq_id2;
	u16 txq_tmp;
	icmp_header_t            * icmp_hdr;
	ipv4_header_t            * ip_hdr;
	struct ethhdr            * bat_eth_hdr;
	arp_ipv4_packet_t        * arp;
	bat_unicast_header_t     * bat_unicast_hdr;
	bat_bcast_header_t       * bat_bcast_hdr;


	void __iomem *reg_addr;
    u32 read_slot_id;
	int packed_pk_cnt;

	
	for(i=0;i<MAX_NODE;i++)
	{
		cross_queue_list[i]=0;
	}
	q_len = virt_eth_queue_get_length(veth_priv,txq_id);


	if(q_len == 0)
		goto err;

	if(node_id == MAX_NODE)
	{
		if(veth_priv->mcs_mode == FIX_MCS_MODE)
			mcs = veth_priv->bcast_mcs;
		mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);
	}
	else
	{
		if(veth_priv->mcs_mode == FIX_MCS_MODE)
			mcs = veth_priv->ucast_mcs;

		mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);
	}

	if(txq_id == MCAST_QID)
	{

		skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);
		if(skb==NULL)
			goto err;
		tx_frame_info_tmp = ((tx_queue_buffer_t*)skb->data);

		if( time_is_before_jiffies(tx_frame_info_tmp->tx_frame_info.timestamp_create + msecs_to_jiffies(MAX_WAITING_TIME_OGM)) || q_len>MAC_OGM_NUM_WAIT)
		{
			OGM_DEQUEUE_CONDITION = 1;

		}

		virt_eth_queue_tx_add_head(veth_priv,txq_id,skb);
		if(OGM_DEQUEUE_CONDITION == 0)
		{
			goto err;
		}
	}

	if(CHECK_UDP_QUEUE(txq_id))
	{
		mcs_queue_length =virt_eth_queue_get_larger_mcs_txqueue_total_length(dev,mcs);

		mcs_len = virt_eth_mgmt_get_mcs_len_by_mcs(veth_priv,mcs);

		skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);
		if(skb==NULL)
			goto err;
		tx_frame_info_tmp = ((tx_queue_buffer_t*)skb->data);

		if( time_is_before_jiffies(tx_frame_info_tmp->tx_frame_info.timestamp_create + msecs_to_jiffies(MAX_WAITING_TIME_UDP)) || mcs_queue_length>=mcs_len)
		{
			UDP_DEQUEUE_CONDITION = 1;
		}

		virt_eth_queue_tx_add_head(veth_priv,txq_id,skb);
		if(UDP_DEQUEUE_CONDITION == 0)
		{
			goto err;
		}
	}

	if(txq_id <= MANAGEMENT_QID){
		type = MULTICAST_TYPE;
	}

	///////////////////////////////
	tx_pkt_buf = virt_eth_get_tx_baram_addr(type);

	if(tx_pkt_buf == NUM_TX_PKT_BUF)
	{
		ret = 1;
		goto err;
	}

	if(mcs_len == 0 || mcs == 0x0f){
		goto setstate;
	}
	else{

/////////////////////////////////
		txbuf = au8TxBuf;//virt_eth_dma_get_tx_buffer((int)tx_pkt_buf);


		if(txbuf == NULL){
			ret = 1;
			goto setstate;
		}

		data = txbuf;

		i = 0;
		cross_q_node_mcs = mcs;
		txq_tmp = txq_id;
		packed_pk_cnt = 0;
		while(ret == 0){
			i++;

			skb = virt_eth_queue_tx_dequeue(veth_priv,txq_id);

			if(skb == NULL)
			{
				txq_id = virt_eth_get_cross_queue_id(veth_priv,txq_id, mcs, cross_queue_list);
				q_len = virt_eth_queue_get_length(veth_priv,txq_id);
				if(q_len>0 && txq_id!=0xffff)
				{
			
					cross_q_node_mcs = virt_eth_mgmt_get_mcs(veth_priv,QUEUE_ID_TO_STATION_ID(txq_id));

					
#ifndef Radio_220
					((llc_header_t*)(txbuf+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind = CONDITION_TURE;
//					printk("txq_id=%d is empty, cross_queue_id = %d,q_len = %d,cross_q_node_mcs=%d,cross_ind = %d\n",\
							txq_id2,txq_id,q_len,cross_q_node_mcs,((llc_header_t*)(data+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind);
#endif
					continue;
				}



				break;
			}

			veth_priv->virt_tx_queue[txq_id].total_len -= skb->len;

			ret = virt_eth_frame_aggre_ampdu_transmit(veth_priv,first_frame,mcs_len,skb,data,mcs,cross_q_node_mcs);

			first_frame = false;

			if(ret == Original_PK_Remaining)
			{

				veth_priv->virt_tx_queue[txq_id].total_len += (skb->len - \
						((tx_queue_buffer_t*)(skb->data))->tx_offset - sizeof(llc_header_t) - sizeof(mac_header_cstdd) );
				virt_eth_queue_tx_add_head(veth_priv,txq_id,skb);
				break;
			}
			else
				dev_kfree_skb(skb);

			if(ret == Fragment_Error)
				veth_priv->v_jgk_info.tx_out_lose ++;
		}


		tx_frame_info = (tx_frame_info_t*)txbuf;


		if(type == MULTICAST_TYPE)
			tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_l_b;
		else if(type == UNICAST_TYPE)
			tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_l_u;

		//ret = virt_eth_dma_tx((dma_addr_t)(TX_PKT_BUF(tx_pkt_buf)),(int)tx_pkt_buf,/*PKG_SIZE*/tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);

		if(ret == 0){
				
//	        reg_addr =	ioremap(INSTR_BASE+SREG0_OFFSET,4);
//	        read_slot_id = readl(reg_addr);
//			printk("Virt-eth0:dma send sucessfully, payoff_length = %d, ret= %d,slot =%d ",tx_frame_info->length,ret,read_slot_id);
//            iounmap(reg_addr);
			if(node_id != MAX_NODE && txq_id%2 == 1 && tx_frame_info->length > 1000)
				veth_priv->v_jgk_info.tx_outall ++;
			
		}
		else{
			veth_priv->v_jgk_info.tx_out_lose ++;
		}
		////////////////////////////////////////
		//amp send tx buf
#ifdef Zynq_Platform
		//tx_pkt_ready.rx_pkt_buf = tx_pkt_buf;
		//tx_pkt_ready.resv = type;
		//tx_pkt_ready.buf_len = tx_frame_info->length;
		//virt_eth_mgmt_send_msg(dev,MGMT_TX_DATA,(u8*)txbuf,tx_frame_info->length + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);



		//memset(txbuf,0,tx_frame_info->length + sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE);

		switch(txq_tmp){
		case MANAGEMENT_QID:{
			veth_priv->v_jgk_info.ping_slot ++;
			veth_priv->v_jgk_info.ping_out_len += tx_pkt_ready.buf_len;
			//printk("Veth CDMA ping packet out, bank id = %d, ping_slot = %d\n",tx_pkt_buf,veth_priv->v_jgk_info.ping_slot);
			break;
		}
		case MCAST_QID:{
			veth_priv->v_jgk_info.ogm_slot ++;
			veth_priv->v_jgk_info.ogm_out_len += tx_pkt_ready.buf_len;
		    //printk("Veth OGM  packet out, bank id = %d, ogm_slot = %d\n",tx_pkt_buf,veth_priv->v_jgk_info.ogm_slot);
			break;
		}
		case MCAST_TCP_QID:{
			veth_priv->v_jgk_info.bcast_slot ++;
			veth_priv->v_jgk_info.bcast_out_len += tx_pkt_ready.buf_len;
			break;
		}
		default:{
			veth_priv->v_jgk_info.ucast_slot ++;
			veth_priv->v_jgk_info.ucast_out_len += tx_pkt_ready.buf_len;
			break;
		}
		}



#endif //for VIRT_ETH_TEST
//		printk("dma send ok mcs_len buf %d %d %d\n",tx_pkt_buf,mcs_len,tx_frame_info->length);
	}
	return 1;

setstate:
err:
	return ret;

}

u16 virt_eth_get_cross_queue_id(struct virt_eth_priv *veth_priv,u16 txq_id, u8 mcs, u8 * cross_queue_list)
{
	u16 txq_id2=0xffff;
	u16 q_len2 = 0;
	int ret=0;
	int i,j;
	u8 station_mcs;
	virt_station_info * vstation = NULL;

	//OGM queue don't participate cross queue packed
	if(txq_id==MCAST_QID ||  txq_id == MCAST_TCP_QID)
	{
		goto get_txqid_over;
	}
	//Management queue and broadcast queue have same mcs
	if(txq_id==MANAGEMENT_QID )
	{
		txq_id2 = MCAST_TCP_QID;
		q_len2 = virt_eth_queue_get_length(veth_priv,txq_id2);
		//don't visit the same queue in the cross queue process
		if( q_len2>0)
		{
//			printk("1,previous queue id %d is empty,get cross queue id = %d,q_len = %d,mcs =2",txq_id,txq_id2,q_len2);
			goto get_txqid_over;
		}
	}
	if(txq_id>MANAGEMENT_QID)
	{
		//if tx queue is unicast, get TCP first, then UCP
		//so if queue is UDP queue, there no need to check the TCP queue again
		if(CHECK_TCP_QUEUE(txq_id))
		{
			txq_id2 = txq_id+1;
			q_len2 = virt_eth_queue_get_length(veth_priv,txq_id2);
			if(txq_id2 != txq_id && q_len2>0)
			{
//				printk("2,previous queue id %d is empty,get cross queue id = %d,q_len = %d",txq_id,txq_id2,q_len2);
				goto get_txqid_over;
			}
		}

	}

	//there is room for packed, get the next node which has same mcs
	//get the packet from the same mcs queue
	rcu_read_lock();
	hlist_for_each_entry_rcu(vstation, &veth_priv->station_list, list)
	{
		if(vstation == NULL)
		{
			break;
		}
		if(CHECK_ID(vstation->id)){
			virt_eth_station_del(vstation);
			continue;
		}
		station_mcs = virt_eth_mgmt_get_mcs(veth_priv,vstation->id);
#ifdef VIRT_ETH_TEST
		if(vstation->state == STATION_ONLINE )
#else
		if(vstation->state == STATION_ONLINE && station_mcs != 0xf)
#endif
		{
			if(vstation->mcs ==mcs)
			{

				//for one node, get packet from tcp queue first,then UDP queue
				//TCP QUEUE
				//check whether this node has been traversed
					for(i=0;i<MAX_NODE;i++)
					{
						if( cross_queue_list[i]== vstation->id)
						{
							break;
						}
					}
					//this node havan't been traversed
					if(i==MAX_NODE)
					{
						//record the node to the traversed list
						for(j=0;j<MAX_NODE;j++)
						{
							if( cross_queue_list[j]== 0)
							{
								cross_queue_list[j] = vstation->id;
								break;
							}
						}
						txq_id2 = STATION_ID_TO_TCP_QUEUE_ID(vstation->id);
						//don't visit the same queue in the cross queue process
						if(txq_id2 == txq_id)
							continue;
						q_len2 = virt_eth_queue_get_length(veth_priv,txq_id2);
						//if TCP queue is empty, then check the UDP queue for this station
						if(q_len2==0)
						{
							txq_id2++;
							//don't visit the same queue in the cross queue process
							if(txq_id2 == txq_id)
								continue;
							q_len2 = virt_eth_queue_get_length(veth_priv,txq_id2);
							//if UDP queue is empty, get to the next node
							if(q_len2==0)
							{
								continue;
							}
						}
//						printk("3,previous queue id %d is empty,get cross queue id = %d,q_len = %d,mcs =%d",txq_id,txq_id2,q_len2,vstation->mcs);
						rcu_read_unlock();
						goto get_txqid_over;
					}
					//this node has been traversed, get the next node
					else
						continue;


			}
		}
	}
	rcu_read_unlock();

	//there is room for packed
	//get the packet from the larger mcs queue
	rcu_read_lock();
	hlist_for_each_entry_rcu(vstation, &veth_priv->station_list, list)
	{
		if(vstation == NULL)
		{
			break;
		}
		if(CHECK_ID(vstation->id)){
			virt_eth_station_del(vstation);
			continue;
		}
		station_mcs = virt_eth_mgmt_get_mcs(veth_priv,vstation->id);
#ifdef VIRT_ETH_TEST
		if(vstation->state == STATION_ONLINE )
#else
		if(vstation->state == STATION_ONLINE && station_mcs != 0xf)
#endif
		{
			if(vstation->mcs > mcs)
			{
				//for one node, get packet from tcp queue first,then UDP queue
				//TCP QUEUE
					//check whether this node has been traversed
					for(i=0;i<MAX_NODE;i++)
					{
						if( cross_queue_list[i]== vstation->id)
						{
							break;
						}
					}
					//this node havan't been traversed
					if(i==MAX_NODE)
					{
						//record the node to the traversed list
						for(j=0;j<MAX_NODE;j++)
						{
							if( cross_queue_list[j]== 0)
							{
								cross_queue_list[j] = vstation->id;
								break;
							}
						}
						txq_id2 = STATION_ID_TO_TCP_QUEUE_ID(vstation->id);
						//don't visit the same queue in the cross queue process
						if(txq_id2 == txq_id)
							continue;
						q_len2 = virt_eth_queue_get_length(veth_priv,txq_id2);
						//if TCP queue is empty, then check the UDP queue for this station
						if(q_len2==0)
						{
							txq_id2++;
							//don't visit the same queue in the cross queue process
							if(txq_id2 == txq_id)
								continue;
							q_len2 = virt_eth_queue_get_length(veth_priv,txq_id2);
							//if UDP queue is empty, get to the next node
							if(q_len2==0)
							{
								continue;
							}
						}
//						printk("4,previous queue id %d is empty,get cross queue id = %d,q_len = %d,mcs =%d",txq_id,txq_id2,q_len2,vstation->mcs);
						rcu_read_unlock();
						goto get_txqid_over;
					}
					//this node has been traversed, get the next node
					else
						continue;

				}

		}
	}
	rcu_read_unlock();
	txq_id2 = 0xffff;
get_txqid_over:
	return txq_id2;
}




#ifdef Docker_Qualnet
void virt_eth_poll_tx_queue(struct net_device * dev){
	virt_station_info * vstation = NULL;
	struct virt_eth_priv *veth_priv = NULL;

	u8 node_id = 0;
	u8 mcs = 0;
#elif defined Zynq_Platform
void virt_eth_poll_tx_queue(struct work_struct *work){
	virt_eth_work_tx_poll    * v_work_tx;
	struct net_device * dev;
	virt_station_info * vstation = NULL;
	struct virt_eth_priv *veth_priv = NULL;
	u8 node_id = 0;
	u8 mcs = 0;

	v_work_tx = container_of(work, virt_eth_work_tx_poll, worker);
	dev = v_work_tx->dev;
#endif

///////////////////////

	//printk("start virt_eth_poll_tx_queue\n");
	veth_priv = netdev_priv(dev);
	//check all queue whether there are packets to send
	//the order is ,1. management queue(ICMP pk),2. multicast/broadcast queue(OGM pk,ect), 3. multicast/broadcast queue(arp,broadcast, ect)
	//4. unicast queue
	if(virt_eth_queue_dequeue_transmit(dev,MAX_NODE,MANAGEMENT_QID,MULTICAST_MCS_DEFAULT)){
		// Found a not-empty queue, transmitted a packet
		goto end;
	}
	else if(virt_eth_queue_dequeue_transmit(dev,MAX_NODE,MCAST_QID,/*MULTICAST_MCS_DEFAULT*/MULTICAST_MCS_DEFAULT)){
		goto end;
		}
	else if(virt_eth_queue_dequeue_transmit(dev,MAX_NODE,MCAST_TCP_QID,MULTICAST_MCS_DEFAULT)){
		//set q_hffull_flag accord to queue length, to decide the number of slot required
		if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 4000)
			veth_priv->virt_traffic_param.q_hffull_flag = 20;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 3000)
			veth_priv->virt_traffic_param.q_hffull_flag = 15;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 2000)
			veth_priv->virt_traffic_param.q_hffull_flag = 10;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 1000)
			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 500)
			veth_priv->virt_traffic_param.q_hffull_flag = 1;
		goto end;
	}
//	printk("station 1\n");

	rcu_read_lock();
	//read all station list by orderly, to check whether there are packets to send
	hlist_for_each_entry_rcu(vstation, &veth_priv->station_list, list) {
		if(vstation == NULL)
		{
			break;
		}
		if(CHECK_ID(vstation->id)){
			virt_eth_station_del(vstation);
			continue;
		}
		if(virt_eth_queue_get_tx_length(veth_priv,vstation->id) > 0 && vstation->state == STATION_ONLINE)
		{
			node_id = vstation->id;
			mcs = vstation->mcs;
			//set q_hffull_flag accord to queue length, to decide the number of slot required
			if(virt_eth_queue_get_tx_length(veth_priv,vstation->id) > 4000)
				veth_priv->virt_traffic_param.q_hffull_flag = 20;
			else if(virt_eth_queue_get_tx_length(veth_priv,vstation->id) > 3000)
				veth_priv->virt_traffic_param.q_hffull_flag = 15;
			else if(virt_eth_queue_get_tx_length(veth_priv,vstation->id) > 2000)
				veth_priv->virt_traffic_param.q_hffull_flag = 10;
			else if(virt_eth_queue_get_tx_length(veth_priv,vstation->id) > 1000)
				veth_priv->virt_traffic_param.q_hffull_flag = 5;
			else if(virt_eth_queue_get_tx_length(veth_priv,vstation->id) > 500)
				veth_priv->virt_traffic_param.q_hffull_flag = 1;

			break;
		}
	}
	rcu_read_unlock();

	if(vstation == NULL){
		goto end;
	}
//	printk("station 2 id %d\n",node_id);

	if(virt_eth_queue_dequeue_transmit(dev,node_id,STATION_ID_TO_TCP_QUEUE_ID(node_id),mcs)){
		goto end;
	}
	else if(virt_eth_queue_dequeue_transmit(dev,node_id,STATION_ID_TO_QUEUE_ID(node_id),mcs)){ //dequeue_transmit_checkin
		goto end;
	}

end:
/////////
//virt_eth_system_rx_data_process(v_work_tx->dev,(u8*)&rx_ready,sizeof(rx_ready));
#ifdef Zynq_Platform
	kfree(v_work_tx);
#endif

	return;
}

u8 virt_eth_queue_tail(struct virt_eth_priv *veth_priv, struct sk_buff * skb,u8 reserved,u8 id)
{
	int mcs;
	unsigned long flags;
	bat_unicast_header_t     * bat_unicast_hdr;
	struct ethhdr            * bat_eth_hdr;
	ipv4_header_t            * ip_hdr;
	tcp_header_t             * tcp_hdr;
	llc_header_t             * llc_hdr;


	

	switch(reserved)
	{
	case MCAST_QID:
	{
		mcs = MULTICAST_MCS_DEFAULT;
		veth_priv->v_jgk_info.tx_qlen[MCAST_QID] = virt_eth_queue_get_length(veth_priv,MCAST_QID);
		if(virt_eth_queue_get_length(veth_priv,MCAST_QID) > 4000)
			veth_priv->virt_traffic_param.q_hffull_flag = 20;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_QID) > 3000)
			veth_priv->virt_traffic_param.q_hffull_flag = 15;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_QID) > 2000)
			veth_priv->virt_traffic_param.q_hffull_flag = 10;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_QID) > 1000)
			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_QID) > 500)
			veth_priv->virt_traffic_param.q_hffull_flag = 1;
		if(virt_eth_queue_get_length(veth_priv,MCAST_QID) > QUEUE_MAX_LEN)
		{
			if(veth_priv->mcs_mode == FIX_MCS_MODE)
				mcs = veth_priv->ucast_mcs;
			veth_priv->virt_traffic_param.pkt_outq_bytes[mcs]+=(((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length-sizeof(llc_header_t)-sizeof(mac_header_cstdd));
			goto over;
		}

		veth_priv->virt_tx_queue[MCAST_QID].total_len += skb->len;
		skb_queue_tail(&veth_priv->virt_tx_queue[MCAST_QID].txq,skb);
//		printk("put a OGM packer in queue %d",MCAST_QID);
		veth_priv->v_jgk_info.tx_in[MCAST_QID] ++;

		break;
	}
	case MCAST_TCP_QID:
	{
		mcs = MULTICAST_MCS_DEFAULT;
		veth_priv->v_jgk_info.tx_qlen[MCAST_TCP_QID] = virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID);
		if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 4000 ) //4000 //QUEUE_MAX_LEN*9/10
			veth_priv->virt_traffic_param.q_hffull_flag = 20;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) >3000)//3000 // QUEUE_MAX_LEN*7/10
			veth_priv->virt_traffic_param.q_hffull_flag = 15;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 2000) //2000 //QUEUE_MAX_LEN*5/10
			veth_priv->virt_traffic_param.q_hffull_flag = 10;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 1000)//1000 //QUEUE_MAX_LEN*3/10
			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		else if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > 500)//500 //QUEUE_MAX_LEN/10
			veth_priv->virt_traffic_param.q_hffull_flag = 1;
		if(virt_eth_queue_get_length(veth_priv,MCAST_TCP_QID) > QUEUE_MAX_LEN)
		{
			if(veth_priv->mcs_mode == FIX_MCS_MODE)
				mcs = veth_priv->ucast_mcs;
			veth_priv->virt_traffic_param.pkt_outq_bytes[mcs]+=(((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length-sizeof(llc_header_t)-sizeof(mac_header_cstdd));
			goto over;
		}
		veth_priv->virt_tx_queue[MCAST_TCP_QID].total_len += skb->len;
		//
		skb_queue_tail(&veth_priv->virt_tx_queue[MCAST_TCP_QID].txq,skb);
		//
		veth_priv->v_jgk_info.tx_in[MCAST_TCP_QID] ++;

		break;
	}
	case MANAGEMENT_QID:
	{
		
		mcs = MULTICAST_MCS_DEFAULT;
		veth_priv->v_jgk_info.tx_qlen[MANAGEMENT_QID] = virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID);
		if(virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID) > 4000)
			veth_priv->virt_traffic_param.q_hffull_flag = 20;
		else if(virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID) > 3000)
			veth_priv->virt_traffic_param.q_hffull_flag = 15;
		else if(virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID) > 2000)
			veth_priv->virt_traffic_param.q_hffull_flag = 10;
		else if(virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID) > 1000)
			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		else if(virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID) > 500)
			veth_priv->virt_traffic_param.q_hffull_flag = 1;
		if(virt_eth_queue_get_length(veth_priv,MANAGEMENT_QID) > QUEUE_MAX_LEN)
		{
			if(veth_priv->mcs_mode == FIX_MCS_MODE)
				mcs = veth_priv->ucast_mcs;
			veth_priv->virt_traffic_param.pkt_outq_bytes[MULTICAST_MCS_DEFAULT]+=(((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length-sizeof(llc_header_t)-sizeof(mac_header_cstdd));
			goto over;
		}

		veth_priv->virt_tx_queue[MANAGEMENT_QID].total_len += skb->len;
		//printk("veth_priv->v_jgk_info.tx_qlen[MANAGEMENT_QID] = %d\n",veth_priv->v_jgk_info.tx_qlen[MANAGEMENT_QID]);
		//printk("veth_priv->virt_tx_queue[MANAGEMENT_QID].total_len = %d\n",veth_priv->virt_tx_queue[MANAGEMENT_QID].total_len);
		//
		skb_queue_tail(&veth_priv->virt_tx_queue[MANAGEMENT_QID].txq,skb);
		//printk("Done enqueue id = %d\n",MANAGEMENT_QID);
		//
		veth_priv->v_jgk_info.tx_in[MANAGEMENT_QID] ++;

//		printk("jgk %d %d\n",v_jgk_info.tx_in[MANAGEMENT_QID],v_jgk_info.tx_qlen[MANAGEMENT_QID]);
		break;
	}
	case IPV4_PROT_TCP:
	{
		if(STATION_ID_TO_TCP_QUEUE_ID(id) >= MAX_QUEUE_NUM)
			goto err;
		veth_priv->v_jgk_info.tx_qlen[STATION_ID_TO_TCP_QUEUE_ID(id)] = virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id));
		if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) >4000) // QUEUE_MAX_LEN*9/10
			veth_priv->virt_traffic_param.q_hffull_flag = 20;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > 3000) //QUEUE_MAX_LEN*7/10
			veth_priv->virt_traffic_param.q_hffull_flag = 15;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > 2000) //QUEUE_MAX_LEN*5/10
			veth_priv->virt_traffic_param.q_hffull_flag = 10;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > 1000)  //QUEUE_MAX_LEN/10
			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > 500) //QUEUE_MAX_LEN/100
			veth_priv->virt_traffic_param.q_hffull_flag = 1;
//		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > ) //QUEUE_MAX_LEN/600
//			veth_priv->virt_traffic_param.q_hffull_flag = 100;
//		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > 0)
//			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_TCP_QUEUE_ID(id)) > QUEUE_MAX_LEN)
		{
			mcs = virt_eth_mgmt_get_mcs(veth_priv,id);
			if(mcs<MCS_NUM)
			veth_priv->virt_traffic_param.pkt_outq_bytes[mcs]+=(((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length-sizeof(llc_header_t)-sizeof(mac_header_cstdd));
			goto over;
		}
		
//		bat_unicast_hdr = (bat_unicast_header_t*)((void*)skb->data +  sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE + 
//				          sizeof(mac_header_cstdd) + sizeof(llc_header_t));
//		llc_hdr = (llc_header_t*)((void*)skb->data +  sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE + 
//				          sizeof(mac_header_cstdd) );
//		if(bat_unicast_hdr->type == BATADV_UNICAST)
//		{
//			bat_eth_hdr  = (struct ethhdr*)((void*)bat_unicast_hdr + sizeof(bat_unicast_header_t));
//			if(bat_eth_hdr->h_proto == ETH_TYPE_IP)
//			{
//				ip_hdr = (ipv4_header_t*)((void*)bat_eth_hdr + sizeof(struct ethhdr));
//				if(ip_hdr->protocol == IPV4_PROT_TCP)
//				{
//					tcp_hdr =  (tcp_header_t*)((void*)ip_hdr + 4*((u8)(ip_hdr->version_ihl) & 0xF));
//
//					if((ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1) ==TARTET_TCP_ACK_NO)
//					{
//						printk("enqueue tcp pk seq %u reserved = %d, queue id = %d llc seqno  = %d\n",
//							(ntohl(tcp_hdr->seq_no)-start_tcp_seq_no+1,reserved,STATION_ID_TO_QUEUE_ID(id),llc_hdr->seqno);
//						taget_tcp_llc_no = llc_hdr->seqno;
//					}
//				}
//			}
//		}


		veth_priv->virt_tx_queue[STATION_ID_TO_TCP_QUEUE_ID(id)].total_len += skb->len;
		skb_queue_tail(&veth_priv->virt_tx_queue[STATION_ID_TO_TCP_QUEUE_ID(id)].txq,skb);
		veth_priv->v_jgk_info.tx_in[STATION_ID_TO_TCP_QUEUE_ID(id)] ++;

		break;
	}
	case IPV4_PROT_UDP:
	{
		if(STATION_ID_TO_QUEUE_ID(id) >= MAX_QUEUE_NUM)
			goto err;
		veth_priv->v_jgk_info.tx_qlen[STATION_ID_TO_QUEUE_ID(id)] = virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id));
		if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id)) > 4000)
			veth_priv->virt_traffic_param.q_hffull_flag = 20;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id)) > 3000)
			veth_priv->virt_traffic_param.q_hffull_flag = 15;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id)) > 2000)
			veth_priv->virt_traffic_param.q_hffull_flag = 10;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id)) > 1000)
			veth_priv->virt_traffic_param.q_hffull_flag = 5;
		else if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id)) > 500)
			veth_priv->virt_traffic_param.q_hffull_flag = 1;
		if(virt_eth_queue_get_length(veth_priv,STATION_ID_TO_QUEUE_ID(id)) > QUEUE_MAX_LEN)
		{
			mcs = virt_eth_mgmt_get_mcs(veth_priv,id);
			if(mcs<MCS_NUM)
			veth_priv->virt_traffic_param.pkt_outq_bytes[mcs]+=( ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length-sizeof(llc_header_t)-sizeof(mac_header_cstdd));
			goto over;
		}
		veth_priv->virt_tx_queue[STATION_ID_TO_QUEUE_ID(id)].total_len += skb->len;
		skb_queue_tail(&veth_priv->virt_tx_queue[STATION_ID_TO_QUEUE_ID(id)].txq,skb);
		veth_priv->v_jgk_info.tx_in[STATION_ID_TO_QUEUE_ID(id)] ++;

		break;
	}
	default:
	{
		goto err;
	}
	}
	veth_priv->v_jgk_info.tx_inall ++;
	return 0;

err:
	veth_priv->v_jgk_info.tx_in_lose ++;
	return 1;
over:
	veth_priv->v_jgk_info.tx_in_lose ++;
	return 1;

}

u8 virt_eth_queue_get_rx_length(struct virt_eth_priv *veth_priv, u8 nodeid){
	if(nodeid >= MAX_QUEUE_NUM)
		return 0;
	return veth_priv->virt_rx_queue[nodeid].txq.qlen;
}

u8 virt_eth_queue_remove_rx_first(struct virt_eth_priv *veth_priv,u8 nodeid){
	struct sk_buff* skb = NULL;
	unsigned long flags;
	spin_lock_irqsave(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);
	skb = __skb_dequeue(&veth_priv->virt_rx_queue[nodeid].txq);
	if(skb)
		dev_kfree_skb (skb);
	spin_unlock_irqrestore(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);

	return 0;
}

struct sk_buff* virt_eth_queue_find_by_id(struct virt_eth_priv *veth_priv, u8 nodeid,u16 seqno){
	struct sk_buff* skb,*tmp = NULL;
	unsigned long flags;
	tx_queue_buffer_t *tx_queue_info;
	if(nodeid >= MAX_QUEUE_NUM)
		return tmp;
	spin_lock_irqsave(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);
	skb_queue_walk(&veth_priv->virt_rx_queue[nodeid].txq,skb){
		if(skb == NULL)
		{
			printk("error skb\n");
			continue;
		}
		tx_queue_info = (tx_queue_buffer_t*)skb->data;
		if(tx_queue_info->seqno == seqno)
		{
			tmp = skb;
			break;
		}
	}
	spin_unlock_irqrestore(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);

	return tmp;
}

void virt_eth_queue_del_by_id(struct virt_eth_priv *veth_priv,u8 nodeid,u16 seqno){
	struct sk_buff *skb,*tmp = NULL;

	unsigned long flags;
	tx_queue_buffer_t *tx_queue_info;
	spin_lock_irqsave(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);
	skb_queue_walk_safe(&veth_priv->virt_rx_queue[nodeid].txq,skb,tmp){
		tx_queue_info = (tx_queue_buffer_t*)skb->data;
		if(tx_queue_info->seqno == seqno)
		{
			__skb_unlink(skb,&veth_priv->virt_rx_queue[nodeid].txq);
			dev_kfree_skb(skb);
			spin_unlock_irqrestore(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);
			return;
		}
	}
	spin_unlock_irqrestore(&veth_priv->virt_rx_queue[nodeid].txq.lock, flags);
//	if(tmp){
//		skb_unlink(tmp,&veth_priv->virt_rx_queue[nodeid].txq);
//		dev_kfree_skb(tmp);
//	}

}

void virt_eth_add_rx_queue(struct virt_eth_priv *veth_priv, u8 nodeid,struct sk_buff* skb){
	skb_queue_tail(&veth_priv->virt_rx_queue[nodeid].txq,skb);
}


void virt_eth_rx_queue_timeout(struct work_struct *work)
{
	struct sk_buff *skb,*tmp = NULL;
	struct virt_eth_priv *veth_priv;
	struct delayed_work *delayed_work;
	unsigned long flags;
	tx_queue_buffer_t *tx_queue_info;
	u16 i = 0;

	delayed_work = to_delayed_work(work);
	veth_priv = container_of(delayed_work, struct virt_eth_priv,
			queue_work);

	for(i = 0; i < MAX_QUEUE_NUM; i ++){
		spin_lock_irqsave(&veth_priv->virt_rx_queue[i].txq.lock, flags);
		skb_queue_walk_safe(&veth_priv->virt_rx_queue[i].txq,skb,tmp){
			tx_queue_info = (tx_queue_buffer_t*)skb->data;
			if(time_is_before_jiffies(tx_queue_info->tx_frame_info.timestamp_create + msecs_to_jiffies(5000)))
			{
				__skb_unlink(skb,&veth_priv->virt_rx_queue[i].txq);
				dev_kfree_skb(skb);
			}
		}
		spin_unlock_irqrestore(&veth_priv->virt_rx_queue[i].txq.lock, flags);

	}
	virt_eth_rx_queue_schedule(veth_priv);
}

void virt_eth_rx_queue_schedule(struct virt_eth_priv *veth_priv){
	unsigned long time_delay = msecs_to_jiffies(QUEUE_SCHEDULE);

	queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			   &veth_priv->queue_work,
			   time_delay);
}

int virt_eth_queue_get_larger_mcs_txqueue_total_length(struct net_device *dev, u8 mcs)
{
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	int i;
	int total_length=0;
	virt_station_info * vstation = NULL;

	rcu_read_lock();
	hlist_for_each_entry_rcu(vstation, &veth_priv->station_list, list)
	{
		if(vstation == NULL)
		{
			break;
		}
		if(CHECK_ID(vstation->id))
		{
			virt_eth_station_del(vstation);
			continue;
		}
		if(vstation->state == STATION_ONLINE)
		{
			if(vstation->mcs >= mcs)
			{
				total_length +=veth_priv->virt_tx_queue[STATION_ID_TO_TCP_QUEUE_ID(vstation->id)].total_len;
				total_length +=veth_priv->virt_tx_queue[STATION_ID_TO_QUEUE_ID(vstation->id)].total_len;
			}
		}
	}
	rcu_read_unlock();

	return total_length;
}




//struct sk_buff* virt_eth_queue_get_skb(u8 id){
//
//	return skb_dequeue(&virt_queue[id].txq);
//}
