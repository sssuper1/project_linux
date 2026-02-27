/*
 * virt_eth_dma.c
 *
 *  Created on: 2020-4-29
 *      Author: lu
 */

#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/of_dma.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/sched/task.h>
#include <linux/dma/xilinx_dma.h>
#include "virt_eth_dma.h"
#include "virt_eth_types.h"



static struct dma_chan *chan;


static dma_addr_t tx_dma_addr[NUM_TX_PKT_BUF];
dma_addr_t rx_dma_addr[NUM_RX_PKT_BUF+1];
static u8* tx_dma_buffer[NUM_TX_PKT_BUF];
static u8* rx_dma_buffer[NUM_RX_PKT_BUF+1];
//static int rx_dma_index;

//dma_addr_t slot_tb_dma_addr[SLOT_CYCLE];
//static u8* slot_tb_dma_buffer[SLOT_CYCLE];


struct dma_chan* virt_eth_dma_get_chan(void){
	return chan;
}

u8* virt_eth_dma_get_tx_buffer(int index){
	if(index >= NUM_TX_PKT_BUF)
		return NULL;
	return tx_dma_buffer[index];
}

//int virt_eth_dma_get_rx_index(void){
//	int index = rx_dma_index;
//	rx_dma_index ++;
//	if(rx_dma_index >= NUM_TX_PKT_BUF)
//		rx_dma_index = 0;
//	return index;
//}

u8* virt_eth_dma_get_rx_buffer(int index){
	if(index >= NUM_RX_PKT_BUF + 1)
		return NULL;
	return rx_dma_buffer[index];
}

//u8* virt_eth_dma_get_slot_tb_buffer(int index){
//	if(index >= SLOT_CYCLE)
//		return NULL;
//	return rx_dma_buffer[index];
//}


u8 virt_eth_dma_create_tx_buf(void){
	struct dma_device *dev = chan->device;
	//printk("device tx 0x%p 0x%p %d\n",(void*)dev,(void*)dev->dev,PKG_SIZE);
	u8 i = 0;
	for(;i < NUM_TX_PKT_BUF; i ++)
		tx_dma_buffer[i] = dma_alloc_coherent(dev->dev, PKG_SIZE, &tx_dma_addr[i], GFP_KERNEL);

//	printk("virt_eth_dma_create_buf tx dma_addr_t 0x%p 0x%p\n",(void*)tx_dma_addr[0],(void*)tx_dma_buffer[0]);
	return 0;
}

u8 virt_eth_dma_create_rx_buf(void){
	struct dma_device *dev = chan->device;
	//printk("device rx 0x%p 0x%p %d\n",(void*)dev,(void*)dev->dev,PKG_SIZE);
	u8 i = 0;
	for(;i < NUM_RX_PKT_BUF; i ++)
		rx_dma_buffer[i] = dma_alloc_coherent(dev->dev, PKG_SIZE, &rx_dma_addr[i], GFP_KERNEL);

	rx_dma_buffer[NUM_RX_PKT_IQ] = dma_alloc_coherent(dev->dev, IQ_BUF_LEN, &rx_dma_addr[NUM_RX_PKT_IQ], GFP_KERNEL);

//	printk("virt_eth_dma_create_buf rx dma_addr_t 0x%p 0x%p\n",(void*)rx_dma_addr[0],(void*)rx_dma_buffer[0]);
	return 0;
}

void virt_eth_dma_free_tx_buf(void){
	struct dma_device *dev = chan->device;
	u8 i = 0;
	for(;i < NUM_TX_PKT_BUF; i ++)
		dma_free_coherent(dev->dev, PKG_SIZE,(void*)tx_dma_buffer[i], tx_dma_addr[i]);
}

void virt_eth_dma_free_rx_buf(void){
	struct dma_device *dev = chan->device;
	u8 i = 0;
	for(;i < NUM_RX_PKT_BUF+1; i ++)
		dma_free_coherent(dev->dev, PKG_SIZE,(void*)rx_dma_buffer[i], rx_dma_addr[i]);
}

//u8 virt_eth_slot_create_slot_tb_buf(void){
//
//	struct dma_device *dev = chan->device;
//	//printk("device tx 0x%p 0x%p %d\n",(void*)dev,(void*)dev->dev,PKG_SIZE);
//	u8 i = 0;
//	for(i=0;i < SLOT_CYCLE; i ++){
//	    slot_tb_dma_buffer[i] = dma_alloc_coherent(dev->dev, TABLE_PART_SIZE, &slot_tb_dma_addr[i], GFP_KERNEL);
//
//	}
//
//
//	printk("virt_eth_slot_create_slot_tb_buf slot dma_addr_t 0x%p 0x%p\n",(void*)slot_tb_dma_addr[0],(void*)slot_tb_dma_buffer[0]);
//	return 0;
//}

//void virt_eth_slot_free_slot_tb_buf(void){
//
//	struct dma_device *dev = chan->device;
//	u8 i = 0;
//	for(;i < SLOT_CYCLE; i ++)
//		dma_free_coherent(dev->dev, TABLE_PART_SIZE,(void*)slot_tb_dma_buffer[i], slot_tb_dma_addr[i]);
//}

//static void virt_eth_slot_slot_tb_callback(void *completion)
//{
//	complete(completion);
//}


static void virt_eth_dma_tx_callback(void *completion)
{
	complete(completion);
}

static void virt_eth_dma_rx_callback(void *completion)
{
	complete(completion);
}

