/*
 * virt_eth_mgmt.c
 *
 *  Created on: 2020-4-14
 *      Author: lu
 */
#include <linux/rpmsg.h>

#include "virt_eth_mgmt.h"
#include "virt_eth_system.h"
#include "virt_eth_util.h"
#include "virt_eth_queue.h"
#include "virt_eth_jgk.h"
#include "virt_eth_station.h"
#include "virt_eth_slot_irq.h"

#ifdef Radio_SWARM_k1  
static int TS_V_COUNT = 8;
#elif defined Radio_SWARM_k2
static int TS_V_COUNT = 8;
#elif defined Radio_SWARM_k4
static int TS_V_COUNT = 8;
#elif defined Radio_SWARM_WNW
static int TS_V_COUNT = 8;
#elif defined Radio_220
static int TS_V_COUNT = 8;
#elif defined Radio_7800
static int TS_V_COUNT = 8;
#elif defined Radio_SWARM_S2
static int TS_V_COUNT = 4;
#elif defined Radio_CEC
static int TS_V_COUNT = 8;
#endif


#define OGM_OCP 3
#define PHY_CLK 120
#define EXT_CLK 150
#define BASE_RATE 20

#ifdef Docker_Qualnet
      static u16 mcs_tbl_mac_1sl[MCS_NUM] = {302,608,913,1219,3250,1219,1219,1219};
//      static u16 mcs_tbl_mac_1sl[MCS_NUM] = {50,50,50,50,50,1500,1500,1500}; //test
#elif defined Zynq_Platform
      //k1 mcs:{384,512,768,1024,1536,2048,2304,3072}
      //220 mcs:{128,128,128,128,128,128,128,128}
      //7800 mcs: {366,737,1107,1478,2219,2960,3330,3701}
#ifdef Radio_SWARM_k1 
    static u16 mcs_tbl_mac_1sl[MCS_NUM] ={384,512,768,1024,1536,2048,2304,3072}; 
#elif defined Radio_SWARM_k2
	static u16 mcs_tbl_mac_1sl[MCS_NUM] = {384,512,768,1024,1536,2048,2304,3072}; 
#elif defined Radio_SWARM_k4
	static u16 mcs_tbl_mac_1sl[MCS_NUM] = {384,512,768,1024,1536,2048,2304,3072}; 
#elif defined Radio_SWARM_WNW
	static u16 mcs_tbl_mac_1sl[MCS_NUM] = {384,512,768,1024,1536,2048,2304,3072}; 
#elif defined Radio_220
    static u16 mcs_tbl_mac_1sl[MCS_NUM] = {128,128,128,128,128,128,128,128};
#elif defined Radio_7800
   static u16 mcs_tbl_mac_1sl[MCS_NUM] = {366,737,1107,1478,2219,2960,3330,3701};
#elif defined Radio_SWARM_S2
   static u16 mcs_tbl_mac_1sl[MCS_NUM] = {1344,1792,2688,3584,5472,7296,8160,10880};//672,896,1344,1792,2688,3584,4032,5376
   static u16 mcs_tbl_mac_1sl_10M[MCS_NUM] = {672,896,1344,1792,2688,3584,4032,5376};//288,384,672,896,1344,1792,2016,2688
   static u16 mcs_tbl_mac_1sl_5M[MCS_NUM] = {288,384,576,768,1248,1664,1920,2560};//288,384,576,768,1248,1664,1920,2560

#elif defined Radio_CEC
   static u16 mcs_tbl_mac_1sl[MCS_NUM] = {2660,2660,2660,2660,2660,2660,2660,2660};

#endif
//{384,512,768,1024,1536,2048,2304,3072}
//		static u16 mcs_tbl_mac_1sl[MCS_NUM] = {374,502,758,1014,1536,2038,2294,3062};
#endif


//static u16 mcs_tbl_mac_1sl[MCS_NUM] = {182,588,893,1999,1700,2441,2746,3052};


