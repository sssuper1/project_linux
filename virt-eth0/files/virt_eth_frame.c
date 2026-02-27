/*
 * virt_eth_frame.c
 *
 *  Created on: 2020-4-14
 *      Author: lu
 */
#include "virt_eth_types.h"
#include "virt_eth_frame.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_queue.h"
#include "virt_eth_util.h"

#define DEL_DATA_LEN                                       (sizeof(mac_header_cstdd))

typedef struct{
#ifdef Docker_Qualnet
	ethernet_header_t   tx_ethhdr;
#endif
	mac_header_cstdd    rx_80211;
	llc_header_t      llc_c;
} movedata;

//Sta_Ampdu_Stat   Aq_info[MAX_STA_NUM];



u8 virt_eth_frame_aggre_ampdu_transmit(struct virt_eth_priv *veth_priv,bool first,u32 mcs_len,struct sk_buff* skb,u8* daddr,u8 mcs,u8 node_mcs){
	void* dest_addr;
	void* src_addr;
	u32   xfer_len;
	u8  ungroup_flag = Fragment_Initial;
	movedata          movedata_c;
	llc_header_t     * llc_hdr;
	mac_header_cstdd * rx_80211;
	tx_frame_info_t* 	tx_frame_info;

	bat_unicast_header_t     * bat_unicast_hdr;
	struct ethhdr            * bat_eth_hdr;
	ipv4_header_t            * ip_hdr;
	tcp_header_t             * tcp_hdr;


	u8* data = NULL;

	if(skb == NULL)
		return 0;
	data = daddr;

	tx_frame_info   = (tx_frame_info_t*)data;


	dest_addr = (void*)daddr;
//	printk("transmit 1  0x%p\n",(void*)daddr);




	//check if the fragment is first one
	if(first)
	{
		src_addr  = (void*)&(((tx_queue_buffer_t*)skb->data)->tx_frame_info);
#ifdef Docker_Qualnet
		xfer_len = sizeof(tx_frame_info_t);
#elif defined Zynq_Platform
		xfer_len = sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE;
#endif
		//copy the tx_frame_info and remaining PHY_TX_PKT_BUF_PHY_HDR_SIZE for mac
		memcpy(dest_addr,src_addr,xfer_len);
		dest_addr += xfer_len;

//		tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_l_u;

		//src_addr = (void*)(data2+xfer_len);
		//dest_addr = (void*)(data+xfer_len);

		src_addr = (u8*)(((tx_queue_buffer_t*)(skb->data))->frame)+((tx_queue_buffer_t*)(skb->data))->tx_offset;
		xfer_len  = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length - (((tx_queue_buffer_t*)(skb->data))->tx_offset);

		tx_frame_info->length = 0;


#ifdef Docker_Qualnet
		llc_hdr = (llc_header_t*)(src_addr+sizeof(ethernet_header_t)+DEL_DATA_LEN);
		llc_hdr->length -=sizeof(ethernet_header_t); //the receiver's skb->data will point to the payoff, don't contain the ethernet header
#elif defined Zynq_Platform
		llc_hdr = (llc_header_t*)(src_addr+DEL_DATA_LEN);
#endif

//		printk("1 llc_length %d, xfer_len %d,mcs_len %d, (skb->data))->tx_frame_info.length %d,(skb->data))->tx_offset %d,llc_hdr->fragment_offset %d \n"\
//				,llc_hdr->length,xfer_len,mcs_len,((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length,((tx_queue_buffer_t*)(skb->data))->tx_offset,ntohs(llc_hdr->fragment_offset));


		//the packet has been split and the length of the packet is smaller than the number of bytes sent in one slot
		//
		if((llc_hdr->fragment_offset & htons(IP_DF)) && (xfer_len <= mcs_len ))
		{
			llc_hdr->fragment_offset = htons(0);
			llc_hdr->fragment_offset |= htons(((tx_queue_buffer_t*)skb->data)->tx_offset + sizeof(movedata)); //this fragment offset
			llc_hdr->fragment_offset |= htons(IP_DF);  //the fragment flag is set ture
			llc_hdr->fragment_offset |= htons(IP_LF); //this is last fragment flag

		}
		//the packet should be split if the length of the packet is larger than the number of bytes sent in one slot
		else if(xfer_len > mcs_len )
		{

			//the length of the packer sent in one slot
			xfer_len = mcs_len ;
#ifdef Docker_Qualnet
			//set the llc_hdr->length to the length of current fragment excerpt the length of ethernet header, not the whole length of packet
			llc_hdr->length = (u16)xfer_len - sizeof(ethernet_header_t);
#elif defined Zynq_Platform
			llc_hdr->length = (u16)xfer_len ;
#endif
			if(llc_hdr->fragment_offset & htons(IP_DF))
			{
				llc_hdr->fragment_offset = htons(((tx_queue_buffer_t*)(skb->data))->tx_offset + sizeof(movedata));
			}
			else if(llc_hdr->fragment_offset == 0)
				llc_hdr->fragment_offset = htons(((tx_queue_buffer_t*)(skb->data))->tx_offset);
			else
			{
				ungroup_flag = Fragment_Error;
//					tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_HIGH_CTRL;
//				xil_printf("ungroup_flag1 = 3\n");
//				daddr = dest_addr;
				return ungroup_flag;
			}
			llc_hdr->fragment_offset |= htons(IP_DF); //the fragment flag is set ture
			llc_hdr->fragment_offset &= htons(~IP_LF); //this is not the last fragment flag

			memcpy(&movedata_c,src_addr,sizeof(movedata));
#ifdef Docker_Qualnet
			movedata_c.llc_c.length = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length - ((tx_queue_buffer_t*)(skb->data))->tx_offset \
					- xfer_len + sizeof(ethernet_header_t) + DEL_DATA_LEN + sizeof(llc_header_t);
#elif defined Zynq_Platform
			movedata_c.llc_c.length = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length - ((tx_queue_buffer_t*)(skb->data))->tx_offset \
					- xfer_len + DEL_DATA_LEN + sizeof(llc_header_t) ;
#endif
			((tx_queue_buffer_t*)(skb->data))->tx_offset += xfer_len - sizeof(movedata);
			ungroup_flag = Original_PK_Remaining; //the packet should insert back to the tx queue, waiting for the next transmition
		}
		else
		{
			llc_hdr->fragment_offset = htons(0);
		}


		tx_frame_info->length += xfer_len ;
		if(node_mcs<8)
		veth_priv->virt_traffic_param.pkt_outq_bytes[node_mcs]+=(xfer_len-sizeof(movedata));
//		printk("2 first, llc_length %d, tx_frame_info->length %d, (skb->data))->tx_offset %d, llc_hdr->fragment_offset %d\n",\
//				llc_hdr->length,tx_frame_info->length,((tx_queue_buffer_t*)(skb->data))->tx_offset,ntohs(llc_hdr->fragment_offset));

	}
	//fragment is not first
	//get a new packet from queue, pack the new packet in the fragment
	else
	{

//			if(tx_frame_info->length == 1280)
//				xil_printf("error data\n");
#ifdef Docker_Qualnet
		//go to end of last packet
		dest_addr += sizeof(tx_frame_info_t)  + tx_frame_info->length;
		//do not use ethernet header and mac_header_cstdd after the first packet
		//the src_addr is set to the header of llc
		src_addr  = (u8*)(((tx_queue_buffer_t*)(skb->data))->frame)+((tx_queue_buffer_t*)(skb->data))->tx_offset + sizeof(ethernet_header_t) + DEL_DATA_LEN;
		//the length of packet is llc_header + payoff
		xfer_len  = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length - sizeof(ethernet_header_t) -((tx_queue_buffer_t*)(skb->data))->tx_offset - DEL_DATA_LEN;
#elif defined Zynq_Platform
		//go to end of last packet
		dest_addr += sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE + tx_frame_info->length;
		//do not use mac_header_cstdd after the first packet
		src_addr  = (u8*)(((tx_queue_buffer_t*)(skb->data))->frame)+((tx_queue_buffer_t*)(skb->data))->tx_offset + DEL_DATA_LEN;
		//the length of packet is llc_header + payoff - tx_offset
		xfer_len  = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length - (((tx_queue_buffer_t*)(skb->data))->tx_offset) - DEL_DATA_LEN;
#endif
		//		if(((llc_header_t*)(data+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind == CONDITION_TURE && ((tx_queue_buffer_t*)(skb->data))->tx_offset !=0 )
//			printk("cross queue: xfer_len = %d, mcs_len = %d,tx_frame_info->length =%d, tx_offset = %d,llc_hdr->fragment_offset = %d\n",\
//					xfer_len,mcs_len,tx_frame_info->length,(((tx_queue_buffer_t*)(skb->data))->tx_offset),llc_hdr->fragment_offset);

		//check if there is enough room for packing packet
		//the length if llc_header and payoff is larger than the left room
		//meanwhile the left room is larger than the the header of llc, which means that
		//we can send some payoff in this slot

		if(xfer_len > (mcs_len - tx_frame_info->length) && (mcs_len - tx_frame_info->length) > MINFREAMLEN)
		{

			llc_hdr = (llc_header_t*)(src_addr);
			//get the length of current sent fragment
			xfer_len = mcs_len - tx_frame_info->length;
			//set the llc_hdr->length to the length of current fragment(contain the ethernet header and mac_header_cstdd),
			//not the whole length of packet
			if(llc_hdr->fragment_offset & htons(IP_DF))
			{
//				return Original_PK_Remaining;
				llc_hdr->fragment_offset = htons(((tx_queue_buffer_t*)(skb->data))->tx_offset + sizeof(movedata));
			}
			else if(llc_hdr->fragment_offset == 0){
//				return Original_PK_Remaining;
				llc_hdr->fragment_offset = htons(((tx_queue_buffer_t*)(skb->data))->tx_offset); //=0?
			}
			else
			{
				ungroup_flag = Fragment_Error;
//					tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_HIGH_CTRL;
//				xil_printf("ungroup_flag1 = 3\n");
//				daddr = dest_addr;
				return ungroup_flag;
			}
			llc_hdr->length = (u16)(xfer_len +  DEL_DATA_LEN ); //LLC_length should contain the  length of both MAC and llc header
			llc_hdr->fragment_offset |= htons(IP_DF);
			llc_hdr->fragment_offset &= htons(~IP_LF);
#ifdef Docker_Qualnet
			//copy the information of ethternet header, mac_header_cstdd and LLC header
			memcpy((u8*)&movedata_c,src_addr - DEL_DATA_LEN -sizeof(ethernet_header_t),sizeof(movedata));
			//the length of rest payoff and llc header
			movedata_c.llc_c.length = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length -\
					(((tx_queue_buffer_t*)(skb->data))->tx_offset)- xfer_len + sizeof(llc_header_t);
			//set the offset to the llc_header of next fragment
			((tx_queue_buffer_t*)(skb->data))->tx_offset += xfer_len - sizeof(movedata) + DEL_DATA_LEN + sizeof(ethernet_header_t);
#elif defined Zynq_Platform
			memcpy((u8*)&movedata_c,src_addr - DEL_DATA_LEN,sizeof(movedata));

			//the length of rest payoff and llc header
			movedata_c.llc_c.length = ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length -\
					(((tx_queue_buffer_t*)(skb->data))->tx_offset)- xfer_len + sizeof(llc_header_t);//-sizeof(mac_header_cstdd)
			((tx_queue_buffer_t*)(skb->data))->tx_offset += xfer_len - sizeof(movedata) + DEL_DATA_LEN;
#endif


			ungroup_flag = Original_PK_Remaining; //the packet should insert back to the tx queue, waiting for the next transmition
//			if(llc_hdr->length == 1076)
//					xil_printf("movedata_c.llc_c.length = %d   %d   %d\n",movedata_c.llc_c.length,((tx_queue_buffer_t*)(packet->data))->tx_offset,llc_hdr->length);

		}
		// there is not enough room to pack a more packet
		// this situation should not happen, due to last time has checked this situation
		else if((mcs_len - tx_frame_info->length) <= MINFREAMLEN)
		{
			ungroup_flag = Fragment_Error;
//				tx_frame_info->tx_pkt_buf_state = TX_PKT_BUF_HIGH_CTRL;
//				xil_printf("ungroup_flag2 = 3 %d %d\n",tx_frame_info->length,xfer_len);
//			daddr = dest_addr;
			return ungroup_flag;
		}
		//the left room can pack a whole packet in this fragment
		else
		{
//			return Original_PK_Remaining;
//			rx_80211 = (mac_header_cstdd*)(daddr + sizeof(tx_frame_info_t) + PHY_TX_PKT_BUF_PHY_HDR_SIZE);

			llc_hdr = (llc_header_t*)(src_addr);

//			if(rx_80211->dest_id != llc_hdr->dest_nodeId)
//				printk("11 %d %d\n",rx_80211->dest_id,llc_hdr->dest_nodeId);

			if((llc_hdr->fragment_offset & htons(IP_DF)))
			{
				llc_hdr->fragment_offset = htons(0);
				llc_hdr->fragment_offset |= htons(((tx_queue_buffer_t*)skb->data)->tx_offset + sizeof(movedata)); //this fragment offset
				llc_hdr->fragment_offset |= htons(IP_DF);  //the fragment flag is set ture
				llc_hdr->fragment_offset |= htons(IP_LF); //this is last fragment flag

			}
			else
				llc_hdr->fragment_offset = htons(0);
#ifdef Docker_Qualnet
			//set the llc_hdr->length to the length of current fragment excerpt the length of ethernet header, not the whole length of packet
			llc_hdr->length -= sizeof(ethernet_header_t);
#endif

		}
		 tx_frame_info->length += xfer_len;
		 if(node_mcs<8)
		 veth_priv->virt_traffic_param.pkt_outq_bytes[node_mcs]+=(xfer_len-sizeof(llc_header_t));
//		 if(((llc_header_t*)(data+sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE+sizeof(mac_header_cstdd)))->corss_q_ind == CONDITION_TURE)
//			printk("cross queue,the first after,xfer_len %d,mcs_len %d, tx_frame_info->length %d,((tx_queue_buffer_t*)(skb->data))->tx_offset %d, ((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length %d,node_mcs=%d\n",\
//					xfer_len,mcs_len,tx_frame_info->length,((tx_queue_buffer_t*)(skb->data))->tx_offset,((tx_queue_buffer_t*)(skb->data))->tx_frame_info.length,node_mcs);


	}//end for if(first) and else



//	//printk("frame llc_hdr->length %d %d 0x%p\n",llc_hdr->length,xfer_len,dest_addr);

		//cdma

	//copy the send date
	memcpy(dest_addr,src_addr,xfer_len);

	//dest_addr += xfer_len;

	//if do fragment, copy the header information for next fragment
	if(ungroup_flag)

	{
			memcpy(src_addr+xfer_len - sizeof(movedata),&movedata_c,sizeof(movedata));
	}
	if(first)
	{

		tx_frame_info->params.phy.mcs = mcs;

	}

	if((mcs_len - tx_frame_info->length) <= MINFREAMLEN)

	{

		if(!ungroup_flag)

			ungroup_flag = Fragment_Finish;

	}

	//daddr = dest_addr;
//		//printk("transmit 2  0x%p\n",(void*)daddr);
	//ungroup_flag = Fragment_Finish;


	return ungroup_flag;
}


