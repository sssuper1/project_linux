/*
 * virt_eth_jgk.c
 *
 *  Created on: 2020-5-16
 *      Author: lu
 */

#include <linux/jiffies.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
//#include <asm/unaligned.h>
#include <linux/ip.h>
#include <linux/udp.h>
//#include <arpa/inet.h>

#include "virt_eth_jgk.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_util.h" 



u8  jgk_set_parameter_flag;
jgk_report_infor jgk_information_data;
jgk_comm_board_parameter jgk_comm_board_parameter_data;
u8  g_u8Bw;
u8  g_u8Slotlen;

u32 start_tcp_seq_no;
u8 version[4]={0,0,9,2};

unsigned int inet_addr(const char *str)
{
	unsigned int lHost = 0;
	int i = 1, j = 1;
	const char *pstr[4] = { NULL };
	pstr[0] = strchr(str, '.');
	pstr[1] = strchr(pstr[0] + 1, '.');
	pstr[2] = strchr(pstr[1] + 1, '.');
	pstr[3] = strchr(str, '\0');

	for (j = 0; j < 4; j++)
	{
		i = 1;
		if (j == 0)
		{
			while (str != pstr[0])
			{
				lHost += (*--pstr[j] - '0') * i;
				i *= 10;
			}
		}
		else
		{
			while (*--pstr[j] != '.')
			{
				lHost += (*pstr[j] - '0') * i << 8 * j;
				i *= 10;
			}
		}
	}
	return lHost;
}
void virt_eth_jgk_init(struct net_device *dev){
	//jgk_information_data = kmalloc(sizeof(*jgk_report_infor), GFP_ATOMIC);
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	int i;


	
	jgk_set_parameter_flag = SET_FALSE;
	memset((void*)&jgk_information_data,0,sizeof(jgk_report_infor));
    for(i=0;i<NET_SIZE;i++){
		jgk_information_data.mac_information_part1.nbr_list[i] = LinkSt_idle;
	}

    memcpy(jgk_information_data.veth_version,version,sizeof(version));
	memcpy(jgk_information_data.ctrl_version,version,sizeof(version));
	
	memset((void*)&jgk_comm_board_parameter_data,0,sizeof(jgk_comm_board_parameter));
	jgk_comm_board_parameter_data.bandwidth = 10000;
	jgk_comm_board_parameter_data.channel_type = 2;
	jgk_comm_board_parameter_data.device_mtu = 1475;
	jgk_comm_board_parameter_data.mode = 0;
	jgk_comm_board_parameter_data.channel_rate = 20000;
	jgk_comm_board_parameter_data.device_mac[0] =  0xb8;
	jgk_comm_board_parameter_data.device_mac[1] =  0x8e;
	jgk_comm_board_parameter_data.device_mac[2] =  0xdf;
	jgk_comm_board_parameter_data.device_mac[3] =  0x00;
	jgk_comm_board_parameter_data.device_mac[4] =  0x01;
	jgk_comm_board_parameter_data.device_mac[5] = dev->dev_addr[5];


	memset((void*)&veth_priv->v_jgk_info,0,sizeof(virt_eth_jgk_info));

	start_tcp_seq_no = 0;

	g_u8Bw = 0;
	g_u8Slotlen =0;
	//printk("192.168.232.1 chang to int  = %d\n",htonl(inet_addr("192.168.232.1")));

	//printk("virt_eth_jgk_init\n");
	//printk("channel_type= %d,device_mtu = %d,channel_freq = %d,channel_bw = %d, mac = %d\n",
	//	jgk_comm_board_parameter_data.channel_type,jgk_comm_board_parameter_data.device_mtu,jgk_comm_board_parameter_data.frequency,
	//	jgk_comm_board_parameter_data.bandwidth,jgk_comm_board_parameter_data.device_mac[5]);
}
u16 ipCksum(void *ip, int len) //��Ϊ�ײ����ȹ̶�������ʹ��ʱlenΪ��ֵ20
{
	u16 *buf = (u16*)ip; //ÿ��ȡ16λ
	u32 cksum = 0;

	while(len > 1)
	{
		cksum += *buf++;
		len -= sizeof(u16);
	}

	if(len)
		cksum += *(u16*)buf;

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	//    xil_printf("cksum = %x\n",cksum);
	return (u16)(~cksum);
}