#ifdef Radio_SWARM_S2
    static u8 qmcs_tab[MCS_NUM]  = {2,4,6,8,10,12,13,14};
#else 
    static u8 qmcs_tab[MCS_NUM] = {3,6,8,10,11,12,13,14};
#endif








static u8 virt_eth_mgmt_q_2_mcs(u8 q_back){
	u8 i = 0;
	if(q_back == 0x0f)
		return 0x0f;
	else{
		for(i = 0; i < 8; i ++)
		{
			if(q_back <= qmcs_tab[i])
				return i;
		}
	}
	return 0x0f;
}

static u8 virt_eth_mgmt_mcs_2_q(u8 mcs){
	if(mcs > 0x07)
		return 0x0f;
	else{
		return qmcs_tab[mcs];
	}
}

u8 virt_eth_mgmt_init(struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	u8 i = 0;
	for(i = 0; i < MAX_NODE; i ++)
	{
#ifdef Docker_Qualnet
		veth_priv->virt_traffic_param.mcs_value[i] = 0x04;
#elif defined Zynq_Platform
		veth_priv->virt_traffic_param.mcs_value[i] = 0x0f;
#endif
	}

	for(i = 0; i < MCS_NUM; i ++)
	{
		mcs_tbl_mac_1sl[i] -= WLAN_PHY_FCS_NBYTES;
		mcs_tbl_mac_1sl_10M[i] -= WLAN_PHY_FCS_NBYTES;
		mcs_tbl_mac_1sl_5M[i] -= WLAN_PHY_FCS_NBYTES;
		veth_priv->virt_traffic_param.pkt_enq_bytes[i] = 0;
		veth_priv->virt_traffic_param.pkt_outq_bytes[i] = 0;
		veth_priv->virt_traffic_param.pkt_enq_bytes_tmp[i] = 0;
		veth_priv->virt_traffic_param.flow_stat_offset[i] = 0;
	}


	memcpy(veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1,mcs_tbl_mac_1sl,sizeof(mcs_tbl_mac_1sl));
//	veth_priv->virt_mgmt_event_workqueue = create_singlethread_workqueue("virt_eth_events");
	veth_priv->virt_mgmt_event_workqueue = create_singlethread_workqueue("virt_eth_mgmt_events");
	veth_priv->virt_data_event_workqueue = create_singlethread_workqueue("virt_eth_data_events");
	veth_priv->virt_traffic_param.eth_data_seqno = 0;
	veth_priv->virt_traffic_param.q_hffull_flag = 0;
	veth_priv->virt_traffic_param.numb = 0;


	return 0;
}


void set_TX_AMPDU_LEM_MAX_1(struct virt_eth_priv *veth_priv,int mode){
        switch(mode)
	{
		case bw20m:
		{
			memcpy(veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1,mcs_tbl_mac_1sl,sizeof(mcs_tbl_mac_1sl));
			break;
		}
		case bw10m:
		{
			memcpy(veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1,mcs_tbl_mac_1sl_10M,sizeof(mcs_tbl_mac_1sl));
			break;
		}
		case bw5m:
		{
			memcpy(veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1,mcs_tbl_mac_1sl_5M,sizeof(mcs_tbl_mac_1sl));
			break;
		}
	}
}


u8 virt_eth_mgmt_get_mcs(struct virt_eth_priv *veth_priv,u8 node_id){
	if(node_id >= MAX_NODE)
		return 0x0f;
	if(veth_priv->mcs_mode == FIX_MCS_MODE)
		return veth_priv->ucast_mcs;
	return veth_priv->virt_traffic_param.mcs_value[node_id];
}

u16 virt_eth_mgmt_get_mcs_len_by_id(struct virt_eth_priv *veth_priv, u8 node_id){
	if(node_id >= MAX_NODE)
		return 0;
	if(veth_priv->virt_traffic_param.mcs_value[node_id] >= MCS_NUM)
		return 0;
	return veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1[veth_priv->virt_traffic_param.mcs_value[node_id]];
}

