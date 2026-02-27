/*
 * mgmt_netlink.h
 *
 *  Created on: Aug 12, 2020
 *      Author: slb
 */

#ifndef MGMT_NETLINK_H_
#define MGMT_NETLINK_H_

//#include <netlink/genl/genl.h>
//#include <netlink/genl/ctrl.h>

#include "mgmt_types.h"

extern Smgmt_transmit_info mgmt_info;

extern ob_state_part1 slot_info;

char* mgmt_netlink_get_info(int ifindex, uint8_t nl_cmd, const char *header,char *remaining);
char mgmt_netlink_set_param(char* buffer,int buflen, const char *header);
char mgmt_netlink_set_param_wg(char* buffer,int buflen, const char *header,int type);

#endif /* MGMT_NETLINK_H_ */
