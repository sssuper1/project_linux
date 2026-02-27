#include "mgmt_netlink.h"
#include "mgmt_types.h"

#include <linux/genetlink.h>

#include <linux/atomic.h>
#include <linux/byteorder/generic.h>
#include <linux/cache.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/genetlink.h>
#include <linux/if_ether.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/printk.h>
#include <linux/rculist.h>
#include <linux/rcupdate.h>
#include <linux/skbuff.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include <net/sock.h>
#include "mgmt_module.h"

struct batadv_jgk_node bat_jgk_node_msg;
struct routetable routet_msg;
jgk_report_infor jgk_information_data_msg;

struct genl_family mgmt_netlink_family;
u8 version[4]={0,0,9,2};

/* multicast groups */
enum mgmt_netlink_multicast_groups {
	MGMT_NL_MCGRP_TPMETER,
};


static const struct genl_multicast_group mgmt_netlink_mcgrps[] = {
	[MGMT_NL_MCGRP_TPMETER] = { .name = MGMT_NL_MCAST_GROUP_TPMETER },
};


static const struct nla_policy mgmt_netlink_policy[NUM_MGMT_ATTR] = {
	[MGMT_ATTR_GET_INFO]		  = { .type = NLA_U32 },
	[MGMT_ATTR_SET_PARAM]		  = { .type = NLA_U32 },
	[MGMT_ATTR_NODEID]		      = { .type = NLA_U8 },
	[MGMT_ATTR_VERSION_ROUTE]	  = { .type = NLA_STRING },
	[MGMT_ATTR_PKTNUMB_ROUTE]	  = {  .len = sizeof(struct batadv_jgk_node)  },
	[MGMT_ATTR_TABLE_ROUTE]	      = { .len = sizeof(struct routetable)},
	[MGMT_ATTR_UCDS_ROUTE]	      = { .type = NLA_STRING },

	[MGMT_ATTR_VERSION_VETH]	  = { .type = NLA_STRING },
	[MGMT_ATTR_INFO_VETH]	      = { .len = sizeof(jgk_report_infor) },
//	[MGMT_ATTR_INFO_VETH2]	      = { .len = sizeof(jgk_report_infor12) },


	[MGMT_ATTR_VETH_ADDRESS]	  = { .len = ETH_ALEN },
	[MGMT_ATTR_VETH_TX]	          = { .type = NLA_U32 },
	[MGMT_ATTR_VETH_RX]	          = { .type = NLA_U32 },
	[MGMT_ATTR_VERSION_MAC_PHY]	  = { .type = NLA_STRING },
	[MGMT_ATTR_FREQ]	          = { .type = NLA_U32 },
	[MGMT_ATTR_BW]	              = { .type = NLA_U16 },
	[MGMT_ATTR_TXPOWER]	          = { .type = NLA_S16 },
	[MGMT_ATTR_SET_NODEID]		  = { .type = NLA_U16 },
	[MGMT_ATTR_SET_INTERVAL]	  = { .type = NLA_U16 },
	[MGMT_ATTR_SET_TTL]	          = { .type = NLA_U16 },
	[MGMT_ATTR_SET_QUEUE_NUM]	  = { .type = NLA_U16 },
	[MGMT_ATTR_SET_QUEUE_LENGTH]  = { .type = NLA_U16 },
	[MGMT_ATTR_SET_QOS_STATEGY]	  = { .type = NLA_U16 },
	[MGMT_ATTR_SET_UNICAST_MCS]	  = { .type = NLA_U8 },
	[MGMT_ATTR_SET_MULTICAST_MCS] = { .type = NLA_U8 },
	[MGMT_ATTR_SET_FREQUENCY]	  = { .type = NLA_U32 },
	[MGMT_ATTR_SET_POWER]	      = { .type = NLA_BINARY },
	[MGMT_ATTR_SET_BANDWIDTH]	  = { .type = NLA_U8 },
	[MGMT_ATTR_SET_TEST_MODE]	  = { .type = NLA_U16 },
	[MGMT_ATTR_SET_TEST_MODE_MCS] = { .type = NLA_U16 },
	[MGMT_ATTR_SET_PHY]		      = { .type = NLA_STRING},
	[MGMT_ATTR_SET_WORKMODE] = {.type = NLA_STRING },
	[MGMT_ATTR_SET_IQ_CATCH] = {.type = NLA_STRING },
	[MGMT_ATTR_SET_SLOTLEN] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_POWER_LEVEL] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_POWER_ATTENUATION] = {.type = NLA_U8 },
	[MGMT_ATTR_SET_RX_CHANNEL_MODE] = {.type = NLA_U8 },
};