u16 virt_eth_mgmt_get_mcs_len_by_mcs(struct virt_eth_priv *veth_priv,u8 mcs){
	if(mcs >= MCS_NUM)
		return 0;
	return veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1[mcs];
}


void virt_eth_mgmt_send_msg(struct net_device *dev,E_MGMT_MSG type,void* data,u8 len){
#ifdef Zynq_Platform
	u8 buf[MAC_PKT_BUF];
	S_MGMT_MSG *msg = (S_MGMT_MSG*)buf;
	u32 buf_len = 0;
	struct _rpmsg_eptdev *local;
	struct virt_eth_priv* veth_priv = netdev_priv(dev);
	local = dev_get_drvdata(&(veth_priv->rpmsg_dev->dev));
	buf_len += sizeof(S_MGMT_MSG);

	switch(type){
	case MGMT_MAC_SETTING:
	{
		msg->type = MGMT_MAC_SETTING;
		msg->param_num = 1;
		msg->version = MSG_VERSION;
		msg->len = len;
		memcpy(buf+sizeof(S_MGMT_MSG),data,len);
		buf_len += len;
		break;
	}
	case MGMT_PARAM_QUERY:
	{
		break;
	}
	case MGMT_TX_READY:
	{
		msg->type = MGMT_TX_READY;
		msg->param_num = 1;
		msg->version = MSG_VERSION;
		msg->len = len;
		memcpy(buf+sizeof(S_MGMT_MSG),data,len);
		buf_len += len;
		break;
	}
	case MGMT_SLOT_NUM:
	{
		break;
	}
	default:
		break;
	}

	rpmsg_send(local->ept, buf,buf_len);
#endif

}

u8 virt_eth_mgmt_get_buffer(void){
	return 0;
}

void virt_eth_mgmt_jgk_info(struct net_device *dev,void* data,u32 len){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	u32 send_data_len = len;
	u8 fix_header[3];
	struct sk_buff* send_data = virt_eth_queue_create_skb(send_data_len);
	u8* buf = skb_put(send_data,send_data_len);
	int i;

	memcpy(buf,data,len);
	
	//get mac jgk information
#ifdef Docker_Qualnet
	memcpy(fix_header,data,sizeof(fix_header));
#elif defined Zynq_Platform
	memcpy(fix_header,data+64,sizeof(fix_header));
#endif

	
	if (fix_header[0]==0x03 && fix_header[1]==0xcc && fix_header[2]==0x04) //mac jgk packet part1
	{
//	    printk("fix_header = %x %x %x %d %d %d %d %d %d\n ",
//			buf[64],buf[65],buf[66],buf[67],buf[68],buf[69],buf[70],buf[71],buf[72]);

	//printk("Recieve mac jgk packet part1\n");
#ifdef Docker_Qualnet
		memcpy((void *)&jgk_information_data[veth_priv->addr[5]-1].mac_information_part1,data+sizeof(fix_header),sizeof(ob_state_part1));
#elif defined Zynq_Platform

#ifdef Radio_CEC
		memcpy((void *)&jgk_information_data.mac_information_part1,data+64+sizeof(fix_header),sizeof(ob_state_part1));
#else
	   memcpy((void *)&jgk_information_data.mac_information_part1,data+42+sizeof(fix_header)+1,sizeof(ob_state_part1));

#endif
       // printk("JGK data: node id = %d\n",jgk_information_data.mac_information_part1.node_id);
//		for(i=0;i<NET_SIZE;i++){
//           if(jgk_information_data.mac_information_part1.nbr_list[i] == LinkSt_h1){
//              printk("virt-eth0: rec jgk neighbor id = %d\n",i+1);
//		   }
//		}
#endif
	}
	if (fix_header[0]==0x01 && fix_header[1]==0xaa && fix_header[2]==0xbb)//mac jgk packet part2
	{
		//printk("Receive mac jgk packet part2\n");
#ifdef Docker_Qualnet
		memcpy((void *)&jgk_information_data[veth_priv->addr[5]-1].mac_information_part2,data+sizeof(fix_header),sizeof(ob_state_part2));
		if (veth_priv->addr[5] == 0x10)
		{
			printk("Receive MCS information:");
			for (i=0;i<16;i++)
			{
				printk(" %d ",jgk_information_data[veth_priv->addr[5]-1].mac_information_part2.mcs[i]);
			}
			printk("\n");
		}
#elif defined Zynq_Platform
#ifdef Radio_CEC
		memcpy((void *)&jgk_information_data.mac_information_part2,data+64+sizeof(fix_header),sizeof(ob_state_part2));
#else 
		memcpy((void *)&jgk_information_data.mac_information_part2,data+64+sizeof(fix_header)+1,sizeof(ob_state_part2));
#endif

#endif
	}
#ifdef Radio_SWARM_WNW
    if(jgk_comm_board_parameter_data.power == 55)
    {
       dev_kfree_skb(send_data);
    }
	else{
		  send_data->dev = dev;
		  send_data->protocol = eth_type_trans(send_data, dev);
		  send_data->ip_summed = CHECKSUM_UNNECESSARY;
		  dev->stats.rx_packets++;
		  dev->stats.rx_bytes += send_data_len;
		  netif_rx_ni(send_data);
	}        
#else
    send_data->dev = dev;
    send_data->protocol = eth_type_trans(send_data, dev);
    send_data->ip_summed = CHECKSUM_UNNECESSARY;
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += send_data_len;
    netif_rx_ni(send_data);
#endif
}

