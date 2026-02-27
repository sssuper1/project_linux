/*
 * virt_eth_system.c
 *
 *  Created on: 2020-4-20
 *      Author: lu
 */


#include "virt_eth_system.h"
#include "virt_eth_dma.h"
#include "virt_eth_util.h"
#include "virt_eth_queue.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_jgk.h"
#include "virt_eth_station.h"

u8 virt_eth_system_init(struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
#ifdef Docker_Qualnet
	INIT_LIST_HEAD(&veth_priv->virt_eth_hardif_list);
#endif
	INIT_HLIST_HEAD(&veth_priv->station_list);
	spin_lock_init(&veth_priv->station_lock);
	spin_lock_init(&veth_priv->dma_rx_lock);
	spin_lock_init(&veth_priv->dma_tx_lock);
	veth_priv->soft_dev = dev;
    veth_priv->ucds_mode = UCDS_INIT_MODE;
    veth_priv->mcs_mode = NFIX_MCS_MODE;
    veth_priv->ucast_mcs = UNICAST_MCS_DEFAULT;
    veth_priv->bcast_mcs = MULTICAST_MCS_DEFAULT;//MULTICAST_MCS_DEFAULT
#ifdef Radio_220
    veth_priv->channel_num = 0;
#endif

//	veth_priv->v_work_tx.dev = dev;
//	veth_priv->v_work_rx.dev = dev;
//	veth_priv->v_work_rx.veth_rx_param.rx_pkt_buf = 0;
//	veth_priv->v_work_rx.veth_rx_param.resv = 0;
//	veth_priv->v_work_rx.veth_rx_param.buf_len = 0;
//	veth_priv->v_work_slot_num.dev = dev;

	atomic_set(&veth_priv->slot_num_time, 1000);
	memcpy(veth_priv->addr,dev->dev_addr,MAC_ADDR_LEN);
	INIT_DELAYED_WORK(&veth_priv->station_work,
			virt_eth_station_timeout);
	INIT_DELAYED_WORK(&veth_priv->queue_work,
			virt_eth_rx_queue_timeout);



	return 0;
}
#ifdef Docker_Qualnet
void virt_eth_system_set_priv(struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
#elif defined Zynq_Platform
void virt_eth_system_set_priv(struct net_device *dev,struct rpmsg_device *rpmsg_dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	veth_priv->rpmsg_dev = rpmsg_dev;
#endif
#if VIRT_ETH_TEST
#else

	virt_eth_mgmt_slot_schedule(veth_priv);


#ifndef Radio_SWARM_WNW
	virt_eth_jgk_schedule(veth_priv);
#endif
	virt_eth_station_schedule(veth_priv);
#endif

}

static void virt_eth_cdma_rx_process(struct work_struct *work){
#ifdef Zynq_Platform
	virt_eth_work_rx_poll    * v_work_rx;
	struct virt_eth_priv *veth_priv = NULL;
	u8* data_buf;
	int ret;
	int rx_index = 0;
#if VIRT_ETH_TEST
	virt_eth_pkt_ready tx_pkt_ready;
	tx_pkt_ready.rx_pkt_buf = 0;
	tx_pkt_ready.resv = 2;
	tx_pkt_ready.buf_len = 0;
#endif
//	printk("virt_eth_cdma_rx_process \n");
	v_work_rx = container_of(work, virt_eth_work_rx_poll, worker);
	veth_priv = netdev_priv(v_work_rx->dev);
	rx_index = v_work_rx->veth_rx_param.rx_pkt_buf;
	data_buf = virt_eth_dma_get_rx_buffer(rx_index);
	if(data_buf != NULL){
#if VIRT_ETH_TEST
		ret = virt_eth_dma_rx((dma_addr_t)(TX_PKT_BUF(rx_index)),rx_index,v_work_rx->veth_rx_param.buf_len + sizeof(tx_frame_info_t)+PHY_TX_PKT_BUF_PHY_HDR_SIZE);
		virt_eth_mgmt_send_msg(v_work_rx->dev,MGMT_TX_READY,(u8*)&tx_pkt_ready,sizeof(virt_eth_pkt_ready));
#else
//		ret = virt_eth_dma_rx((dma_addr_t)(RX_PKT_BUF(rx_index)),rx_index,v_work_rx->veth_rx_param.buf_len + sizeof(rx_frame_info_t)+PHY_RX_PKT_BUF_PHY_HDR_SIZE);
#endif

		if(NUM_RX_PKT_IQ == rx_index)
		{
			virt_eth_jgk_iq_schedule(veth_priv,data_buf,0);
		}
		else
		{
			if(ret == 0 && v_work_rx->veth_rx_param.buf_len > 1000){
				veth_priv->v_jgk_info.rx_inall ++;
			}else{
		//		v_jgk_info.rx_in_lose ++;
			}
		//	spin_lock_bh(&veth_priv->dma_rx_lock);
			virt_eth_util_rx_process(v_work_rx->dev,data_buf);
		//	spin_unlock_bh(&veth_priv->dma_rx_lock);
			memset(data_buf,0,v_work_rx->veth_rx_param.buf_len + sizeof(rx_frame_info_t)+PHY_RX_PKT_BUF_PHY_HDR_SIZE);
		}
	}
	kfree(v_work_rx);
#endif

}

u8 virt_eth_system_rx_data_process(struct net_device *dev,u8* data,u32 len){
	int ret;
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	virt_eth_pkt_ready* v_rx = (virt_eth_pkt_ready*)data;
	virt_eth_work_rx_poll* v_work_rx = kmalloc(sizeof(*v_work_rx), GFP_KERNEL);


	v_work_rx->veth_rx_param.rx_pkt_buf = v_rx->rx_pkt_buf;
	v_work_rx->veth_rx_param.buf_len = v_rx->buf_len;
	v_work_rx->dev = veth_priv->soft_dev;
	v_work_rx->veth_rx_param.resv = 0;

//	printk("Virt-eth0:virt_eth_system_rx_data_process %d\n",v_work_rx->veth_rx_param.rx_pkt_buf);

	if(v_rx->rx_pkt_buf >= NUM_RX_PKT_BUF+1){
		return 1;
	}

	INIT_WORK(&(v_work_rx->worker), virt_eth_cdma_rx_process);
	queue_work(veth_priv->virt_data_event_workqueue,&(v_work_rx->worker));
	//schedule_work(&(v_work_rx->worker));
	
	return 0;
}

void virt_eth_system_tx_done(struct net_device *dev){
	struct virt_eth_priv *veth_priv = netdev_priv(dev);
	virt_eth_work_tx_poll* v_work_tx = kmalloc(sizeof(*v_work_tx), GFP_KERNEL);
	v_work_tx->dev = dev;
#ifdef Zynq_Platform
	INIT_WORK(&(v_work_tx->worker), virt_eth_poll_tx_queue);
	queue_work(veth_priv->virt_data_event_workqueue,&(v_work_tx->worker));
	//schedule_work(&(v_work_tx->worker));
#endif
}

void virt_eth_system_param_set_report(void){

}
