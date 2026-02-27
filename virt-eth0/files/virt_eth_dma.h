/*
 * virt_eth_dma.h
 *
 *  Created on: 2020-4-29
 *      Author: lu
 */

#ifndef VIRT_ETH_DMA_H_
#define VIRT_ETH_DMA_H_
#include <linux/types.h>
#include <linux/of_dma.h>
#include <linux/device.h>

extern dma_addr_t rx_dma_addr[];
#define PKG_SIZE 16384 //16384//4096

struct dma_chan* virt_eth_dma_get_chan(void);
//int virt_eth_dma_get_rx_index(void);
u8* virt_eth_dma_get_tx_buffer(int index);
u8* virt_eth_dma_get_rx_buffer(int index);
//u8* virt_eth_dma_get_slot_tb_buffer(int index);
//int virt_eth_slot_get_slot_tb(void);



//u8* virt_eth_dma_create_buf(dma_addr_t *databuf,unsigned int buf_len);
//void virt_eth_dma_free_buf(u8* data,dma_addr_t databuf,unsigned int buf_len);

int virt_eth_dma_tx(dma_addr_t dma_dsts,int tx_index,unsigned int len);
int virt_eth_dma_rx(dma_addr_t dma_srcs,int rx_index,unsigned int len);

int virt_eth_dma_init(void);
void virt_eth_dma_exit(void);
#endif /* VIRT_ETH_DMA_H_ */