static void virt_eth_mgmt_route_info(struct net_device *dev,void* data,u32 len){
	virt_station_info * vstation = NULL;
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	u32 send_data_len = sizeof(S_BATMAN_ADV_ROUTE_INFO) + sizeof(ethernet_header_t);
	struct sk_buff* send_data = virt_eth_queue_create_skb(send_data_len);
	u8* buf = skb_put(send_data,send_data_len);
	u8  srcmac[MAC_ADDR_LEN] = {0xd4,0x3d,0x7e,0xcc,0xdf,0x31};
	ethernet_header_t *eth_hdr = (ethernet_header_t*)buf;
	S_BATMAN_ADV_ROUTE_INFO* b_route_info = (S_BATMAN_ADV_ROUTE_INFO*)(buf+sizeof(ethernet_header_t));
	u8 i = 0;

	S_TEST_INFO *test_info = (S_TEST_INFO*)data;
	veth_priv->v_jgk_info.mac_list_tx_cnt += test_info->mac_list_tx_cnt;
	veth_priv->v_jgk_info.phy_rx_done_cnt += test_info->phy_rx_done_cnt;
	veth_priv->v_jgk_info.phy_tx_done_cnt += test_info->phy_tx_done_cnt;
	veth_priv->v_jgk_info.tx_in_cnt += test_info->tx_in_cnt;



	memcpy(eth_hdr->dest_mac_addr,dev->dev_addr,MAC_ADDR_LEN);
	memcpy(eth_hdr->src_mac_addr,srcmac,MAC_ADDR_LEN);
	eth_hdr->ethertype = ETH_TYPE_BAT;

	b_route_info->type = BATMAN_ADV_ROUTE_INFO_TYPE;
	b_route_info->version = BATMAN_ADV_VERSION;
	b_route_info->flags = 0;
	b_route_info->len = len;
	b_route_info->ttl = 50;
	memcpy((void*)&b_route_info->route_info,data,sizeof(S_ROUTE_INFO));
	for(i = 0; i < MAX_NODE; i ++)
	{
		veth_priv->virt_traffic_param.mcs_value[i] = virt_eth_mgmt_q_2_mcs(b_route_info->route_info.forword_link_quality[i]);
//		if(veth_priv->virt_traffic_param.mcs_value[i] != 0x0f && veth_priv->mcs_mode == FIX_MCS_MODE){
//			if(veth_priv->virt_traffic_param.mcs_value[i] < veth_priv->ucast_mcs)
//				b_route_info->route_info.forword_link_quality[i] = 0x0f;
//			else
//				b_route_info->route_info.forword_link_quality[i] = veth_priv->ucast_mcs;
//		}
//		if(veth_priv->virt_traffic_param.mcs_value[i] != 0x0f){
////			printk("virt-eth0: node id = %d, mcs_value = %d\n ",i,veth_priv->virt_traffic_param.mcs_value[i]);
//			if(veth_priv->virt_traffic_param.mcs_value[i] < MULTICAST_MCS_DEFAULT_2){
//				if(veth_priv->mcs_mode != FIX_MCS_MODE){
//					b_route_info->route_info.forword_link_quality[i] = 0x0f;
//					veth_priv->virt_traffic_param.mcs_value[i] = 0x0f;
//				}
//			}
//			else if(veth_priv->virt_traffic_param.mcs_value[i] > UNICAST_MCS_DEFAULT){
//				b_route_info->route_info.forword_link_quality[i] = 11;
//				veth_priv->virt_traffic_param.mcs_value[i] = UNICAST_MCS_DEFAULT;
//			}
//		}
		if (veth_priv->virt_traffic_param.mcs_value[i] == 0x0f)
			{
			  b_route_info->route_info.forword_link_quality[i] = 0x0f;
			}
	}

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
		vstation->mcs = virt_eth_mgmt_get_mcs(veth_priv,vstation->id);
		if(vstation->mcs < MCS_NUM){
			vstation->state = STATION_ONLINE;
			vstation->last_seen = jiffies;
		}
		else
			vstation->state = STATION_OFFLINE;
	}
	rcu_read_unlock();


    send_data->dev = dev;
    send_data->protocol = eth_type_trans(send_data, dev);
    send_data->ip_summed = CHECKSUM_UNNECESSARY;
	dev->stats.rx_packets++;
	dev->stats.rx_bytes += send_data_len;
    netif_rx_ni(send_data);
