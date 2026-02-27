/*
 * RPMSG User Device Kernel Driver
 *
 * Copyright (C) 2014 Mentor Graphics Corporation
 * Copyright (C) 2015 Xilinx, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/atomic.h>
#include <linux/skbuff.h>
#include <linux/idr.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <asm/irq.h>



#include "linux/mm.h"

#include "virt_eth_types.h"
#include "virt_eth_dma.h"
#include "virt_eth_system.h"
#include "virt_eth_queue.h"
#include "virt_eth_util.h"
#include "virt_eth_mgmt.h"
#include "virt_eth_station.h"
#include "virt_eth_comm_board.h"
#include "virt_eth_jgk.h"

#define RPMSG_KFIFO_SIZE		(MAX_RPMSG_BUFF_SIZE * 4)
#define PAYLOAD_MIN_SIZE	1
#define PAYLOAD_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 24)
/* Shutdown message ID */
#define SHUTDOWN_MSG			0xEF56A55A
#define RPMSG_USER_DEV_MAX_MINORS 10

#define RPMG_INIT_MSG "init_msg"


static void emulator_rx_packet(struct sk_buff *skb, struct net_device *dev);

//typedef struct {
//    u8                       dest_mac_addr[ETH_ADDR_SIZE];                      // Destination MAC address
//    u8                       src_mac_addr[ETH_ADDR_SIZE];                       // Source MAC address
//    u16                      ethertype;                                        // EtherType
//} ethernet_header_t;
//
//typedef struct {
//    u8                       version_ihl;                                      // [7:4] Version; [3:0] Internet Header Length
//    u8                       dscp_ecn;                                         // [7:2] Differentiated Services Code Point; [1:0] Explicit Congestion Notification
//    u16                      total_length;                                     // Total Length (includes header and data - in bytes)
//    u16                      identification;                                   // Identification
//    u16                      fragment_offset;                                  // [15:14] Flags;   [13:0] Fragment offset
//    u8                       ttl;                                              // Time To Live
//    u8                       protocol;                                         // Protocol
//    u16                      header_checksum;                                  // IP header checksum
//    u32                      src_ip_addr;                                      // Source IP address (big endian)
//    u32                      dest_ip_addr;                                     // Destination IP address (big endian)
//} ipv4_header_t;
//
//
//
//typedef struct {
//    u16                      src_port;                                         // Source port number
//    u16                      dest_port;                                        // Destination port number
//    u16                      length;                                           // Length of UDP header and UDP data (in bytes)
//    u16                      checksum;                                         // Checksum
//} udp_header_t;

/*
u16 ipCksum(void *ip, int len) //因为首部长度固定，所以使用时len为定值20
{
    u16 *buf = (u16*)ip; //每次取16位
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
}*/

static struct net_device *vnet_dev;
static struct rpmsg_device *Rpmsg_dev;

struct _payload {
	unsigned int num;
	unsigned int size;
	unsigned char data[];
};

static struct class *rpmsg_class;
static dev_t rpmsg_dev_major;
static DEFINE_IDA(rpmsg_minor_ida);



#define dev_to_eptdev(dev) container_of(dev, struct _rpmsg_eptdev, dev)
#define cdev_to_eptdev(i_cdev) container_of(i_cdev, struct _rpmsg_eptdev, cdev)

static int rpmsg_dev_open(struct inode *inode, struct file *filp)
{
	/* Initialize rpmsg instance with device params from inode */
	struct _rpmsg_eptdev *local = cdev_to_eptdev(inode->i_cdev);
	struct rpmsg_device *rpdev = local->rpdev;
	unsigned long flags;

	filp->private_data = local;

	spin_lock_irqsave(&local->queue_lock, flags);
	local->is_sk_queue_closed = false;
	spin_unlock_irqrestore(&local->queue_lock, flags);

	if (rpmsg_sendto(rpdev->ept,
			RPMG_INIT_MSG,
			sizeof(RPMG_INIT_MSG),
			rpdev->dst)) {
		dev_err(&rpdev->dev,
			"Failed to send init_msg to target 0x%x.",
			rpdev->dst);
		return -ENODEV;
	}
	dev_info(&rpdev->dev, "Sent init_msg to target 0x%x.", rpdev->dst);
	return 0;
}

