/*
 * virt_eth_comm_board.c
 *
 *  Created on: 2020-5-15
 *      Author: lu
 */
#include <linux/jiffies.h>

#include "virt_eth_types.h"
#include "virt_eth_comm_board.h"
#include "virt_eth_util.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_queue.h"
#include "virt_eth_jgk.h"

#define INFO_REPORT_TIME (10*1000)
#define BATADV_COMM 0x09

void virt_eth_comm_create_frame(struct work_struct *work){
		u8 data[1024];
		int len = 1024;
		enum CTL_TYPE type;
		struct net_device *dev;
		u8 dst_mac[ETH_ALEN]   = {0x01,0x01,0x01,0x01,0x01,0x01};
		u8 comm_addr[ETH_ALEN] = {0x00,0x00,0xc0,0xa8,0x01,0x00};
		struct sk_buff *skb;
		unsigned char *skb_buff;
		unsigned int skb_size;
		struct ethhdr *ethh;
		u8 *pdata;
		u32 i = 0;
		struct batadv_comm_msg *comm_msg;
		struct virt_eth_priv *veth_priv;
		virt_eth_comm_work* work_slot_num;
		struct delayed_work *delayed_work;

		delayed_work = to_delayed_work(work);
		work_slot_num = container_of(delayed_work, virt_eth_comm_work,
						   delayed_work);

		type = work_slot_num->type;
		dev = work_slot_num->dev;

		veth_priv = netdev_priv(work_slot_num->dev);
//		printk("virt_eth_comm_create_frame type %d\n",type);

		memset(data,0,len);
		switch(type)
		{
		case A_CHANNEL_CONFIG_INFO:
		case G_CHANNEL_CONFIG_INFO:
			{
				ethh = (struct ethhdr*)data;
				memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
				memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
				ethh->h_proto = ETH_TYPE_BAT;
				comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
				comm_msg->packet_type = BATADV_COMM;
				comm_msg->version = BATADV_VERSION;
				comm_msg->ttl = 50;
				comm_msg->flags = 0;
				comm_msg->len = htonl(ETH_HLEN + 12);

				ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
				memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
				memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
				ethh->h_proto = htons(ETH_P_CTL);

				struct channel_config_info frame;
				frame.packet_type = type;
				frame.packet_len = htons(22);
				frame.channel_bw = htonl(jgk_comm_board_parameter_data.bandwidth);
				frame.channel_freq = htonl(jgk_comm_board_parameter_data.frequency);
				frame.channel_rate = htonl(jgk_comm_board_parameter_data.channel_rate);
				frame.channel_type = jgk_comm_board_parameter_data.channel_type;
				frame.device_mtu = htons(jgk_comm_board_parameter_data.device_mtu);
				frame.mode = jgk_comm_board_parameter_data.mode;

				pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
				*pdata = frame.packet_type;
				pdata ++;
				memcpy(pdata,&frame.packet_len,2);
				pdata += 2;
				memcpy(pdata,&(jgk_comm_board_parameter_data.device_mac),ETH_ALEN);
				pdata += ETH_ALEN;
				*pdata =frame.channel_type;
				pdata += 1;
				memcpy(pdata,&frame.channel_rate,4);
				pdata += 4;
				memcpy(pdata,&frame.device_mtu,2);
				pdata += 2;
				memcpy(pdata,&frame.channel_freq,4);
				pdata += 4;
				memcpy(pdata,&frame.channel_bw,4);
				pdata += 4;
				*pdata =frame.mode;

				skb_size = 25 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
				skb = dev_alloc_skb(skb_size);
				if(!skb)
				{
					//printk("skb error!\n");
					goto re;
				}

				skb_buff = skb_put(skb, skb_size);
				memcpy(skb_buff, data, skb_size);

				skb->dev = dev;
				skb->protocol = eth_type_trans(skb, dev);
				skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

				netif_rx_ni(skb);
				//printk("virt_eth_comm_create_frame A_CHANNEL_CONFIG_INFO\n");
				//printk("channel_type= %d,device_mtu = %d,channel_freq = %d,channel_bw = %d, mac = %d\n",
				//	jgk_comm_board_parameter_data.channel_type,jgk_comm_board_parameter_data.device_mtu,jgk_comm_board_parameter_data.frequency,
				//	jgk_comm_board_parameter_data.bandwidth,jgk_comm_board_parameter_data.device_mac[5]);
				break;
			}
		case A_CHANNEL_QUALITY_INFO:
		{
			ethh = (struct ethhdr*)data;
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
			ethh->h_proto = ETH_TYPE_BAT;
			comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
			comm_msg->packet_type = BATADV_COMM;
			comm_msg->version = BATADV_VERSION;
			comm_msg->ttl = 50;
			comm_msg->flags = 0;
			comm_msg->len = htonl(ETH_HLEN + 12);

			ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
			ethh->h_proto = htons(ETH_P_CTL);

			struct channel_quality_info frame;
			frame.packet_type = A_CHANNEL_QUALITY_INFO;
			frame.node_num = htons(2);

			for(i = 0; i < MAX_NODE;i ++){
				if(virt_eth_mgmt_get_mcs(veth_priv,i) != 0x0f)
					break;
			}
			comm_addr[5] = i;
			memcpy(frame.node_name,comm_addr,ETH_ALEN);

			//check channel state 
			for (i=0;i<NET_SIZE;i++)
			{
				if (jgk_information_data.mac_information_part1.nbr_list[i] != 0x02)
				{
					break;
				}
				
			}
			if (i<NET_SIZE)
			{
				// chennel information is good
				frame.node_quality = 1;
			}
			else if (i==NET_SIZE)
			{
				// chennel information is bad
				frame.node_quality = 0;
			}
			//for (i = 0; i < MAX_NODE;i ++)
			//{
			//	
			//	if (veth_priv->virt_traffic_param.mcs_value[i] != 0x0f)
			//	{
			//		break;
			//	}				
			//}
			//if (i<MAX_NODE)
			//{
			//	// chennel information is good
			//	frame.node_quality = 1;
			//}
			//else if (i==MAX_NODE)
			//{
			//	// chennel information is bad
			//	frame.node_quality = 0;
			//}

			
			frame.packet_len = htons(16);

			pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
			*pdata = frame.packet_type;
			pdata ++;
			memcpy(pdata,&frame.packet_len,2);
			pdata += 2;
			memcpy(pdata,&frame.node_num,2);
			pdata += 2;
			memcpy(pdata,frame.node_name,ETH_ALEN);
			pdata += ETH_ALEN;
			*pdata = frame.node_quality;

			skb_size = 19 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
			skb = dev_alloc_skb(skb_size);
			if(!skb)
			{
				//printk("skb error!\n");
				goto re;
			}

			skb_buff = skb_put(skb, skb_size);
			memcpy(skb_buff, data, skb_size);

			skb->dev = dev;
			skb->protocol = eth_type_trans(skb, dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

			netif_rx_ni(skb);
	//printk("virt_eth_comm_create_frame A_CHANNEL_QUALITY_INFO\n");
			break;
		}
		case R_QOS_QUEUE_OCCUPY:
		{
			for(i = 0; i < 256; i ++){
				if(virt_eth_queue_get_length(veth_priv,i) > QUEUE_MAX_LEN * 2/ 3)
				{
					break;
				}
			}

			if(i < 256){
				ethh = (struct ethhdr*)data;
				memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
				memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
				ethh->h_proto = ETH_TYPE_BAT;
				comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
				comm_msg->packet_type = BATADV_COMM;
				comm_msg->version = BATADV_VERSION;
				comm_msg->ttl = 50;
				comm_msg->flags = 0;
				comm_msg->len = htonl(ETH_HLEN + 9);

				ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
				memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
				memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
				ethh->h_proto = htons(ETH_P_CTL);

				struct qos_queue_occupy frame;
				struct report_no_msg rnm;
				rnm.packet_type = R_QOS_QUEUE_OCCUPY;
				rnm.packet_len = htons(6);
				//frame.packet_type = R_QOS_QUEUE_OCCUPY;
				frame.queue1_occupy = htons(75);
				frame.queue2_occupy = htons(75);
				frame.queue3_occupy = htons(75);
				//frame.packet_len = 6;

				pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
				*pdata = rnm.packet_type;
				pdata ++;
				memcpy(pdata,&rnm.packet_len,2);
				pdata += 2;
				memcpy(pdata,&frame,sizeof(frame));

				skb_size = 9 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
				skb = dev_alloc_skb(skb_size);
				if(!skb)
				{
					//printk("skb error!\n");
					virt_eth_comm_queue_warning(dev);
					goto re;
				}

				skb_buff = skb_put(skb, skb_size);
				memcpy(skb_buff, data, skb_size);

				skb->dev = dev;
				skb->protocol = eth_type_trans(skb, dev);
				skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

				netif_rx_ni(skb);
			}
			virt_eth_comm_queue_warning(dev);
	//printk("bat R_QOS_QUEUE_OCCUPY\n");
			break;
		}
		case A_QOS_QUEUE_INFO:
		{
			ethh = (struct ethhdr*)data;
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
			ethh->h_proto = ETH_TYPE_BAT;
			comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
			comm_msg->packet_type = BATADV_COMM;
			comm_msg->version = BATADV_VERSION;
			comm_msg->ttl = 50;
			comm_msg->flags = 0;
			comm_msg->len = htonl(ETH_HLEN + 9);

			ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
			ethh->h_proto = htons(ETH_P_CTL);

			struct qos_queue_info frame;
			struct report_no_msg rnm;
			rnm.packet_type = A_QOS_QUEUE_INFO;
			frame.queue1_occupy = htons(75);
			frame.queue2_occupy = htons(75);
			frame.queue3_occupy = htons(75);
			rnm.packet_len = htons(6);

			pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
			*pdata = rnm.packet_type;
			pdata ++;
			memcpy(pdata,&rnm.packet_len,2);
			pdata += 2;
			memcpy(pdata,&frame,sizeof(frame));

			skb_size = 9 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
			skb = dev_alloc_skb(skb_size);
			if(!skb)
			{
				//printk("skb error!\n");
				goto re;
			}

			skb_buff = skb_put(skb, skb_size);
			memcpy(skb_buff, data, skb_size);

			skb->dev = dev;
			skb->protocol = eth_type_trans(skb, dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

			netif_rx_ni(skb);
	//printk("bat A_QOS_QUEUE_INFO\n");
			break;
		}
		case A_CHANNEL_LDLE_RETIO_MSG:
		{
			ethh = (struct ethhdr*)data;
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
			ethh->h_proto = ETH_TYPE_BAT;
			comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
			comm_msg->packet_type = BATADV_COMM;
			comm_msg->version = BATADV_VERSION;
			comm_msg->ttl = 50;
			comm_msg->flags = 0;
			comm_msg->len = htonl(ETH_HLEN + 5);

			ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
			ethh->h_proto = htons(ETH_P_CTL);

			struct channel_ldle_retio_msg frame;
			struct report_no_msg rnm;
			rnm.packet_type = A_CHANNEL_LDLE_RETIO_MSG;
			frame.channel_ldle_retio = htons(50);
			rnm.packet_len = htons(2);

			pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
			*pdata = rnm.packet_type;
			pdata ++;
			memcpy(pdata,&rnm.packet_len,2);
			pdata += 2;
			memcpy(pdata,&frame,sizeof(frame));

			skb_size = 5 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
			skb = dev_alloc_skb(skb_size);
			if(!skb)
			{
				//printk("skb error!\n");
				goto re;
			}

			skb_buff = skb_put(skb, skb_size);
			memcpy(skb_buff, data, skb_size);

			skb->dev = dev;
			skb->protocol = eth_type_trans(skb, dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

			netif_rx_ni(skb);
	//printk("bat A_CHANNEL_LDLE_RETIO_MSG\n");
			break;
		}
		case A_PRIORITY_R_S_PKG_NUM:
		{
			ethh = (struct ethhdr*)data;
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
			ethh->h_proto = ETH_TYPE_BAT;
			comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
			comm_msg->packet_type = BATADV_COMM;
			comm_msg->version = BATADV_VERSION;
			comm_msg->ttl = 50;
			comm_msg->flags = 0;
			comm_msg->len = htonl(ETH_HLEN + 27);

			ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
			ethh->h_proto = htons(ETH_P_CTL);


			struct priority_r_s_pkg_num frame;
			struct report_no_msg rnm;
			rnm.packet_type = A_PRIORITY_R_S_PKG_NUM;
			frame.queue1_r_num = htonl(50);
			frame.queue1_s_num = htonl(50);
			frame.queue2_r_num = htonl(50);
			frame.queue2_s_num = htonl(50);
			frame.queue3_r_num = htonl(50);
			frame.queue3_s_num = htonl(50);

			rnm.packet_len = htons(24);

			pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
			*pdata = rnm.packet_type;
			pdata ++;
			memcpy(pdata,&rnm.packet_len,2);
			pdata += 2;
			memcpy(pdata,&frame,sizeof(frame));

			skb_size = 27 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
			skb = dev_alloc_skb(skb_size);
			if(!skb)
			{
				//printk("skb error!\n");
				goto re;
			}
			skb_buff = skb_put(skb, skb_size);
			memcpy(skb_buff, data, skb_size);

			skb->dev = dev;
			skb->protocol = eth_type_trans(skb, dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

			netif_rx_ni(skb);
	//printk("bat A_PRIORITY_R_S_PKG_NUM\n");
			break;
		}
		case A_PRIORITY_OVERFLOW_PKG_NUM:
		{
			ethh = (struct ethhdr*)data;
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dev->dev_addr,ETH_ALEN);
			ethh->h_proto = ETH_TYPE_BAT;
			comm_msg = (struct batadv_comm_msg*)(data + ETH_HLEN);
			comm_msg->packet_type = BATADV_COMM;
			comm_msg->version = BATADV_VERSION;
			comm_msg->ttl = 50;
			comm_msg->flags = 0;
			comm_msg->len = htonl(ETH_HLEN + 15);

			ethh = (struct ethhdr*)(data + sizeof(struct batadv_comm_msg) + ETH_HLEN);
			memcpy(ethh->h_source,dev->dev_addr,ETH_ALEN);
			memcpy(ethh->h_dest,dst_mac,ETH_ALEN);
			ethh->h_proto = htons(ETH_P_CTL);

			struct priority_overflow_pkg_num frame;
			struct report_no_msg rnm;
			rnm.packet_type = A_PRIORITY_OVERFLOW_PKG_NUM;
			frame.queue1_overflow_num = htonl(0);
			frame.queue2_overflow_num = htonl(0);
			frame.queue3_overflow_num = htonl(0);
			rnm.packet_len = htons(12);

			pdata = (u8*)((u8*)ethh + sizeof(struct ethhdr));
			*pdata = rnm.packet_type;
			pdata ++;
			memcpy(pdata,&rnm.packet_len,2);
			pdata += 2;
			memcpy(pdata,&frame,sizeof(frame));

			skb_size = 15 + ETH_HLEN + ETH_HLEN + sizeof(struct batadv_comm_msg);
			skb = dev_alloc_skb(skb_size);
			if(!skb)
			{
				//printk("skb error!\n");
				goto re;
			}

			skb_buff = skb_put(skb, skb_size);
			memcpy(skb_buff, data, skb_size);

			skb->dev = dev;
			skb->protocol = eth_type_trans(skb, dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */

			netif_rx_ni(skb);
	//printk("bat A_PRIORITY_OVERFLOW_PKG_NUM\n");
			break;
		}
		default:
			break;
		}

re:
	kfree(work_slot_num);
	return;
}

u8 virt_eth_comm_queue_warning(struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	unsigned long time_delay = jiffies + msecs_to_jiffies(INFO_REPORT_TIME);
	virt_eth_comm_work *v_work_comm = kmalloc(sizeof(*v_work_comm), GFP_KERNEL);
	v_work_comm->dev = dev;

	v_work_comm->type = R_QOS_QUEUE_OCCUPY;
	INIT_DELAYED_WORK(&v_work_comm->delayed_work,
			virt_eth_comm_create_frame);
	queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			   &v_work_comm->delayed_work,
			   time_delay - jiffies);
}

u8 virt_eth_do_comm_board(struct net_device *dev,struct sk_buff *skb,u32 len){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	char ctl_type = skb->data[ETH_HLEN];
	struct channel_config_info * rec_channel_infor = (struct channel_config_info *)(skb->data+ ETH_HLEN);
	u8 ret = 1;
	u32 msecs = 10;
	unsigned long time_delay = jiffies + msecs_to_jiffies(msecs);
	virt_eth_comm_work *v_work_comm = kmalloc(sizeof(*v_work_comm), GFP_KERNEL);
	u8 buf[MAC_PKT_BUF - sizeof(S_MGMT_MSG)];
	u32 buf_len = 0;
	S_PARAM_FREQ sfreq;
	v_work_comm->dev = dev;

	//printk("virt_eth_do_comm_board type %d\n",ctl_type);

	switch(ctl_type){
	case Q_CHANNEL_CONFIG_INFO:
	{
		//printk("virt_eth_do_comm_board Q_CHANNEL_CONFIG_INFO\n");
		//generate response message
		v_work_comm->type = A_CHANNEL_CONFIG_INFO;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
			virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			&v_work_comm->delayed_work,
			time_delay - jiffies);
		break;
	}
	case S_CHANNEL_CONFIG_INFO:
	{
		//printk("virt_eth_do_comm_board Set CHANNEL QUALITY INFROMATION,freq = %d\n",ntohl(rec_channel_infor->channel_freq));

		jgk_comm_board_parameter_data.frequency = ntohl(rec_channel_infor->channel_freq);
		jgk_comm_board_parameter_data.device_mtu = ntohs(rec_channel_infor->device_mtu);
		jgk_comm_board_parameter_data.mode = ntohs(rec_channel_infor->mode);
		jgk_comm_board_parameter_data.bandwidth = ntohl(rec_channel_infor->channel_bw);
		jgk_comm_board_parameter_data.channel_rate = ntohl(rec_channel_infor->channel_rate);
		jgk_comm_board_parameter_data.channel_type = ntohs(rec_channel_infor->channel_type);
		memcpy(&(jgk_comm_board_parameter_data.device_mac),&(rec_channel_infor->device_mac),ETH_ALEN);
		//set frequenc information to MAC
		sfreq.msg.type = freq;
		sfreq.msg.len = 2;
		sfreq.freq =  ntohl(rec_channel_infor->channel_freq);
		memcpy(buf + buf_len,(u8*)&sfreq,sizeof(S_PARAM_FREQ));
		buf_len += sizeof(S_PARAM_FREQ);
		virt_eth_mgmt_send_msg(dev,MGMT_MAC_SETTING,buf,buf_len);
		
		//generate response message
		v_work_comm->type = G_CHANNEL_CONFIG_INFO;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
			virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			&v_work_comm->delayed_work,
			time_delay - jiffies);
		break;
	}
	case Q_CHANNEL_QUALITY_INFO:
	{
		//amp

		//printk("virt_eth_do_comm_board Q_CHANNEL_QUALITY_INFO\n");

		v_work_comm->type = A_CHANNEL_QUALITY_INFO;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
				virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
				   &v_work_comm->delayed_work,
				   time_delay - jiffies);

//		virt_eth_create_frame(A_CHANNEL_QUALITY_INFO,dev);
		break;
	}
	case Q_QOS_QUEUE_INFO:
	{
		v_work_comm->type = A_QOS_QUEUE_INFO;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
				virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
				   &v_work_comm->delayed_work,
				   time_delay - jiffies);
//		virt_eth_create_frame(A_QOS_QUEUE_INFO,dev);
		break;
	}
	case Q_CHANNEL_LDLE_RETIO_MSG:
	{
		v_work_comm->type = A_CHANNEL_LDLE_RETIO_MSG;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
				virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
				   &v_work_comm->delayed_work,
				   time_delay - jiffies);
//		virt_eth_create_frame(A_CHANNEL_LDLE_RETIO_MSG,dev);
		break;
	}
	case Q_PRIORITY_R_S_PKG_NUM:
	{
		v_work_comm->type = A_PRIORITY_R_S_PKG_NUM;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
				virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
				   &v_work_comm->delayed_work,
				   time_delay - jiffies);
//		virt_eth_create_frame(A_PRIORITY_R_S_PKG_NUM,dev);
		break;
	}
	case Q_PRIORITY_OVERFLOW_PKG_NUM:
	{
		v_work_comm->type = A_PRIORITY_OVERFLOW_PKG_NUM;
		INIT_DELAYED_WORK(&v_work_comm->delayed_work,
				virt_eth_comm_create_frame);
		queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
				   &v_work_comm->delayed_work,
				   time_delay - jiffies);
//		virt_eth_create_frame(A_PRIORITY_OVERFLOW_PKG_NUM,dev);
		break;
	}
	default:
		break;
	}
	return ret;
}

//u8 virt_eth_queue_info_report(){
//
//}
