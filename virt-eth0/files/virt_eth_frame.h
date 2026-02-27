/*
 * virt_eth_frame.h
 *
 *  Created on: 2020-4-14
 *      Author: lu
 */

#ifndef VIRT_ETH_FRAME_H_
#define VIRT_ETH_FRAME_H_

#include <linux/types.h>
#include <linux/skbuff.h>

#define IP_CE        0x8000
#define IP_OFFSET	 0x1FFF
#define IP_DF		 0x4000		/* Flag: "Do   Fragment"	*/
#define IP_LF		 0x2000		/* Flag: "last Fragments"	*/

#define MINFREAMLEN       									sizeof(llc_header_t)


u8 virt_eth_frame_aggre_ampdu_transmit(struct virt_eth_priv *veth_priv,bool first,u32 mcs_len,struct sk_buff* skb,u8* daddr,u8 mcs,u8 node_mcs);

#endif /* VIRT_ETH_FRAME_H_ */