static ssize_t rpmsg_dev_write(struct file *filp,
				const char __user *ubuff, size_t len,
				loff_t *p_off)
{
	struct _rpmsg_eptdev *local = filp->private_data;
	void *kbuf;
	int ret;

	kbuf = kzalloc(len, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, ubuff, len)) {
		ret = -EFAULT;
		goto free_kbuf;
	}

	if (filp->f_flags & O_NONBLOCK)
		ret = rpmsg_trysend(local->ept, kbuf, len);
	else
		ret = rpmsg_send(local->ept, kbuf, len);

free_kbuf:
	kfree(kbuf);
	return ret < 0 ? ret : len;
}

static ssize_t rpmsg_dev_read(struct file *filp, char __user *ubuff,
				size_t len, loff_t *p_off)
{
	struct _rpmsg_eptdev *local = filp->private_data;
	struct sk_buff *skb;
	unsigned long flags;
	int retlen;

	spin_lock_irqsave(&local->queue_lock, flags);

	/* wait for data int he queue */
	if (skb_queue_empty(&local->queue)) {
		spin_unlock_irqrestore(&local->queue_lock, flags);

		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		if (wait_event_interruptible(local->readq,
				!skb_queue_empty(&local->queue)))
			return -ERESTARTSYS;

		spin_lock_irqsave(&local->queue_lock, flags);
	}

	skb = skb_dequeue(&local->queue);
	if (!skb) {
		dev_err(&local->dev, "Read failed, RPMsg queue is empty.\n");
		return -EFAULT;
	}

	spin_unlock_irqrestore(&local->queue_lock, flags);
	retlen = min_t(size_t, len, skb->len);
	if (copy_to_user(ubuff, skb->data, retlen)) {
		dev_err(&local->dev, "Failed to copy data to user.\n");
		kfree_skb(skb);
		return -EFAULT;
	}

	kfree_skb(skb);
	return retlen;
}