//	dev_kfree_skb(send_data);
}




static void virt_eth_mgmt_do_param_report(struct net_device *dev,void* data,u32 len){
	S_MGMT_MSG* mmsg = (S_MGMT_MSG*)data;
	S_PARAM_MSG* param_msg = (S_PARAM_MSG*)(data+sizeof(S_MGMT_MSG));
	S_PARAM_ROUTE* route_msg = NULL;
	switch(param_msg->type)
	{
	case soft_id:
	case channel:
	case bw:
	case power:
	case mcs_mode:
	case mac_mode:
		break;
	case route:
	{
		route_msg = (S_PARAM_ROUTE*)(data+sizeof(S_MGMT_MSG));
		virt_eth_mgmt_route_info(dev,(void*)&route_msg->route_info,route_msg->msg.len);
		break;
	}
	case jgk:
	{
		route_msg = (S_PARAM_ROUTE*)(data+sizeof(S_MGMT_MSG));
		virt_eth_mgmt_jgk_info(dev,(u8*)param_msg + sizeof(S_PARAM_MSG),len - sizeof(S_MGMT_MSG) - sizeof(S_PARAM_MSG));
		break;
	}
	default:
		break;
	}
}

int virt_eth_mgmt_recv(struct net_device *dev,u8* data,u32 len){
	S_MGMT_MSG* mmsg = (S_MGMT_MSG*)data;
	int i = 0;
	int ret;
	//printk("virt_eth_mgmt_recv %d\n",mmsg->type);

	switch(mmsg->type){
	case MGMT_PARAM_REPORT:{
		//printk("MGMT_PARAM_REPORT\n");
			virt_eth_mgmt_do_param_report(dev,data,len);
			break;
		}

	case MGMT_MAC_SETTING_REPLY:{
//		virt_eth_system_rx_data_process(mmsg->offset,mmsg->datalen);
		virt_eth_system_param_set_report();

		break;
	}
	case MGMT_RX_READY:{
		if(len == (sizeof(S_MGMT_MSG) + VIRT_ETH_MGMT_RX_READY_LEN))
			virt_eth_system_rx_data_process(dev,data+sizeof(S_MGMT_MSG),VIRT_ETH_MGMT_RX_READY_LEN);
		break;
	}
	case MGMT_TX_DONE:{
//1231
//		printk("MGMT_TX_DONE %d\n",mmsg->param_num);
//		if(mmsg->param_num == 8){
//			for(i = 0; i < 7; i ++){
//				VIRT_ETH_BRAM_NUM[i] = 1;
//			}
//			break;
//		}
//		VIRT_ETH_BRAM_NUM[mmsg->param_num] = 1;
		virt_eth_system_tx_done(dev);

		break;
	}
#ifdef Radio_CEC
	case MGMT_IQR_START:{
   printk("virt-eth0: rec MGMT_IQR_START\n");

	
		ret = virt_eth_slot_irq_init();
		if(ret != 0)
			return ret;
		printk("slot_irq_init\n");

		break;
	}
#endif
	default:
		break;
		}

	return 0;
}