static int
mgmt_netlink_route_info_put(struct sk_buff *msg)
{
	struct net_device *hard_iface;
	int ret = -ENOBUFS;

	struct batadv_jgk_node bat_jgk_node;
	struct routetable routet;
	jgk_report_infor* jgk_information_data;
	jgk_report_infor jgk_information_data1;
	u8* veth_jgk_data;
	int batparamtype = 0;
	struct route_para r_para;
	jgk_set_parameter veth_jgk_set;

	/////////////////get
	extern struct batadv_jgk_node batman_get_mgmt_pktnumb(void);

	extern struct routetable batman_get_mgmt_routetable(void);

	bat_jgk_node = batman_get_mgmt_pktnumb();
//	printk("mgmt_agent bat_jgk_node nodeid: %d, s_ogm: %d, r_ogm: %d, f_ogm: %d, s_bcast: %d, r_bcast: %d, f_bcast: %d", bat_jgk_node.nodeid,
//		bat_jgk_node.s_ogm, bat_jgk_node.r_ogm, bat_jgk_node.f_ogm, bat_jgk_node.s_bcast, bat_jgk_node.r_bcast, bat_jgk_node.f_bcast);
	routet = batman_get_mgmt_routetable();
	memcpy((void*)&bat_jgk_node_msg,(void*)&bat_jgk_node,sizeof(struct batadv_jgk_node));
	memcpy((void*)&routet_msg,(void*)&routet,sizeof(struct routetable));


	nla_put(msg,MGMT_ATTR_PKTNUMB_ROUTE,sizeof(struct batadv_jgk_node),(char*)&bat_jgk_node);
//	printk("batadv_jgk_node %d\n",sizeof(struct batadv_jgk_node));

	nla_put(msg,MGMT_ATTR_TABLE_ROUTE,sizeof(struct routetable),(char*)&routet);
//	printk("routetable %d\n",sizeof(struct routetable));

	ret = 0;
//	printk("return ret = %d msg %p\n",ret,msg);

	return ret;
}

static int
mgmt_netlink_get_route_info(struct sk_buff *skb, struct genl_info *info)
{
	struct net *net = genl_info_net(info);
	struct sk_buff *msg = NULL;
	void *msg_head;
	int ifindex;
	int ret=0;
	struct batadv_jgk_node bat_jgk_node;
	struct routetable routet;
	extern struct batadv_jgk_node batman_get_mgmt_pktnumb(void);

	extern struct routetable batman_get_mgmt_routetable(void);

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg) {
		ret = -ENOMEM;
		goto out;
	}

	msg_head = genlmsg_put(msg, info->snd_portid, info->snd_seq,
			       &mgmt_netlink_family, 0,
			       MGMT_CMD_GET_ROUTE_INFO);
	if (!msg_head) {
		ret = -ENOBUFS;
		goto out;
	}

	bat_jgk_node = batman_get_mgmt_pktnumb();