static unsigned int rpmsg_dev_poll(struct file *filp, poll_table *wait)
{
	struct _rpmsg_eptdev *local = filp->private_data;
	unsigned int mask = 0;

	poll_wait(filp, &local->readq, wait);

	if (!skb_queue_empty(&local->queue))
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

static long rpmsg_dev_ioctl(struct file *p_file, unsigned int cmd,
				unsigned long arg)
{
	/* No ioctl supported a the moment */
	return -EINVAL;
}

static int rpmsg_dev_release(struct inode *inode, struct file *p_file)
{
	struct _rpmsg_eptdev *eptdev = cdev_to_eptdev(inode->i_cdev);
	struct rpmsg_device *rpdev = eptdev->rpdev;
	struct sk_buff *skb;
	unsigned int msg = SHUTDOWN_MSG;

	spin_lock(&eptdev->queue_lock);
	eptdev->is_sk_queue_closed = true;
	spin_unlock(&eptdev->queue_lock);

	/* Delete the skb buffers */
	while(!skb_queue_empty(&eptdev->queue)) {
		skb = skb_dequeue(&eptdev->queue);
		kfree_skb(skb);
	}

	dev_info(&rpdev->dev, "Sending shutdown message.\n");
	if (rpmsg_send(eptdev->ept,
			&msg,
			sizeof(msg))) {
		dev_err(&rpdev->dev,
			"Failed to send shutdown message.\n");
		return -EINVAL;
	}

	put_device(&rpdev->dev);
	return 0;
}

static const struct file_operations rpmsg_dev_fops = {
	.owner = THIS_MODULE,
	.read = rpmsg_dev_read,
	.poll = rpmsg_dev_poll,
	.write = rpmsg_dev_write,
	.open = rpmsg_dev_open,
	.unlocked_ioctl = rpmsg_dev_ioctl,
	.release = rpmsg_dev_release,
};

static void rpmsg_user_dev_release_device(struct device *dev)
{
	struct _rpmsg_eptdev *eptdev = dev_to_eptdev(dev);

	dev_info(dev, "Releasing rpmsg user dev device.\n");
	ida_simple_remove(&rpmsg_minor_ida, dev->id);
	cdev_del(&eptdev->cdev);
	/* No need to free the local dev memory eptdev.
	 * It will be freed by the system when the dev is freed
	 */
}

static const char *const shutdown_argv[]
= { "/sbin/shutdown", "-h", "-P", "now", NULL };

static int rpmsg_echo_test_kern_app_echo_test(struct _rpmsg_eptdev
					      *rpmsg_dev)
{
	static int payload_num = 0;
	static int next_payload_size = PAYLOAD_MIN_SIZE;
	int payload_size = 0;
//	int i = 0;
	struct _payload *payload;
	int err = 0;
	if (!rpmsg_dev) {
		return -1;
	}
	//pr_info("%s\n", __func__);
	if (next_payload_size > PAYLOAD_MAX_SIZE) {
		*((unsigned int *)rpmsg_dev->tx_buff) = SHUTDOWN_MSG;
		//pr_info("Sending shutdown message to remote.\n");
		err =
		    rpmsg_send(rpmsg_dev->ept, rpmsg_dev->tx_buff,
			       sizeof(unsigned int));
		if (err) {
			pr_err("Shutdown message send failed.\n");
			return -1;
		}
	} else {
		payload_size = next_payload_size++;
		payload = (struct _payload *)(rpmsg_dev->tx_buff);
		payload->num = payload_num++;
		payload->size = payload_size;
		memset(payload->data, 0xA5, payload_size);
		err =
		    rpmsg_send(rpmsg_dev->ept, rpmsg_dev->tx_buff,
			       (payload_size + sizeof(struct _payload)));
		if (err) {
			pr_err("Failed to send echo test message to remote.\n");
			return -1;
		}
	}
	return payload_size;
}

//static void rpmsg_echo_test_kern_app_work_func(struct work_struct *work)
//{
//	struct _rpmsg_eptdev *local =
//	    container_of(work, struct _rpmsg_eptdev, rpmsg_work);
//	//pr_info ("%s:%p.\n", __func__, local);
//	int local_err_cnt = 0;
//	if (rpmsg_echo_test_kern_app_echo_test(local) <= 0) {
//		spin_lock(&local->queue_lock);
//		local_err_cnt = local->err_cnt;
//		spin_unlock(&local->queue_lock);
//		pr_info("\r\n *******************************************\r\n");
//		pr_info("\r\n Echo Test Results: Error count = %d\r\n",
//			local_err_cnt);
//		pr_info("\r\n *******************************************\r\n");
//	}
//}

static int rpmsg_user_dev_rpmsg_drv_cb(struct rpmsg_device *rpdev, void *data,
					int len, void *priv, u32 src)
{
//    struct sk_buff *rx_skb;
//	struct _rpmsg_eptdev *local = dev_get_drvdata(&rpdev->dev);
//	struct _payload *payload = data;
//    unsigned char *type;
//int i = 0;
//    struct iphdr *ih;
//    __be32 *saddr, *daddr, tmp;
//    unsigned char    tmp_dev_addr[ETH_ALEN];
//    struct ethhdr *ethhdr;
//char* buf;
//short port = 0;
//ipv4_header_t* ihdr;
//udp_header_t* uhdr;

if(vnet_dev == NULL)
return 0;
	if (!data) {
		return 0;
	}

	if ((*(int *)data) == SHUTDOWN_MSG) {
		dev_info(&rpdev->dev,
			 "shutdown message is received. Shutting down...\n");
	} else {

//		rx_skb = alloc_skb(len, GFP_ATOMIC);
//		if (!rx_skb)
//			return -ENOMEM;
//		memcpy(skb_put(rx_skb, len), data, len);
//		rx_skb->dev = vnet_dev;
//		rx_skb->protocol = eth_type_trans(rx_skb, vnet_dev);
//		rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
//		vnet_dev->stats.rx_packets++;
//		vnet_dev->stats.rx_bytes += len;
//		netif_rx_ni(rx_skb);

#if VIRT_ETH_TEST
#else
		virt_eth_mgmt_recv(vnet_dev,data,len);
#endif

//    		emulator_rx_packet(rx_skb, vnet_dev);
//    		dev_kfree_skb (rx_skb);
	}
	return 0;
}

static int rpmsg_user_dev_rpmsg_drv_probe(struct rpmsg_device *rpdev)
{
	struct _rpmsg_eptdev *local;
	struct device *dev;
	int err = 0;
	int ret;
//	struct ethhdr *ethhdr;
	S_MGMT_MSG *mgmt_msg;
	S_PARAM_SOFT_ID* s_id;
	S_PARAM_MSG* s_rx_param;
	S_PARAM_RX_DMA* s_rx_dma;
	char buf[500];
	u32 buf_len = 0;
	int i = 0;
	dev_info(&rpdev->dev, "%s\n", __func__);

	local = devm_kzalloc(&rpdev->dev, sizeof(struct _rpmsg_eptdev),
			GFP_KERNEL);
	if (!local)
		return -ENOMEM;

	/* Initialize locks */
	spin_lock_init(&local->queue_lock);

	/* Initialize sk_buff queue */
	skb_queue_head_init(&local->queue);
	init_waitqueue_head(&local->readq);

	local->rpdev = rpdev;
	local->ept = rpdev->ept;

	dev = &local->dev;
	device_initialize(dev);
	dev->parent = &rpdev->dev;
	dev->class = rpmsg_class;

	/* Initialize character device */
	cdev_init(&local->cdev, &rpmsg_dev_fops);
	local->cdev.owner = THIS_MODULE;

	/* Get the rpmsg char device minor id */
	ret = ida_simple_get(&rpmsg_minor_ida, 0, RPMSG_USER_DEV_MAX_MINORS,
			GFP_KERNEL);
	if (ret < 0) {
		dev_err(&rpdev->dev, "Not able to get minor id for rpmsg device.\n");
		goto error1;
	}
	dev->id = ret;
	dev->devt = MKDEV(MAJOR(rpmsg_dev_major), ret);
	dev_set_name(&local->dev, "rpmsg%d", ret);

	ret = cdev_add(&local->cdev, dev->devt, 1);
	if (ret) {
		dev_err(&rpdev->dev, "chardev registration failed.\n");
		goto error2;
	}

	/* Set up the release function for cleanup */
	dev->release = rpmsg_user_dev_release_device;

	ret = device_add(dev);
	if (ret) {
		dev_err(&rpdev->dev, "device reister failed: %d\n", ret);
		put_device(dev);
		return ret;
	}

	dev_set_drvdata(&rpdev->dev, local);

	dev_info(&rpdev->dev, "new channel: 0x%x -> 0x%x!\n",
			rpdev->src, rpdev->dst);

	sprintf(local->tx_buff, RPMG_INIT_MSG);
	if (rpmsg_sendto(local->ept,
			 local->tx_buff, sizeof(RPMG_INIT_MSG), rpdev->dst)) {
		dev_err(&rpdev->dev, "Failed to send init_msg to target 0x%x.",
			local->rpmsg_dst);
		goto error2;
	}

	//INIT_WORK(&local->rpmsg_work, rpmsg_echo_test_kern_app_work_func);

	//schedule_work(&local->rpmsg_work);

//	ethhdr = (struct ethhdr *)(buf);
//  memcpy(ethhdr->h_dest, vnet_dev->dev_addr, ETH_ALEN);
//  memcpy(ethhdr->h_source, vnet_dev->dev_addr, ETH_ALEN);
//	ethhdr->h_proto = 0x0101;

	mgmt_msg = (S_MGMT_MSG*)buf;
	mgmt_msg->type = MGMT_MAC_SETTING;
	mgmt_msg->param_num = 2;
	mgmt_msg->version = MSG_VERSION;
	buf_len += sizeof(S_MGMT_MSG);

	s_id = (S_PARAM_SOFT_ID*)(buf+sizeof(S_MGMT_MSG));
	s_id->msg.type = soft_id;
	s_id->msg.len = 2;
	s_id->id = vnet_dev->dev_addr[5];
	s_id->resv = 0;
	buf_len += sizeof(S_PARAM_SOFT_ID);
	s_rx_param = (S_PARAM_MSG*)((void*)s_id + sizeof(S_PARAM_SOFT_ID));
	s_rx_param->type = rx_dma;
	s_rx_param->len = 400;
	buf_len += sizeof(S_PARAM_MSG);
	s_rx_dma = (S_PARAM_RX_DMA*)((void*)s_rx_param + sizeof(S_PARAM_MSG));
//	memcpy((u8*)s_rx_dma->rx_dma_addr,(u8*)rx_dma_addr,sizeof(dma_addr_t)*NUM_RX_PKT_BUF);
#if TCP_LOOPBACK_TEST

	for(i = 0; i < NUM_RX_PKT_BUF-1; i ++)
	{
		s_rx_dma->rx_dma_addr[i] = rx_dma_addr[i];
	}
#else
	for(i = 0; i < NUM_RX_PKT_BUF+1; i ++)
	{
		s_rx_dma->rx_dma_addr[i] = rx_dma_addr[i];
	}

#endif
	buf_len += sizeof(S_PARAM_RX_DMA);
	mgmt_msg->len = buf_len - sizeof(S_MGMT_MSG);
	err = rpmsg_send(local->ept,buf,buf_len);

	if (err) {
		pr_err("Failed to send message to remote.\n");
		//return -1;
	}


	Rpmsg_dev = rpdev;
#ifdef Zynq_Platform
	virt_eth_system_set_priv(vnet_dev,Rpmsg_dev);
#endif
	return 0;

error2:
	ida_simple_remove(&rpmsg_minor_ida, dev->id);
	put_device(dev);

error1:
	return ret;
}

static void rpmsg_user_dev_rpmsg_drv_remove(struct rpmsg_device *rpdev)
{
	struct _rpmsg_eptdev *local = dev_get_drvdata(&rpdev->dev);

	dev_info(&rpdev->dev, "Removing rpmsg user dev.\n");

	device_del(&local->dev);
	put_device(&local->dev);
}

static struct rpmsg_device_id rpmsg_user_dev_drv_id_table[] = {
	{ .name = "rpmsg-openamp-demo-channel" },
	{},
};

static struct rpmsg_driver rpmsg_user_dev_drv = {
	.drv.name = KBUILD_MODNAME,
	.drv.owner = THIS_MODULE,
	.id_table = rpmsg_user_dev_drv_id_table,
	.probe = rpmsg_user_dev_rpmsg_drv_probe,
	.remove = rpmsg_user_dev_rpmsg_drv_remove,
	.callback = rpmsg_user_dev_rpmsg_drv_cb,
};




//static void emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
//{
//    unsigned char *type;
//    struct iphdr *ih;
//    __be32 *saddr, *daddr, tmp;
//    unsigned char    tmp_dev_addr[ETH_ALEN];
//    struct ethhdr *ethhdr;
//
//short port = 0;
//
//    struct sk_buff *rx_skb;
//ipv4_header_t* ihdr;
//udp_header_t* uhdr;
//
//   // 从硬件读出/保存数据
//   /* 对调"源/目的"的mac地址 */
//    ethhdr = (struct ethhdr *)skb->data;
//    memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
//    memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
//    memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);
//
//    /* 对调"源/目的"的ip地址 */
//    ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
//    saddr = &ih->saddr;
//    daddr = &ih->daddr;
//
//
//    tmp = *saddr;
//    *saddr = *daddr;
//    *daddr = tmp;
//
//ihdr = (ipv4_header_t*)(skb->data + sizeof(struct ethhdr));
//if(ihdr->protocol == 0x01)
//{
//    //((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
//    //((u8 *)daddr)[2] ^= 1;
//    type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
//    //printk("tx package type = %02x\n", *type);
//    // 修改类型, 原来0x8表示ping
//    *type = 0; /* 0表示reply */
//
//}
//else if(ihdr->protocol == 0x11)
//{
//uhdr = (udp_header_t*)((void*)ihdr + sizeof(ipv4_header_t));
//port = uhdr->src_port;
//uhdr->src_port = uhdr->dest_port;
//uhdr->dest_port = 0x1027;
//uhdr->checksum = 0;
//}
//    ih->check = 0;           /* and rebuild the checksum (ip needs it) */
//    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
//
//    // 构造一个sk_buff
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
//
//    // 提交sk_buff
//    netif_receive_skb(rx_skb);
//}
 
static int virt_net_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	u8 ret = 0;
	bat_unicast_header_t     * bat_unicast_hdr;
	ret = virt_eth_util_encap(skb,dev);
	if(ret != 0){

		dev_kfree_skb (skb); 
		bat_unicast_hdr = (bat_unicast_header_t*)((void*)eth_hdr(skb) + sizeof(struct ethhdr));
        if(bat_unicast_hdr->type == BATADV_IV_OGM){
	      // printk("delet OGM PK\n");

		}


	}
	



//	struct _rpmsg_eptdev *local;
//	static int cnt = 0;
//	int err = 0,i = 0;
//char* buf;
//
//	if(Rpmsg_dev == NULL){
//	    dev_kfree_skb (skb);
//		return 0;
//	}
//	local = dev_get_drvdata(&Rpmsg_dev->dev);
//
//
//
//    netif_stop_queue(dev); /* 停止该网卡的队列 */
////    emulator_rx_packet(skb, dev);
//
//	err = rpmsg_send(local->ept, skb->data,skb->len);
//
//	if (err) {
//		//pr_err("Failed to send echo test message to remote.\n");
//		//return -1;
//	}
//
//    dev_kfree_skb (skb);   /* 释放skb */
//    netif_wake_queue(dev); /* 数据全部发送出去后,唤醒网卡的队列 */
//
//    /* 更新统计信息 */
//    dev->stats.tx_packets++;
//    dev->stats.tx_bytes += skb->len;
    return 0;
}