void virt_eth_mgmt_tf_change_cal(struct work_struct *work){

	struct virt_eth_priv *veth_priv;
	virt_eth_work_slot_num* work_slot_num;
	struct delayed_work *delayed_work;
	struct _rpmsg_eptdev *local;
	u8  ts_v[8];
	u8  ts_all;
	u8 i = 0;
	u32 pkt_outq_bytes_tmp[MCS_NUM];
	u32  enq_bytes[MCS_NUM];
	u32  outq_bytes[MCS_NUM];
	delayed_work = to_delayed_work(work);
	work_slot_num = container_of(delayed_work, virt_eth_work_slot_num,
		delayed_work);
	veth_priv = netdev_priv(work_slot_num->dev);
#ifdef Zynq_Platform
	u8 data[sizeof(S_MGMT_MSG)+sizeof(S_PARAM_SLOT_NUM)];
	S_MGMT_MSG* mgmt_msg = (S_MGMT_MSG*)data;
	S_PARAM_SLOT_NUM* slot_msg = (S_PARAM_SLOT_NUM*)(data+sizeof(S_MGMT_MSG));
	if(!veth_priv->rpmsg_dev)
		return;
	local = dev_get_drvdata(&(veth_priv->rpmsg_dev->dev));

	mgmt_msg->type = MGMT_SLOT_NUM;
	mgmt_msg->version = MSG_VERSION;
	mgmt_msg->param_num = 1;
	mgmt_msg->len = sizeof(S_PARAM_SLOT_NUM);
#endif
	ts_all = 0;

#ifdef Radio_CEC
	atomic_set(&veth_priv->slot_num_time, 1000*EXT_CLK/PHY_CLK*BASE_RATE/40);
#else
	switch(jgk_comm_board_parameter_data.bandwidth){
#ifdef Radio_7800
	case bw40m:{
		atomic_set(&veth_priv->slot_num_time, 1000*EXT_CLK/PHY_CLK*BASE_RATE/40);
		break;
			   }
#endif
	case bw20m:{
		atomic_set(&veth_priv->slot_num_time, 1000/**EXT_CLK/PHY_CLK*BASE_RATE/20*/);
		break;
	}
	case bw10m:{
		atomic_set(&veth_priv->slot_num_time, 1000/**EXT_CLK/PHY_CLK*BASE_RATE/10*/);
		break;
	}
	case bw5m:{
		atomic_set(&veth_priv->slot_num_time, 1750);
		break;
	}
//	case bw3m:{
//		atomic_set(&veth_priv->slot_num_time, 1000*EXT_CLK/PHY_CLK*BASE_RATE/3);
//		break;
//	}
//	case bw_1p2m:{
//		atomic_set(&veth_priv->slot_num_time, 1000*EXT_CLK/PHY_CLK*BASE_RATE*10/12);
//		break;
//	}
	default:{
		atomic_set(&veth_priv->slot_num_time, 1000);
		break;
	}
	}
#endif
	//get last second statistics information 
#ifdef Docker_Qualnet
	memcpy((void*)(jgk_information_data[veth_priv->addr[5]-1].enqueue_bytes),(void*)veth_priv->virt_traffic_param.pkt_enq_bytes,sizeof(u32)*MCS_NUM);
	memcpy((void*)(jgk_information_data[veth_priv->addr[5]-1].outqueue_bytes),(void*)veth_priv->virt_traffic_param.pkt_outq_bytes,sizeof(u32)*MCS_NUM);
#elif defined Zynq_Platform
	memcpy((void*)(jgk_information_data.enqueue_bytes),(void*)veth_priv->virt_traffic_param.pkt_enq_bytes,sizeof(u32)*MCS_NUM);
	memcpy((void*)(jgk_information_data.outqueue_bytes),(void*)veth_priv->virt_traffic_param.pkt_outq_bytes,sizeof(u32)*MCS_NUM);
#endif

	for(i = 0; i < MCS_NUM ; i ++){
		enq_bytes[i] = veth_priv->virt_traffic_param.pkt_enq_bytes[i];
		outq_bytes[i] = veth_priv->virt_traffic_param.pkt_outq_bytes[i];

		veth_priv->virt_traffic_param.pkt_enq_bytes[i] = 0;
		veth_priv->virt_traffic_param.pkt_outq_bytes[i] = 0;
	}


	for(i = 0; i < MCS_NUM ; i ++){
		//
		veth_priv->virt_traffic_param.pkt_enq_bytes_tmp[i] = \
				enq_bytes[i] +veth_priv->virt_traffic_param.flow_stat_offset[i];


		if(outq_bytes[i]<=(enq_bytes[i]+veth_priv->virt_traffic_param.flow_stat_offset[i]))
		{
			veth_priv->virt_traffic_param.flow_stat_offset[i] =veth_priv->virt_traffic_param.flow_stat_offset[i]+ \
					enq_bytes[i]-outq_bytes[i];
		}
		else
        {
			veth_priv->virt_traffic_param.flow_stat_offset[i] = 0;
		}


		if(outq_bytes[i] <= ((veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1[i] - sizeof(mac_header_cstdd)
				- sizeof(llc_header_t)) * (TS_V_COUNT)) && enq_bytes[i] <= ((veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1[i] - sizeof(mac_header_cstdd)
						- sizeof(llc_header_t)) * (TS_V_COUNT)))
		{
			veth_priv->virt_traffic_param.flow_stat_offset[i] = 0;
		}


//		pkt_outq_bytes_tmp[i]= veth_priv->virt_traffic_param.pkt_outq_bytes[i];

		ts_v[i] = 0;
	}

	for(i = 0; i < 8; i ++)
	{
		while (ts_all < MAX_SLOT) {

//			if(i == 2){
//				if (((veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1[i] - sizeof(mac_header_cstdd) - 4
//								- sizeof(llc_header_t)) * ((TS_V_COUNT*2- OGM_OCP) * ts_v[i])) < veth_priv->virt_traffic_param.pkt_enq_bytes_tmp[i]) {
//					ts_v[i]++;
//					ts_all ++;
//				}
//				else
//					break;
//			}else{
				if (((veth_priv->virt_traffic_param.TX_AMPDU_LEM_MAX_1[i] - sizeof(mac_header_cstdd)
								- sizeof(llc_header_t)) * ((TS_V_COUNT) * ts_v[i])) < veth_priv->virt_traffic_param.pkt_enq_bytes_tmp[i]) {
					ts_v[i]++;
					ts_all ++;
				}
				else
					break;
//			}
		}
		if(ts_v[i] == SLOT_TMP)
			ts_all -= SLOT_TMP;
	}



	if (veth_priv->virt_traffic_param.q_hffull_flag) {
		ts_all+=veth_priv->virt_traffic_param.q_hffull_flag;
	}
	veth_priv->virt_traffic_param.q_hffull_flag = 0;
	if(ts_all > MAX_SLOT)
		ts_all = MAX_SLOT;

	if(ts_all == SLOT_TMP){
		ts_all = 0;
	}
#ifdef Zynq_Platform
	slot_msg->slot_num = ts_all;
//	printk("%d\n",slot_msg->slot_num);

	rpmsg_send(local->ept, data,sizeof(data));
#endif
	virt_eth_mgmt_slot_schedule(veth_priv);

	kfree(work_slot_num);

}