//	printk("mgmt_agent bat_jgk_node nodeid: %d, s_ogm: %d, r_ogm: %d, f_ogm: %d, s_bcast: %d, r_bcast: %d, f_bcast: %d", bat_jgk_node.nodeid,
//		bat_jgk_node.s_ogm, bat_jgk_node.r_ogm, bat_jgk_node.f_ogm, bat_jgk_node.s_bcast, bat_jgk_node.r_bcast, bat_jgk_node.f_bcast);
	routet = batman_get_mgmt_routetable();
//	memcpy((void*)&bat_jgk_node_msg,(void*)&bat_jgk_node,sizeof(struct batadv_jgk_node));
//	memcpy((void*)&routet_msg,(void*)&routet,sizeof(struct routetable));


	nla_put(msg,MGMT_ATTR_PKTNUMB_ROUTE,sizeof(struct batadv_jgk_node),(char*)&bat_jgk_node);
//	printk("batadv_jgk_node %d\n",sizeof(struct batadv_jgk_node));

	nla_put(msg,MGMT_ATTR_TABLE_ROUTE,sizeof(struct routetable),(char*)&routet);
//	printk("routetable %d\n",sizeof(struct routetable));


	//ret = mgmt_netlink_route_info_put(msg);
//    printk("mgmt_netlink_route_info_put end ret = %d\n",ret);

 out:
	if (ret) {
		if (msg)
			nlmsg_free(msg);
		return ret;
	}
//	printk("genlmsg_end\n");

	genlmsg_end(msg, msg_head);
//	printk("genlmsg_reply\n");
	return genlmsg_reply(msg, info);
}