static const struct net_device_ops virt_netdev_ops = {
    .ndo_start_xmit        = virt_net_send_packet,
};


#define MAC_FILE "/etc/vnet-mac"

static int __init init(void)
{
	u8 mac_buf[17] = {0,};
	u8 mac_addr[6];
	int ret,i = 0;
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
	u8 flag = 0;
//	dma_addr_t daddr;
	ret = virt_eth_dma_init();
	if(ret != 0)
		return ret;

	
	//printk("rpmsg init\n");
	fp = filp_open(MAC_FILE,O_RDONLY,0);
	if(IS_ERR(fp))
	{
		printk("error to read file vnet-mac\n");
		flag = 0;
	}
	else{
		//printk("rpmsg read mac\n");
		fs = get_fs();
		set_fs(KERNEL_DS);
		pos = 0;

		kernel_read(fp,mac_buf,sizeof(mac_buf),&pos);
		mac_buf[17] = '\0';
		if(strlen(mac_buf) < 17)
		{
			printk("read mac_buf length < 12");
			flag = 0;
		}
		else
		{
			for(i = 0; i < 6; i ++)
			{
				u8 mac_tmp[3] = {0,};
				memcpy(mac_tmp,(char*)(mac_buf+i*3),2);
				mac_addr[i] = simple_strtoul(mac_tmp,NULL,16);
			}
			//printk("mac_addr %2x %2x %2x %2x %2x %2x\n",mac_addr[0],
//mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
		
			flag = 1;
		}

		filp_close(fp,NULL);
		set_fs(fs);
	}


	/* Allocate char device for this rpmsg driver */

    vnet_dev = alloc_netdev(sizeof(struct virt_eth_priv), "eth%d", NET_NAME_UNKNOWN,ether_setup);  /* alloc_etherdev */
    if(vnet_dev == NULL)
    {
	printk("vnet_dev create error!\n");
	return -1;
    }
	
    /* 2. 设置 */
    vnet_dev->netdev_ops    = &virt_netdev_ops;
    vnet_dev->mtu = 1560;
    vnet_dev->needs_free_netdev = true;
//    vnet_dev->features |= NETIF_F_HW_VLAN_CTAG_FILTER | NETIF_F_NETNS_LOCAL;
    vnet_dev->priv_flags |= IFF_NO_QUEUE;

    /* 设置MAC地址 */
if(flag)
{
    vnet_dev->dev_addr[0] = mac_addr[0];
    vnet_dev->dev_addr[1] = mac_addr[1];
    vnet_dev->dev_addr[2] = mac_addr[2];
    vnet_dev->dev_addr[3] = mac_addr[3];
    vnet_dev->dev_addr[4] = mac_addr[4];
    vnet_dev->dev_addr[5] = mac_addr[5];

}
else{
    vnet_dev->dev_addr[0] = 0xb8;
    vnet_dev->dev_addr[1] = 0x8e;
    vnet_dev->dev_addr[2] = 0xdf;
    vnet_dev->dev_addr[3] = 0x00;
    vnet_dev->dev_addr[4] = 0x01;
    vnet_dev->dev_addr[5] = 0x01;
 }
    /* 设置下面两项才能ping通 */
    vnet_dev->flags           |= IFF_NOARP;
    vnet_dev->features        |= NETIF_F_HW_CSUM;

    /* 3. 注册 */
    //register_netdevice(vnet_dev);
    register_netdev(vnet_dev);


	ret = alloc_chrdev_region(&rpmsg_dev_major, 0,
				RPMSG_USER_DEV_MAX_MINORS,
				KBUILD_MODNAME);
	if (ret) {
		pr_err("alloc_chrdev_region failed: %d\n", ret);
		return ret;
	}

	/* Create device class for this device */
	rpmsg_class = class_create(THIS_MODULE, KBUILD_MODNAME);
	if (IS_ERR(rpmsg_class)) {
		ret = PTR_ERR(rpmsg_class);
		pr_err("class_create failed: %d\n", ret);
		goto unreg_region;
	}



	
	ret = virt_eth_system_init(vnet_dev);
	if(ret != 0)
		return ret;
//	printk("eth0 1\n");
//	virt_eth_dma_create_buf(&daddr,824);
//	printk("eth0 2\n");

	ret = virt_eth_station_init(vnet_dev);
	if(ret != 0)
		return ret;
	ret = virt_eth_queue_init(vnet_dev);
	if(ret != 0)
		return ret;
	ret = virt_eth_mgmt_init(vnet_dev);
	if(ret != 0)
		return ret;
	virt_eth_jgk_init(vnet_dev);

	ret = register_rpmsg_driver(&rpmsg_user_dev_drv);


#if VIRT_ETH_TEST
#else
	virt_eth_comm_queue_warning(vnet_dev);
#endif
	return ret;



unreg_region:
	unregister_chrdev_region(rpmsg_dev_major, RPMSG_USER_DEV_MAX_MINORS);
	return ret;
}

static void __exit fini(void)
{
printk("__exit\n");

	virt_eth_dma_exit();


    unregister_netdev(vnet_dev);
    free_netdev(vnet_dev);

	
	unregister_rpmsg_driver(&rpmsg_user_dev_drv);
	unregister_chrdev_region(rpmsg_dev_major, RPMSG_USER_DEV_MAX_MINORS);
	class_destroy(rpmsg_class);
}

module_init(init);
module_exit(fini);


MODULE_DESCRIPTION("Sample driver to exposes rpmsg svcs to userspace via a char device");
MODULE_LICENSE("GPL");
