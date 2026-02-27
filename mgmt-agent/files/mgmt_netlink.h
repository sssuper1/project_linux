/*
 * mgmt_netlink.h
 *
 *  Created on: Aug 12, 2020
 *      Author: slb
 */

#ifndef MGMT_NETLINK_H_
#define MGMT_NETLINK_H_


#include <linux/types.h>
#include <net/genetlink.h>
#include "mgmt_types.h"

struct nlmsghdr;

extern struct batadv_jgk_node bat_jgk_node_msg;
extern struct routetable routet_msg;
extern jgk_report_infor jgk_information_data_msg;

void mgmt_netlink_register(void);
void mgmt_netlink_unregister(void);
int mgmt_netlink_get_ifindex(const struct nlmsghdr *nlh, int attrtype);

int mgmt_netlink_tpmeter_notify(struct batadv_priv *bat_priv, const u8 *dst,
				  u8 result, u32 test_time, u64 total_bytes,
				  u32 cookie);

extern struct genl_family mgmt_netlink_family;

#endif /* MGMT_NETLINK_H_ */