static int
mgmt_netlink_veth_info_put(uint8_t id,struct sk_buff *msg)
{
	struct net_device *hard_iface;
	int ret = -ENOBUFS;

	struct batadv_jgk_node bat_jgk_node;
	struct routetable routet;
	jgk_report_infor* jgk_information_data;
	jgk_report_infor jgk_information_data1;
	u8* veth_jgk_data;
	int batparamtype = 0;
	struct route_para r_para;
	jgk_set_parameter veth_jgk_set;

//	uint32_t  enqueue_bytes[MCS_NUM];
//	enqueue_bytes[0] = 20;

//	jgk_report_infor11 test;
//	test.enqueue_bytes[0] = 12;
//	jgk_report_infor12 test2;
//	test2.traffic_queue_information.ucds[0] = 100;
//	struct routetable1 routet1;
//	char type = 2;
//	int  value;
//	extern int get_value(char);
//    value = get_value(type);
//	printk("mgmt_module value = %d\n",value);
//
//	if(nla_put_u32(msg, MGMT_ATTR_MESH_IFINDEX, value)){
//		goto out;
//	}

	//get
//	mgmt_info->macaddr[0] = 0xb8;
//	mgmt_info->macaddr[1] = 0x8e;
//	mgmt_info->macaddr[2] = 0xdf;
//	mgmt_info->macaddr[3] = 0x00;
//	mgmt_info->macaddr[4] = 0x01;
//	mgmt_info->macaddr[5] = 0x01;

	/////////////////get
//	extern struct batadv_jgk_node batman_get_mgmt_pktnumb(void);
//
//	extern struct routetable batman_get_mgmt_routetable(void);

	extern u8 * virt_eth_jgk_info_get(uint8_t node_id);

//	bat_jgk_node = batman_get_mgmt_pktnumb();
//	printk("mgmt_agent bat_jgk_node nodeid: %d, s_ogm: %d, r_ogm: %d, f_ogm: %d, s_bcast: %d, r_bcast: %d, f_bcast: %d", bat_jgk_node.nodeid,
//		bat_jgk_node.s_ogm, bat_jgk_node.r_ogm, bat_jgk_node.f_ogm, bat_jgk_node.s_bcast, bat_jgk_node.r_bcast, bat_jgk_node.f_bcast);
//	routet = batman_get_mgmt_routetable();
//	printk("routet agent %d\n",routet.bat_jgk_route[NODE_MAX-1].route);
	veth_jgk_data = virt_eth_jgk_info_get(id);
	jgk_information_data = (jgk_report_infor*)veth_jgk_data;
	memcpy(jgk_information_data->agent_version,version,sizeof(version));
	memcpy((void*)&jgk_information_data_msg,(void*)jgk_information_data,sizeof(jgk_report_infor));

	//printk("[MGMT_AGENT] amp info: temper:%d,power_ac220_power:%d,freq_12v_voltage:%d,freq_lo1_freq:%d\r\n",jgk_information_data->amp_infomation.power_temperature,jgk_information_data->amp_infomation.power_ac220_power,
	//	jgk_information_data->amp_infomation.freq_12v_voltage,jgk_information_data->amp_infomation.freq_lo1_freq);

//	printk("mgmt_agent jgk_information_data enqueue_bytes %d outqueue_bytes %d tx_inall %d tx_outall %d rx_inall %d rx_outall %d\n",
//			jgk_information_data->enqueue_bytes[0],jgk_information_data->outqueue_bytes[0],
//			jgk_information_data->traffic_queue_information.tx_inall,jgk_information_data->traffic_queue_information.tx_outall,
//			jgk_information_data->traffic_queue_information.rx_inall,jgk_information_data->traffic_queue_information.rx_outall);

//	mgmt_info->freq = get_mgmt_freq();
//
//	nla_put_u16(msg,MGMT_ATTR_NODEID,mgmt_info->id);
//	nla_put(msg,MGMT_ATTR_VETH_ADDRESS,ETH_ALEN,mgmt_info->macaddr);
//	nla_put_u32(msg,MGMT_ATTR_VETH_TX,mgmt_info->txrate);
//	nla_put_u32(msg,MGMT_ATTR_VETH_RX,mgmt_info->rxrate);
//	nla_put_u32(msg,MGMT_ATTR_FREQ,mgmt_info->freq);
//	nla_put_u16(msg,MGMT_ATTR_BW,mgmt_info->bw);
//	nla_put_s16(msg,MGMT_ATTR_TXPOWER,mgmt_info->txpower);



	/////////////set
//	extern void batman_set_mgmt_para(int type, struct route_para para_set);
//	batparamtype |= 0x01;
//	batparamtype |= 0x02;
//	r_para.interval = 2000;
//	r_para.ttl = 10;
//	batman_set_mgmt_para(batparamtype,r_para);
//
//	extern void virt_eth_jgk_param_set(jgk_set_parameter * parameter_data);
//	veth_jgk_set.indication = 0xffffffff;
//	veth_jgk_set.enqueue_num = 1;
//	veth_jgk_set.queue_length = 2;
//	veth_jgk_set.QoS_Stategy = 3;
//	veth_jgk_set.unicast_msc = 4;
//	veth_jgk_set.multicast_msc = 5;
//	veth_jgk_set.frequency = 6;
//	veth_jgk_set.power = 7;
//	veth_jgk_set.bandwidth = 8;
//	veth_jgk_set.test_send_mode = 9;
//	veth_jgk_set.test_send_mode_mcs = 10;
//	virt_eth_jgk_param_set(&veth_jgk_set);

//	nla_put(msg,MGMT_ATTR_PKTNUMB_ROUTE,sizeof(struct batadv_jgk_node),(char*)&bat_jgk_node);
//	printk("batadv_jgk_node %d\n",sizeof(struct batadv_jgk_node));

//	nla_put(msg,MGMT_ATTR_TABLE_ROUTE,sizeof(struct routetable1),(char*)&routet1);
//	printk("routetable %d\n",sizeof(struct routetable1));

	nla_put(msg,MGMT_ATTR_INFO_VETH,sizeof(jgk_report_infor),veth_jgk_data);

//	printk("jgk_report_infor11 %d\n",sizeof(jgk_report_infor));
//
//	nla_put(msg,MGMT_ATTR_INFO_VETH2,sizeof(jgk_report_infor12),(u8*)&test2);
//
//	printk("jgk_report_infor12 %d\n",sizeof(jgk_report_infor12));

//	nla_put_u16(msg,MGMT_ATTR_NODEID,5);
//	nla_put_u32(msg,MGMT_ATTR_VETH_TX,10);



	ret = 0;


 out:
	return ret;
}