static unsigned long virt_eth_mgmt_emit_send_time(struct virt_eth_priv *veth_priv)
{
	unsigned int msecs;

	msecs = atomic_read(&veth_priv->slot_num_time);

	return jiffies + msecs_to_jiffies(msecs);

}

void virt_eth_mgmt_slot_schedule(struct virt_eth_priv *veth_priv){
	unsigned long time_delay = virt_eth_mgmt_emit_send_time(veth_priv);

	virt_eth_work_slot_num *v_work_slot_num = kmalloc(sizeof(*v_work_slot_num), GFP_KERNEL);
	v_work_slot_num->dev = veth_priv->soft_dev;

	INIT_DELAYED_WORK(&v_work_slot_num->delayed_work,
			virt_eth_mgmt_tf_change_cal);
	queue_delayed_work(veth_priv->virt_mgmt_event_workqueue,
			   &v_work_slot_num->delayed_work,
			   time_delay - jiffies);
}

void virt_eth_mgmt_manage(struct work_struct *work){
	struct virt_eth_priv *veth_priv;
	virt_eth_work_manage* work_manage;
	struct delayed_work *delayed_work;
	struct _rpmsg_eptdev *local;
	u8 i = 0;
	u8 manage_type;


	work_manage = container_of(work, virt_eth_work_manage, worker);
	veth_priv = netdev_priv(work_manage->dev);

	manage_type = work_manage->manage_array[0];
	switch(manage_type){
	case MCS_MODE:
	{
		veth_priv->ucast_mcs = work_manage->manage_array[1];
		veth_priv->bcast_mcs = work_manage->manage_array[3];
		if(veth_priv->ucast_mcs < MCS_NUM)
			veth_priv->mcs_mode = FIX_MCS_MODE;
		else
			veth_priv->mcs_mode = NFIX_MCS_MODE;

//		printk("set mcs %d %d\n",veth_priv->mcs_mode,veth_priv->ucast_mcs);

		break;
	}
	case PHYDLY_SLOT:
	{
		break;
	}
	case TRX_FREQ:
	{
		break;
	}
	case CHANNEL_BW:
	{
		break;
	}
	case TXFIFO_DLY:
	{
		break;
	}
	case RTS_SW:
	{
		break;
	}
	case TX_ATTEN:
	{
		break;
	}
	case PRINT_EN:
	{
		break;
	}
	case ACK_MODE:
	{
		break;
	}
	case TEST_MODE:
	{
		break;
	}
	default:
	{
		break;
	}
	}


	kfree(work_manage);
}

int virt_eth_mgmt_get_id(void){
	int id = 8;
	return id;
}
EXPORT_SYMBOL(virt_eth_mgmt_get_id);