void virt_eth_jgk_report_packet(struct work_struct *work)
{
	struct virt_eth_priv *veth_priv;
	virt_eth_work_jgk* work_jgk;
	struct delayed_work *delayed_work;
	struct sk_buff *skb, *skb_jgk_set,*skb_jgk_set2;
	u32 skb_size;
	unsigned char *skb_buff;


	unsigned int remote_ip,local_ip;
	int udp_len;
	int ip_len;
	u8   ret = 0;
	int i=0;
	struct virt_eth_hard_iface *hard_iface;
	//unsigned char nodeid;
	unsigned char send_data_buf[sizeof(ethernet_header_t) + sizeof(jgk_set_parameter)];
	unsigned char send_data_buf2[sizeof(ethernet_header_t) +sizeof(ipv4_header_t) + sizeof(udp_header_t)+ sizeof(jgk_set_parameter)];
	ethernet_header_t *eth = (ethernet_header_t *)send_data_buf;
	ethernet_header_t *eth2 = (ethernet_header_t *)send_data_buf2;
	ipv4_header_t* iph = (ipv4_header_t *)(send_data_buf2+sizeof(ethernet_header_t));
	udp_header_t* udph = (udp_header_t *)(send_data_buf2+sizeof(ethernet_header_t) + sizeof(ipv4_header_t));
	u8 remote_mac[MAC_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
	//u8 local_mac[MAC_ADDR_LEN]  = {0x00,0x0c,0x29,0xb4,0xc1,0xf4};



	unsigned char buf[64] = {0xb8,0x8e,0xdf,0x00,0x01,0x01,0xb8,0x8e,0xdf,0x00,0x01,0x01,0x73,0x45,
			0x08,0x0f,0x32,0x00,0x00,0x00,0x00,0x00,
			0xff,0xff,0xff,0xff,0xff,0xff,0xd4,0x3d,0x7e,0xcc,0xdf,0x31,0x08,0x00,0x45,0x00,0x04,0x8c,
			0x0b,0x4d,0x00,0x00,0x40,0x11,0x90,0x9b,0xc0,0xa8,0x02,0x79,0xff,0xff,0xff,0xff,0xfa,0x20,0x4e,0x22,0x04,0x78,0x00,0x00};


	delayed_work = to_delayed_work(work);
	work_jgk = container_of(delayed_work, virt_eth_work_jgk,
					   delayed_work);

	veth_priv = netdev_priv(work_jgk->dev);
	skb_size = 64 + sizeof(virt_eth_jgk_info);

	skb = dev_alloc_skb(skb_size);

	if(!skb)
	{
		//printk("skb error!\n");
		goto err;
	}

	skb_buff = skb_put(skb, skb_size);

	buf[5] = veth_priv->addr[5];
	buf[11] = veth_priv->addr[5];
	memcpy(skb_buff, buf, 64);
	memcpy(skb_buff + 64, (void*)&veth_priv->v_jgk_info, sizeof(virt_eth_jgk_info));
#ifdef Docker_Qualnet
	memcpy((void*)&jgk_information_data[veth_priv->addr[5]-1].traffic_queue_information,(void*)&veth_priv->v_jgk_info, sizeof(virt_eth_jgk_info));
#elif defined Zynq_Platform
	memcpy((void*)&jgk_information_data.traffic_queue_information,(void*)&veth_priv->v_jgk_info, sizeof(virt_eth_jgk_info));
#endif
	memset((void*)&veth_priv->v_jgk_info,0,sizeof(virt_eth_jgk_info));
	skb->dev = work_jgk->dev;
	skb->protocol = eth_type_trans(skb, work_jgk->dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	netif_rx_ni(skb);
	veth_priv->v_jgk_info.mac_list_tx_cnt = 0;
	veth_priv->v_jgk_info.phy_rx_done_cnt = 0;
	veth_priv->v_jgk_info.phy_tx_done_cnt = 0;
	veth_priv->v_jgk_info.tx_in_cnt = 0;
	//
#ifdef Docker_Qualnet
	//printk("!!node_id =%d,veth_priv->addr[5] = %d\n ",jgk_information_data[veth_priv->addr[5]-1].mac_information_part1.node_id,veth_priv->addr[5]);
#endif
	//jgk_set_parameter_flag[veth_priv->addr[5]-1] = SET_TURE;
#ifdef Docker_Qualnet
	if (jgk_set_parameter_flag[veth_priv->addr[5]-1] == SET_TURE)
	{
		hard_iface = virt_eth_hardif_get_netdev(work_jgk->dev);
		memset(send_data_buf,0,sizeof(send_data_buf));

		memcpy(eth->src_mac_addr, veth_priv->addr, MAC_ADDR_LEN);
		memcpy(eth->dest_mac_addr, remote_mac, MAC_ADDR_LEN);
		eth->ethertype = htons(0x4308);

		memcpy(send_data_buf+sizeof(ethernet_header_t),(void*)&jgk_comm_board_parameter_data[veth_priv->addr[5]-1],sizeof(jgk_set_parameter));
		skb_size = sizeof(ethernet_header_t) + sizeof(jgk_set_parameter);
		//printk("packet size = %d\n",skb_size);
		skb_jgk_set = alloc_skb(skb_size, GFP_ATOMIC);
		if (!skb_jgk_set)
			return;
		skb_put(skb_jgk_set, skb_size);

		memcpy(skb_jgk_set->data,send_data_buf,skb_size);
		skb_jgk_set->protocol = htons(0x4308);
		skb_jgk_set->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		skb_jgk_set->dev = hard_iface->net_dev;


		ret = dev_queue_xmit(skb_jgk_set);
		//printk("mac xmit return value = %d\n",ret);

		//udp_len = sizeof(*udph)+sizeof(jgk_set_parameter);

		//udph->src_port = htons(56789);
		//udph->dest_port = htons(5138);
		//udph->length = htons(udp_len);
		//udph->checksum = 0;

		//remote_ip = inet_addr("192.168.1.2");
		//local_ip = inet_addr("192.168.1.1");
		//ip_len = udp_len + sizeof(*iph);
		//iph->version_ihl = 0x45;
		//iph->dscp_ecn = 0;
		//iph->total_length = htons(ip_len);
		//iph->identification  = htons(1560);
		//iph->fragment_offset = 0;
		//iph->ttl      = 64;
		//iph->protocol = IPV4_PROT_UDP;
		//iph->src_ip_addr = local_ip;
		//iph->dest_ip_addr = remote_ip;
		//iph->header_checksum = 0x00;
		//iph->header_checksum = ipCksum(iph,20);

		//memcpy(eth2->src_mac_addr, veth_priv->addr, MAC_ADDR_LEN);
		//memcpy(eth2->dest_mac_addr, remote_mac, MAC_ADDR_LEN);
		//eth2->ethertype = htons(0x0800);
		//memcpy(send_data_buf2+sizeof(ethernet_header_t)+sizeof(ipv4_header_t) + sizeof(udp_header_t),(void*)&jgk_comm_board_parameter_data[veth_priv->addr[5]-1],sizeof(jgk_set_parameter));
		//skb_size = sizeof(ethernet_header_t) +sizeof(ipv4_header_t) + sizeof(udp_header_t)+ sizeof(jgk_set_parameter);
		////printk("packet size = %d\n",skb_size);
		//skb_jgk_set2 = alloc_skb(skb_size, GFP_ATOMIC);
		//if (!skb_jgk_set2)
		//	return;
		//skb_put(skb_jgk_set2, skb_size);

		//memcpy(skb_jgk_set2->data,send_data_buf2,skb_size);
		//skb_jgk_set->protocol = htons(0x0800);
		//skb_jgk_set->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		//skb_jgk_set2->dev = hard_iface->net_dev;
		////printk("send udp data = ");
		////for (i=0;i<sizeof(ethernet_header_t) +sizeof(ipv4_header_t) + sizeof(udp_header_t);i++)
		////{
		////	printk(" %x ",skb_jgk_set2->data[i]);
		////}
		////printk("\n");

		//ret = dev_queue_xmit(skb_jgk_set2);
		//printk("udp xmit return value = %d\n",ret);

		jgk_set_parameter_flag[veth_priv->addr[5]-1] == SET_FALSE;
	}
#endif

err:
	kfree(work_jgk);
	virt_eth_jgk_schedule(veth_priv);
}

void virt_eth_jgk_iq_report(struct work_struct *work)
{
	struct virt_eth_priv *veth_priv;
	virt_eth_work_iq* work_iq;
	struct delayed_work *delayed_work;
	struct sk_buff *skb, *skb_jgk_set,*skb_jgk_set2;
	u32 skb_size;
	unsigned char *skb_buff;


	unsigned int remote_ip,local_ip;
	int udp_len;
	int ip_len;
	u8   ret = 0;
	int i=0;
	struct virt_eth_hard_iface *hard_iface;
	unsigned char send_data_buf[sizeof(ethernet_header_t) + sizeof(jgk_set_parameter)];
	unsigned char send_data_buf2[sizeof(ethernet_header_t) +sizeof(ipv4_header_t) + sizeof(udp_header_t)+ sizeof(jgk_set_parameter)];
	ethernet_header_t *eth = (ethernet_header_t *)send_data_buf;
	ethernet_header_t *eth2 = (ethernet_header_t *)send_data_buf2;
	ipv4_header_t* iph = (ipv4_header_t *)(send_data_buf2+sizeof(ethernet_header_t));
	udp_header_t* udph = (udp_header_t *)(send_data_buf2+sizeof(ethernet_header_t) + sizeof(ipv4_header_t));
	u8 remote_mac[MAC_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

	unsigned char buf[64] = {0xb8,0x8e,0xdf,0x00,0x01,0x01,0xb8,0x8e,0xdf,0x00,0x01,0x01,0x73,0x45,
			0x08,0x0f,0x32,0x00,0x00,0x00,0x00,0x00,
			0xff,0xff,0xff,0xff,0xff,0xff,0xd4,0x3d,0x7e,0xcc,0xdf,0x31,0x08,0x00,0x45,0x00,0x04,0x8c,
			0x0b,0x4d,0x00,0x00,0x40,0x11,0x90,0x9b,0xc0,0xa8,0x02,0x79,0xff,0xff,0xff,0xff,0xfb,0x20,0x4e,0x22,0x04,0x78,0x00,0x00};


	delayed_work = to_delayed_work(work);
	work_iq = container_of(delayed_work, virt_eth_work_iq,
					   delayed_work);

	veth_priv = netdev_priv(work_iq->dev);
	skb_size = 64 + UDP_IQ_PKT_LEN;
	skb = dev_alloc_skb(skb_size);

	if(!skb)
	{
		//printk("skb error!\n");
		goto err;
	}

	skb_buff = skb_put(skb, skb_size);

	buf[5] = veth_priv->addr[5];
	buf[11] = veth_priv->addr[5];
	memcpy(skb_buff, buf, 64);
	memcpy(skb_buff + 64, work_iq->pu8Pkt, UDP_IQ_PKT_LEN);
#ifdef Docker_Qualnet
	memcpy((void*)&jgk_information_data[veth_priv->addr[5]-1].traffic_queue_information,(void*)&veth_priv->v_jgk_info, sizeof(virt_eth_jgk_info));
#elif defined Zynq_Platform
	memcpy((void*)&jgk_information_data.traffic_queue_information,(void*)&veth_priv->v_jgk_info, sizeof(virt_eth_jgk_info));
#endif
	skb->dev = work_iq->dev;
	skb->protocol = eth_type_trans(skb, work_iq->dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	netif_rx_ni(skb);

err:
	if(IQ_BUF_LEN > work_iq->u32PktLen+UDP_IQ_PKT_LEN)
	{
		virt_eth_jgk_iq_schedule(veth_priv,work_iq->pu8Pkt+UDP_IQ_PKT_LEN,work_iq->u32PktLen+UDP_IQ_PKT_LEN);
	}
	kfree(work_iq);
}

void virt_eth_jgk_schedule(struct virt_eth_priv *veth_priv){
	unsigned long time_delay = jiffies + msecs_to_jiffies(1000);

	virt_eth_work_jgk *work_jgk = kmalloc(sizeof(*work_jgk), GFP_KERNEL);
	work_jgk->dev = veth_priv->soft_dev;

	INIT_DELAYED_WORK(&work_jgk->delayed_work,
			virt_eth_jgk_report_packet);
	queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			   &work_jgk->delayed_work,
			   time_delay - jiffies);
}

void virt_eth_jgk_iq_schedule(struct virt_eth_priv *veth_priv,u8* pu8Pkt,u32 u32PktLen){
	unsigned long time_delay = jiffies + msecs_to_jiffies(20);

	virt_eth_work_iq *work_iq = kmalloc(sizeof(*work_iq), GFP_KERNEL);
	work_iq->dev = veth_priv->soft_dev;
	work_iq->pu8Pkt = pu8Pkt;
	work_iq->u32PktLen = u32PktLen;

	INIT_DELAYED_WORK(&work_iq->delayed_work,
			virt_eth_jgk_iq_report);
	queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			   &work_iq->delayed_work,
			   time_delay - jiffies);
}

u8 * virt_eth_jgk_info_get(u8 node_id)
{
	int i;
#ifdef Docker_Qualnet
	u8 * data =(u8*)jgk_information_data[node_id];

	//struct net_device *soft_iface = dev_get_by_name(&init_net,"eth1");
	//struct virt_eth_priv *veth_priv = netdev_priv(soft_iface);

	//printk("soft_iface %x\n",(int)soft_iface);
	//test 
	for ( i=0;i<MCS_NUM;i++)
	{
		jgk_information_data[node_id].enqueue_bytes[i] = 1500;
		jgk_information_data[node_id].outqueue_bytes[i] = 2500;
	}

	jgk_information_data[node_id].mac_information_part1.node_id = node_id; 
	jgk_information_data[node_id].traffic_queue_information.tx_inall = 2000;
	jgk_information_data[node_id].traffic_queue_information.tx_outall = 2000;
	jgk_information_data[node_id].traffic_queue_information.rx_inall = 3000;
	jgk_information_data[node_id].traffic_queue_information.rx_outall = 3000;
#elif defined Zynq_Platform
		u8 * data =(u8*)&jgk_information_data;

	//struct net_device *soft_iface = dev_get_by_name(&init_net,"eth1");
	//struct virt_eth_priv *veth_priv = netdev_priv(soft_iface);

	//printk("soft_iface %x\n",(int)soft_iface);
	//test 
/*	for ( i=0;i<MCS_NUM;i++)
	{
		jgk_information_data.enqueue_bytes[i] = 1500;
		jgk_information_data.outqueue_bytes[i] = 2500;
	}

	jgk_information_data.mac_information_part1.node_id = 1; 
	jgk_information_data.traffic_queue_information.tx_inall = 2000;
	jgk_information_data.traffic_queue_information.tx_outall = 2000;
	jgk_information_data.traffic_queue_information.rx_inall = 3000;
	jgk_information_data.traffic_queue_information.rx_outall = 3000;
*/

	//memcpy((void*)&jgk_information_data->traffic_queue_information,(void*)&veth_priv->v_jgk_info,sizeof(virt_eth_jgk_info));
	//memcpy((void*)jgk_information_data->enqueue_bytes,(void*)veth_priv->enqueue_bytes,MCS_NUM);
	//memcpy((void*)jgk_information_data->outqueue_bytes,(void*)veth_priv->outqueue_bytes,MCS_NUM);
	//memcpy((void*)&jgk_information_data->mac_information_part1,(void*)&veth_priv->mac_information_part1,sizeof(ob_state_part1));
	//memcpy((void*)&jgk_information_data->mac_information_part2,(void*)&veth_priv->mac_information_part2,sizeof(ob_state_part2));
#endif
	return data;
}
EXPORT_SYMBOL(virt_eth_jgk_info_get);

void virt_eth_jgk_param_set(u8 node_id,jgk_set_parameter * parameter_data)

{
	jgk_set_parameter param_temp;
	struct net_device *dev;
	struct virt_eth_priv *veth_priv;
	//struct net_device *soft_iface = dev_get_by_name(&init_net,"eth1");
	//struct virt_eth_priv *veth_priv = netdev_priv(soft_iface);
	u8 i = 0;
	u8 buf[MAC_PKT_BUF - sizeof(S_MGMT_MSG)];
	u32 buf_len = 0;
	u8 issetmac = 0;
	u8 u8IsSetAmpdu = 0;
	int j;

	memcpy((void*)&param_temp,(void*)parameter_data,sizeof(jgk_set_parameter));

	jgk_set_parameter_flag = SET_FALSE;
	
//	printk("virt_eth_jgk_param_set %d\n",param_temp.indication);

	dev = dev_get_by_name(&init_net,"eth1");
	if(dev == NULL)
		return;
	veth_priv = netdev_priv(dev);
	if(veth_priv == NULL)
		return;

//	printk("dev %d %d\n",(int)dev,(int)veth_priv);


	if (param_temp.indication & SET_QUEUE_NUM)
	{
		jgk_comm_board_parameter_data.enqueue_num = param_temp.enqueue_num;
		//printk("set queue number = %d\n",jgk_comm_board_parameter_data.enqueue_num);
		i++;
	}
	if (param_temp.indication & SET_QUEUE_LENGTH)
	{
		jgk_comm_board_parameter_data.queue_length = param_temp.queue_length;
		//printk("set queue length = %d\n",jgk_comm_board_parameter_data.queue_length);
		i++;
	}
	if (param_temp.indication & SET_QOS_STATEGY)
	{
		jgk_comm_board_parameter_data.QoS_Stategy = param_temp.QoS_Stategy;
		//printk("set qos stategy = %d\n",jgk_comm_board_parameter_data.QoS_Stategy);
		i++;
	}
	if (param_temp.indication & SET_UNICAST_MCS)
	{
		if(param_temp.unicast_msc >=  MCS_NUM)
			veth_priv->mcs_mode = NFIX_MCS_MODE;
		else{
			veth_priv->ucast_mcs = param_temp.unicast_msc;
			veth_priv->mcs_mode = FIX_MCS_MODE;
			veth_priv->bcast_mcs = veth_priv->ucast_mcs;

			S_PARAM_MCS_MODE smcsmode;
			smcsmode.msg.type = mcs_mode;
			smcsmode.msg.len = 2;
			smcsmode.mcs_mode = veth_priv->ucast_mcs;
			memcpy(buf + buf_len,(u8*)&smcsmode,sizeof(S_PARAM_MCS_MODE));
			buf_len += sizeof(S_PARAM_MCS_MODE);

			jgk_comm_board_parameter_data.unicast_msc = param_temp.unicast_msc;

			issetmac = 1;
			printk("virt-eth0:set unicast mcs = %d\n",veth_priv->ucast_mcs);
			i++;
		}
	}
	if (param_temp.indication & SET_MULTICAST_MCS)
	{
		veth_priv->bcast_mcs = param_temp.multicast_msc;

		jgk_comm_board_parameter_data.multicast_msc = param_temp.multicast_msc;
		printk("virt-eth0:set bcast_mcs = %d\n",veth_priv->bcast_mcs);
		i++;
	}
	if (param_temp.indication & SET_FREQUENCY)
	{
		S_PARAM_FREQ sfreq;
		sfreq.msg.type = freq;
		sfreq.msg.len = 2;
		sfreq.freq = param_temp.frequency;
		memcpy(buf + buf_len,(u8*)&sfreq,sizeof(S_PARAM_FREQ));
		buf_len += sizeof(S_PARAM_FREQ);
		jgk_comm_board_parameter_data.frequency = param_temp.frequency;
		issetmac = 1;
		printk("virt-eth0:set send frequency = %d\n",jgk_comm_board_parameter_data.frequency);
		i++;
	}
	if (param_temp.indication & SET_POWER)
	{
		S_PARAM_POWER spower;
		spower.msg.type = power;
		spower.msg.len = sizeof(S_PARAM_POWER) - sizeof(S_PARAM_MSG);
		spower.tx1a_power = param_temp.power_ch[0];
		spower.tx1b_power = param_temp.power_ch[1];
		spower.tx2a_power = param_temp.power_ch[2];
		spower.tx2b_power = param_temp.power_ch[3];
		memcpy(buf + buf_len,(u8*)&spower,sizeof(S_PARAM_POWER));
		buf_len += sizeof(S_PARAM_POWER);

		jgk_comm_board_parameter_data.power = param_temp.power;
		memcpy(jgk_comm_board_parameter_data.tx_power_ch, param_temp.power_ch, sizeof(param_temp.power_ch));
		issetmac = 1;
		printk("virt-eth0:set send power ch = %d %d %d %d \n",
			jgk_comm_board_parameter_data.tx_power_ch[0],
			jgk_comm_board_parameter_data.tx_power_ch[1],
			jgk_comm_board_parameter_data.tx_power_ch[2],
			jgk_comm_board_parameter_data.tx_power_ch[3]);
		i++;
	}
	if (param_temp.indication & SET_BANDWIDTH)
	{
		S_PARAM_BW smacbw;
		smacbw.msg.type = bw;
		smacbw.msg.len = 2;
		smacbw.bw = param_temp.bandwidth;
		memcpy(buf + buf_len,(u8*)&smacbw,sizeof(S_PARAM_BW));
		buf_len += sizeof(S_PARAM_BW);
		jgk_comm_board_parameter_data.bandwidth = param_temp.bandwidth;
		issetmac = 1;
		if(g_u8Bw != jgk_comm_board_parameter_data.bandwidth)
		{
		    g_u8Bw = jgk_comm_board_parameter_data.bandwidth;
		    u8IsSetAmpdu = 1;;
		}
		
	#ifdef Radio_220
		veth_priv->channel_num = param_temp.bandwidth;
	    printk("virt-eth0:Set Voice Channel num = %d\n",veth_priv->channel_num);
	#endif
		//printk("virt-eth0:set send bandwidth = %d\n",jgk_comm_board_parameter_data.bandwidth);
		i++;
	}
	if (param_temp.indication & SET_TEST_MODE)
	{
		S_PARAM_MAC_MODE smacmode;
		smacmode.msg.type = mac_mode;
		smacmode.msg.len = 2;
		smacmode.mac_mode = param_temp.test_send_mode;
		memcpy(buf + buf_len,(u8*)&smacmode,sizeof(S_PARAM_MAC_MODE));
		buf_len += sizeof(S_PARAM_MAC_MODE);
		jgk_comm_board_parameter_data.test_send_mode = param_temp.test_send_mode;
		issetmac = 1;
		//printk("set test send mode = %d\n",jgk_comm_board_parameter_data.test_send_mode);
		i++;
	}
	if (param_temp.indication & SET_TEST_MODE_MCS)
	{
		jgk_comm_board_parameter_data.test_send_mode_mcs = param_temp.test_send_mode_mcs;
		//printk("set test send mode mcs = %d\n",jgk_comm_board_parameter_data.test_send_mode_mcs);
		i++;
	}
	if (param_temp.indication & SET_PHY)
	{
		S_PARAM_PHY_MSG sphymsg;
		sphymsg.msg.type = phy_msg;
		sphymsg.msg.len = sizeof(Smgmt_phy);
		sphymsg.phy_msg= param_temp.phy_msg;
		memcpy(buf + buf_len,(u8*)&sphymsg,sizeof(S_PARAM_PHY_MSG));
		buf_len += sizeof(S_PARAM_PHY_MSG);

//		jgk_comm_board_parameter_data.phy_msg = param_temp.phy_msg;
//		printk("virt phymsg = %d %d %d %d %d %d\n", param_temp.phy_msg.rf_agc_framelock_en,param_temp.phy_msg.phy_cfo_bypass_en,
//				param_temp.phy_msg.phy_pre_STS_thresh,param_temp.phy_msg.phy_pre_LTS_thresh,
//				param_temp.phy_msg.phy_tx_iq0_scale,param_temp.phy_msg.phy_tx_iq1_scale);
		i++;
		issetmac = 1;
	}
	if (param_temp.indication & SET_WORKMODE)
	{
		S_PARAM_FREQ_HOP stFreqHop;
		stFreqHop.msg.type = freq_hop;
		stFreqHop.msg.len = sizeof(u32)*32+2;
		stFreqHop.fh_mode= param_temp.work_mode_msg.NET_work_mode;
		stFreqHop.fh_len= param_temp.work_mode_msg.fh_len;
		memcpy((u8*)stFreqHop.hop_freq_tb,(u8*)param_temp.work_mode_msg.hop_freq_tb,sizeof(u32)*32);
		memcpy(buf + buf_len,(u8*)&stFreqHop,sizeof(S_PARAM_FREQ_HOP));

		if(HOP_FREQ_MODE == param_temp.work_mode_msg.NET_work_mode) // hop freq mode
		{
			
//			memcpy((void *)hp_tb,(void*)sworkmsg.hop_freq_tb,4*32);
//			sworkmsg.hop_freq_tb = param_temp.hop_freq_tb;
			printk("virt-eth0:hop table sizeof(S_PARAM_FREQ_HOP) %d \n",
				sizeof(S_PARAM_FREQ_HOP)); //freq %d %d ,param_temp.hop_freq_tb,sworkmsg.hop_freq_tb

			for(j = 0;j<HOP_FREQ_NUM;j++)
			{
//				sworkmsg.hop_freq_tb[i] = param_temp.hop_freq_tb[i];

				printk("param %d ",stFreqHop.hop_freq_tb[j]);
			}

		}
		buf_len += sizeof(S_PARAM_FREQ_HOP);

		printk("virt-eth0:set freq hop num = %d\n",param_temp.work_mode_msg.fh_len);

		i++;
		issetmac = 1;
	}
	if (param_temp.indication & SET_IQ_CATCH)
	{
		S_PARAM_IQ_CATCH stIqCatch;
		stIqCatch.msg.type = iq_catch;
		stIqCatch.msg.len = sizeof(u32)*2+2;
		stIqCatch.trig_mode= (u8)param_temp.iq_catch.trig_mode;
		stIqCatch.catch_addr= (u8)param_temp.iq_catch.catch_addr;
		stIqCatch.catch_length= (u8)param_temp.iq_catch.catch_length;
		memcpy(buf + buf_len,(u8*)&stIqCatch,sizeof(S_PARAM_IQ_CATCH));
		buf_len += sizeof(S_PARAM_IQ_CATCH);

		printk("virt-eth0:set iq catch mode = %d\n",param_temp.iq_catch.trig_mode);

		i++;
		issetmac = 1;
	}
	if (param_temp.indication & SET_SLOT_LEN)
	{
		S_PARAM_SLOT_LEN stSlotlen;
		stSlotlen.msg.type = slotlen;
		stSlotlen.msg.len = 2;
		stSlotlen.u8Slotlen= (u8)param_temp.u8Slotlen;
		memcpy(buf + buf_len,(u8*)&stSlotlen,sizeof(S_PARAM_SLOT_LEN));
		buf_len += sizeof(S_PARAM_SLOT_LEN);

		printk("virt-eth0:set slot len = %d\n",param_temp.u8Slotlen);

		i++;
		issetmac = 1;
	        if(g_u8Slotlen != param_temp.u8Slotlen)
		{
		    g_u8Slotlen = param_temp.u8Slotlen;
		    u8IsSetAmpdu = 1;;
		}
	}
	if (param_temp.indication & SET_POWER_LEVEL)
	{
		S_PARAM_POWER_LEVEL stPowerLevel;
		stPowerLevel.msg.type = power_level;
		stPowerLevel.msg.len = 2;
		stPowerLevel.level = param_temp.power_level;
		memcpy(buf + buf_len,(u8*)&stPowerLevel,sizeof(S_PARAM_POWER_LEVEL));
		buf_len += sizeof(S_PARAM_POWER_LEVEL);

		jgk_comm_board_parameter_data.power_level = param_temp.power_level;
		printk("[virt-eth0]set power_level = %d \r\n",jgk_comm_board_parameter_data.power_level);
		i++;
		issetmac = 1;
	}
	if (param_temp.indication & SET_POWER_ATTENUATION)
	{
		S_PARAM_POWER_ATTENUATION stPowerAttenuation;
		stPowerAttenuation.msg.type = power_attenuation;
		stPowerAttenuation.msg.len = 2;
		stPowerAttenuation.attenuation = param_temp.power_attenuation;
		memcpy(buf + buf_len,(u8*)&stPowerAttenuation,sizeof(S_PARAM_POWER_ATTENUATION));
		buf_len += sizeof(S_PARAM_POWER_ATTENUATION);

		jgk_comm_board_parameter_data.power_attenuation = param_temp.power_attenuation;
		printk("[virt-eth0]set power attenuation =  %d \r\n",jgk_comm_board_parameter_data.power_attenuation);

		i++;
		issetmac = 1;
	}
	if (param_temp.indication & SET_RX_CHANNEL_MODE)
	{
		S_PARAM_RX_CHANNEL_MODE srx_mode;
		srx_mode.msg.type = rx_channel_mode;
		srx_mode.msg.len = 2;
		srx_mode.mode = param_temp.rx_channel_mode;
		srx_mode.resv = 0;
		memcpy(buf + buf_len, (u8*)&srx_mode, sizeof(S_PARAM_RX_CHANNEL_MODE));
		buf_len += sizeof(S_PARAM_RX_CHANNEL_MODE);

		jgk_comm_board_parameter_data.rx_channel_mode = param_temp.rx_channel_mode;
		printk("[virt-eth0]set rx_channel_mode = %d\r\n", jgk_comm_board_parameter_data.rx_channel_mode);

		i++;
		issetmac = 1;
	}


	if (i>0)
	{
		jgk_set_parameter_flag = SET_TURE;
		jgk_comm_board_parameter_data.indication = param_temp.indication;
	}
	if(issetmac)
	{
		virt_eth_mgmt_send_msg(dev,MGMT_MAC_SETTING,buf,buf_len);
	}
	if(u8IsSetAmpdu)
	{
		set_TX_AMPDU_LEM_MAX_2(veth_priv,g_u8Bw,g_u8Slotlen);
	}

}
EXPORT_SYMBOL(virt_eth_jgk_param_set);