static int
mgmt_netlink_get_veth_info(struct sk_buff *skb, struct genl_info *info)
{
	struct net *net = genl_info_net(info);
	struct sk_buff *msg = NULL;
	void *msg_head;
	int ifindex;
	int ret;
	uint8_t node_id = 0;

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg) {
		ret = -ENOMEM;
		goto out;
	}

	msg_head = genlmsg_put(msg, info->snd_portid, info->snd_seq,
			       &mgmt_netlink_family, 0,
			       MGMT_CMD_GET_VETH_INFO);
	if (!msg_head) {
		ret = -ENOBUFS;
		goto out;
	}

	node_id = nla_get_u8(info->attrs[MGMT_ATTR_NODEID]);
//	printk("mgmt_netlink_get_veth_info id = %d\n",node_id);

	ret = mgmt_netlink_veth_info_put(node_id,msg);

 out:
	if (ret) {
		if (msg)
			nlmsg_free(msg);
		return ret;
	}

	genlmsg_end(msg, msg_head);
	return genlmsg_reply(msg, info);
}

static int
mgmt_netlink_param_put(struct sk_buff *msg,int param,struct genl_info *info)
{
	int ret = -ENOBUFS;
	u32 nodeid = 0;
	u32 freq;
	u16 bw;
	s16 txpower;
	int routeparamdata = 0;
	int virtparamdata = 0;
	jgk_set_parameter virt_set_param;
	struct route_para route_set_param;
	u8 node_id = 0;
	u8 *phymsg = NULL;
	u8 * hf_data = NULL;
	u32  *pu8Param = NULL;
	u32 i = 0;
	memset((void*)&virt_set_param,0,sizeof(jgk_set_parameter));
	//
//	printk("param %d\n",param);

	if(param & MGMT_SET_ID){
//		paramdata |= SET_ID;
//		nla_put_u16(msg,MGMT_ATTR_SET_NODEID,ntohs(sparam->mgmt_id));
		node_id = nla_get_u8(info->attrs[MGMT_ATTR_NODEID]);
	}
	if(param & MGMT_SET_INTERVAL){
		routeparamdata |= ROUTE_SET_INTERVAL;
		route_set_param.interval = nla_get_u16(info->attrs[MGMT_ATTR_SET_INTERVAL]);
	}
	if(param & MGMT_SET_TTL){
		routeparamdata |= ROUTE_SET_TTL;
		route_set_param.ttl = nla_get_u16(info->attrs[MGMT_ATTR_SET_TTL]);
	}
	if(param & MGMT_SET_QUEUE_NUM){
		virt_set_param.indication |= VETH_SET_QUEUE_NUM;
		virt_set_param.enqueue_num = nla_get_u16(info->attrs[MGMT_ATTR_SET_QUEUE_NUM]);
	}
	if(param & MGMT_SET_QUEUE_LENGTH){
		virt_set_param.indication |= VETH_SET_QUEUE_LENGTH;
		virt_set_param.queue_length = nla_get_u16(info->attrs[MGMT_ATTR_SET_QUEUE_LENGTH]);
	}
	if(param & MGMT_SET_QOS_STATEGY){
		virt_set_param.indication |= VETH_SET_QOS_STATEGY;
		virt_set_param.QoS_Stategy = nla_get_u16(info->attrs[MGMT_ATTR_SET_QOS_STATEGY]);
	}
	if(param & MGMT_SET_UNICAST_MCS){
		virt_set_param.indication |= VETH_SET_UNICAST_MCS;
		virt_set_param.unicast_msc = nla_get_u8(info->attrs[MGMT_ATTR_SET_UNICAST_MCS]);
	}
	if(param & MGMT_SET_MULTICAST_MCS){
		virt_set_param.indication |= VETH_SET_MULTICAST_MCS;
		virt_set_param.multicast_msc = nla_get_u8(info->attrs[MGMT_ATTR_SET_MULTICAST_MCS]);
	}
	if(param & MGMT_SET_FREQUENCY){
		virt_set_param.indication |= VETH_SET_FREQUENCY;
		virt_set_param.frequency = nla_get_u32(info->attrs[MGMT_ATTR_SET_FREQUENCY]);
//		printk("agent set freq %d\n",virt_set_param.frequency);
	}
	if(param & MGMT_SET_POWER){
		virt_set_param.indication |= VETH_SET_POWER;
		memset(virt_set_param.power_ch, 0, sizeof(virt_set_param.power_ch));
		if (info->attrs[MGMT_ATTR_SET_POWER]) {
			uint16_t payload[POWER_CHANNEL_NUM] = {0};
			int payload_len = nla_len(info->attrs[MGMT_ATTR_SET_POWER]);
			int copy_len = min_t(int, payload_len, (int)sizeof(payload));
			int copied_cnt;
			memcpy(payload, nla_data(info->attrs[MGMT_ATTR_SET_POWER]), copy_len);
			copied_cnt = copy_len / sizeof(uint16_t);
			if (copied_cnt <= 0) {
				virt_set_param.power = 0;
				for (i = 0; i < POWER_CHANNEL_NUM; ++i)
					virt_set_param.power_ch[i] = 0;
			} else {
				virt_set_param.power = payload[0];
				for (i = 0; i < POWER_CHANNEL_NUM; ++i) {
					if (i < copied_cnt && payload[i])
						virt_set_param.power_ch[i] = payload[i];
					else
						virt_set_param.power_ch[i] = payload[0];
				}
			}
		} else {
			virt_set_param.power = 0;
		}
	}
	if(param & MGMT_SET_BANDWIDTH){
		virt_set_param.indication |= VETH_SET_BANDWIDTH;
		virt_set_param.bandwidth = nla_get_u8(info->attrs[MGMT_ATTR_SET_BANDWIDTH]);
		printk("agent set bw %d\n",virt_set_param.bandwidth);
	}
	if(param & MGMT_SET_TEST_MODE){
		virt_set_param.indication |= VETH_SET_TEST_MODE;
		virt_set_param.test_send_mode = nla_get_u16(info->attrs[MGMT_ATTR_SET_TEST_MODE]);
	}
	if(param & MGMT_SET_TEST_MODE_MCS){
		virt_set_param.indication |= VETH_SET_TEST_MODE_MCS;
		virt_set_param.test_send_mode_mcs = nla_get_u16(info->attrs[MGMT_ATTR_SET_TEST_MODE_MCS]);
	}
	if(param & MGMT_SET_WORKMODE){
		virt_set_param.indication |= VETH_SET_WORKMODE;
		pu8Param = nla_data(info->attrs[MGMT_ATTR_SET_WORKMODE]);
		memcpy((void*)&virt_set_param.work_mode_msg,pu8Param,sizeof(Smgmt_net_work_mode));
		printk("agent: set NET_work_mode %d\n",virt_set_param.work_mode_msg.NET_work_mode);
		if(virt_set_param.work_mode_msg.NET_work_mode == 2) // hop freq mode
		{

//			virt_set_param.hop_freq_tb = hop_freq[0];

//			printk("agent hop freq table freq %d \n",virt_set_param.hop_freq_tb);
			for(i=0;i<32;i++)
			{
				printk("%d\n",virt_set_param.work_mode_msg.hop_freq_tb[i]);
			}
			
		}
	}
	if(param & MGMT_SET_IQ_CATCH){
		virt_set_param.indication |= VETH_SET_IQ_CATCH;
		pu8Param = nla_data(info->attrs[MGMT_ATTR_SET_IQ_CATCH]);
		memcpy((void*)&virt_set_param.iq_catch,pu8Param,sizeof(Smgmt_iq_catch));
		printk("agent: set MGMT_SET_IQ_CATCH %d\n",virt_set_param.iq_catch.trig_mode);
	}
	if(param & MGMT_SET_SLOTLEN){
		virt_set_param.indication |= VETH_SET_SLOT_LEN;
		virt_set_param.u8Slotlen = nla_get_u8(info->attrs[MGMT_ATTR_SET_SLOTLEN]);
	}
	if(param & MGMT_SET_POWER_LEVEL){
		virt_set_param.indication |= VETH_SET_POWER_LEVEL;
		virt_set_param.power_level = nla_get_u8(info->attrs[MGMT_ATTR_SET_POWER_LEVEL]);
				printk("[mgmt_agent]set MGMT_SET_POWER_LEVEL\r\n");

	}
	if(param & MGMT_SET_POWER_ATTENUATION){
		virt_set_param.indication |= VETH_SET_POWER_ATTENUATION;
		virt_set_param.power_attenuation = nla_get_u8(info->attrs[MGMT_ATTR_SET_POWER_ATTENUATION]);
		printk("[mgmt_sgent]set MGMT_SET_POWER_ATTENUATION\r\n");
	}
	if(param & MGMT_SET_RX_CHANNEL_MODE){
		virt_set_param.indication |= VETH_SET_RX_CHANNEL_MODE;
		if (info->attrs[MGMT_ATTR_SET_RX_CHANNEL_MODE])
			virt_set_param.rx_channel_mode = nla_get_u8(info->attrs[MGMT_ATTR_SET_RX_CHANNEL_MODE]);
		else
			virt_set_param.rx_channel_mode = 0;
	}
#ifdef Radio_CEC
//	if(param & MGMT_SET_NCU){
//		
//		virt_set_param.indication |= VETH_SET_NCU;
//		virt_set_param.NCU_node_id = nla_get_u8(info->attrs[MGMT_ATTR_SET_NCU]);
//	    printk("agent set NCU %d\n",virt_set_param.NCU_node_id);
//	}

#endif
	if(param & MGMT_SET_PHY){
		virt_set_param.indication |= VETH_SET_PHY;
		pu8Param = nla_data(info->attrs[MGMT_ATTR_SET_PHY]);
		memcpy((void*)&virt_set_param.phy_msg,pu8Param,sizeof(Smgmt_phy));
	}


	if(routeparamdata != NO_FLAGS)
	{
		extern void batman_set_mgmt_para(int type, struct route_para para_set);
		batman_set_mgmt_para(routeparamdata,route_set_param);
	}


	if(virt_set_param.indication != NO_FLAGS){
		extern void virt_eth_jgk_param_set(uint8_t node_id,jgk_set_parameter* parameter_data);
		virt_eth_jgk_param_set(node_id,&virt_set_param);
	}

//	if(param & SET_ID){
//		nodeid = nla_get_u32(info->attrs[MGMT_ATTR_SET_NODEID]);
//		mgmt_info->id = nodeid;
//		printk("set_id %d\n",nodeid);
//		//set...
//	}
//	if(param & SET_FREQ){
//		freq = nla_get_u32(info->attrs[MGMT_ATTR_SET_FREQ]);
////		extern void set_mgmt_freq(uint32_t);
////		set_mgmt_freq(freq);
//		//mgmt_info->freq = freq;
//		printk("set_freq %d\n",freq);
//		//set...
//	}
//	if(param & SET_BW){
//		bw = nla_get_u16(info->attrs[MGMT_ATTR_SET_BW]);
//		mgmt_info->bw = bw;
//		printk("set_bw %d\n",bw);
//		//set...
//	}
//	if(param & SET_TXPOWER){
//		txpower = nla_get_s16(info->attrs[MGMT_ATTR_SET_TXPOWER]);
//		mgmt_info->txpower = txpower;
//		printk("set_txpower %d\n",txpower);
//		//set...
//	}

	ret = 0;

 out:
	return ret;
}