//int virt_eth_slot_get_slot_tb(void){
//	struct dma_device *dev = chan->device;
//	struct dma_async_tx_descriptor *rx = NULL;
//	enum dma_ctrl_flags flags;
//	struct completion cmp;
//	enum dma_status status;
//	unsigned long tmo = msecs_to_jiffies(3000);
//	dma_cookie_t cookie;
//	dma_addr_t dma_srcs;
//	unsigned int len = sizeof(resource_table_info_t);
//	int i;
//
//	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
//
//	for(i=0;i<SLOT_CYCLE;i++){
//
//	    dma_srcs =  (dma_addr_t)(RESOURCE_TABLE_TO_ADDR(i));
//		rx = dev->device_prep_dma_memcpy(chan,slot_tb_dma_addr[i],dma_srcs, len,flags);
//	    printk("dma_phy 0x%p %d\n",(void*)rx->phys,len);
//	}
//
//
//	
//	//printk("dma_phy 0x%p %d %d\n",(void*)rx->phys,dst_off,len);
//
//	init_completion(&cmp);
//	rx->callback = virt_eth_slot_slot_tb_callback;
//	rx->callback_param = &cmp;
//	cookie = rx->tx_submit(rx);
//
//	if (dma_submit_error(cookie)) {
//		return -1;
//	}
//	dma_async_issue_pending(chan);
//
//	tmo = wait_for_completion_timeout(&cmp, tmo);
//	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
//
//	if (tmo == 0) {
//		return -1;
//	} else if (status != DMA_COMPLETE) {
//		return -1;
//	}
//
//	return 0;
//}



int virt_eth_dma_tx(dma_addr_t dma_dsts,int tx_index,unsigned int len){
	struct dma_device *dev = chan->device;
	struct dma_async_tx_descriptor *tx = NULL;
	enum dma_ctrl_flags flags;
	struct completion cmp;
	enum dma_status status;
	unsigned long tmo = msecs_to_jiffies(3000);
	dma_cookie_t cookie;

	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;

//	if(len > 1600)
//		printk("len %d\n",len);
	tx = dev->device_prep_dma_memcpy(chan,dma_dsts,tx_dma_addr[tx_index], len,flags);
//	//printk("dma_phy 0x%p %d\n",(void*)tx->phys,len);

	init_completion(&cmp);
	tx->callback = virt_eth_dma_tx_callback;
	tx->callback_param = &cmp;
	cookie = tx->tx_submit(tx);

	if (dma_submit_error(cookie)) {
		return -1;
	}
	dma_async_issue_pending(chan);

//	//printk("dma_async_issue_pending done\n");

	tmo = wait_for_completion_timeout(&cmp, tmo);
	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

	if (tmo == 0) {
		return -1;
	} else if (status != DMA_COMPLETE) {
		return -1;
	}

	return 0;
}

int virt_eth_dma_rx(dma_addr_t dma_srcs,int rx_index,unsigned int len){
	struct dma_device *dev = chan->device;
	struct dma_async_tx_descriptor *rx = NULL;
	enum dma_ctrl_flags flags;
	struct completion cmp;
	enum dma_status status;
	unsigned long tmo = msecs_to_jiffies(3000);
	dma_cookie_t cookie;

	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;


	rx = dev->device_prep_dma_memcpy(chan,rx_dma_addr[rx_index],dma_srcs, len,flags);
//	//printk("dma_phy 0x%p %d %d\n",(void*)tx->phys,dst_off,len);

	init_completion(&cmp);
	rx->callback = virt_eth_dma_rx_callback;
	rx->callback_param = &cmp;
	cookie = rx->tx_submit(rx);

	if (dma_submit_error(cookie)) {
		return -1;
	}
	dma_async_issue_pending(chan);

	tmo = wait_for_completion_timeout(&cmp, tmo);
	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);

	if (tmo == 0) {
		return -1;
	} else if (status != DMA_COMPLETE) {
		return -1;
	}

	return 0;
}


static int virt_eth_dma_probe(struct platform_device *pdev)
{
	int err = 0;

	//printk("xilinx_cdmatest_probe\n");
	chan = dma_request_slave_channel(&pdev->dev, "cdma");
	if (IS_ERR(chan)) {
		//printk("xilinx_cdmatest: No channel\n");
		return PTR_ERR(chan);
	}
	virt_eth_dma_create_tx_buf();

	virt_eth_dma_create_rx_buf();

	//virt_eth_slot_create_slot_tb_buf();

	return err;
}

static int virt_eth_dma_remove(struct platform_device *pdev)
{
	virt_eth_dma_free_tx_buf();
	virt_eth_dma_free_rx_buf();
	//virt_eth_slot_free_slot_tb_buf();
	dma_release_channel(chan);
	return 0;
}

static const struct of_device_id xilinx_cdmatest_of_ids[] = {
	{ .compatible = "xlnx,axi-cdma-test-1.00.a", },
	{}
};

static struct platform_driver xilinx_cdmatest_driver = {
	.driver = {
		.name = "xilinx_cdmatest1",
		.owner = THIS_MODULE,
		.of_match_table = xilinx_cdmatest_of_ids,
	},
	.probe = virt_eth_dma_probe,
	.remove = virt_eth_dma_remove,
};

int virt_eth_dma_init(void)
{
//printk("cdma_init\n");
//	rx_dma_index = 0;
	return platform_driver_register(&xilinx_cdmatest_driver);
}

void virt_eth_dma_exit(void)
{
	platform_driver_unregister(&xilinx_cdmatest_driver);
}