static int
mgmt_netlink_set_param(struct sk_buff *skb, struct genl_info *info)
{
	struct net *net = genl_info_net(info);
	struct sk_buff *msg = NULL;
	void *msg_head;
	int ifindex;
	int paramdata = NO_FLAGS;
	int ret;


//	printk("mgmt_netlink_set_param \n");

	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg) {
		ret = -ENOMEM;
		goto out;
	}

	msg_head = genlmsg_put(msg, info->snd_portid, info->snd_seq,
			       &mgmt_netlink_family, 0,
			       MGMT_CMD_SET_PARAM);
	if (!msg_head) {
		ret = -ENOBUFS;
		goto out;
	}

	paramdata = nla_get_u32(info->attrs[MGMT_ATTR_SET_PARAM]);

//	printk("module paramdata %d\n",paramdata);

	ret = mgmt_netlink_param_put(msg,paramdata,info);

 out:
	if (ret) {
		if (msg)
			nlmsg_free(msg);
		return ret;
	}

	genlmsg_end(msg, msg_head);
	return genlmsg_reply(msg, info);
}

static int
mgmt_netlink_tp_meter_start(struct sk_buff *skb, struct genl_info *info)
{
	struct net *net = genl_info_net(info);
	struct net_device *soft_iface;
	struct batadv_priv *bat_priv;
	struct sk_buff *msg = NULL;
	u32 test_length;
	void *msg_head;
	int ifindex;
	u32 cookie;
	u8 *dst;
	int ret;





	genlmsg_end(msg, msg_head);
	return genlmsg_reply(msg, info);
}


static const struct genl_ops mgmt_netlink_ops[] = {
	{
		.cmd = MGMT_CMD_GET_ROUTE_INFO,
		.flags = GENL_ADMIN_PERM,
		.policy = mgmt_netlink_policy,
		.doit = mgmt_netlink_get_route_info,
	},
	{
		.cmd = MGMT_CMD_GET_VETH_INFO,
		.flags = GENL_ADMIN_PERM,
		.policy = mgmt_netlink_policy,
		.doit = mgmt_netlink_get_veth_info,
	},
	{
		.cmd = MGMT_CMD_SET_PARAM,
		.flags = GENL_ADMIN_PERM,
		.policy = mgmt_netlink_policy,
		.doit = mgmt_netlink_set_param,
	},




};


struct genl_family mgmt_netlink_family __ro_after_init = {
	.hdrsize = 0,
	.name = MGMT_NL_NAME,
	.version = 1,
	.maxattr = MGMT_ATTR_MAX, //1
	.netnsok = true,
	.module = THIS_MODULE,
	.ops = mgmt_netlink_ops,
	.n_ops = ARRAY_SIZE(mgmt_netlink_ops),
	.mcgrps = mgmt_netlink_mcgrps,
	.n_mcgrps = ARRAY_SIZE(mgmt_netlink_mcgrps),
};


void mgmt_netlink_register(void){
	int ret;

	ret = genl_register_family(&mgmt_netlink_family);
	if (ret)
		pr_warn("unable to register netlink family");
	else{
		printk("mgmt_netlink_register ok\n");
	}
}

void mgmt_netlink_unregister(void){
	genl_unregister_family(&mgmt_netlink_family);
}

int mgmt_netlink_get_ifindex(const struct nlmsghdr *nlh, int attrtype){
	struct nlattr *attr = nlmsg_find_attr(nlh, GENL_HDRLEN, attrtype);

	return attr ? nla_get_u32(attr) : 0;
}
