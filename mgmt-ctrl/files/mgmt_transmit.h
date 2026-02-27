#ifndef MGMT_TRANSMIT_H_
#define MGMT_TRANSMIT_H_

//#include <mysql/mysql.h>
#include "mgmt_types.h"

extern uint8_t SELFID;
extern uint32_t FREQ_INIT;
extern uint16_t POWER_INIT;
extern uint8_t BW_INIT;
extern uint8_t MCS_INIT;
extern uint16_t MACMODE_INIT;
extern double longitude;
extern double latitude;
//extern stMeshInfo meshinfo;
extern uint8_t NET_WORKMOD_INIT;
extern uint8_t RX_CHANNEL_MODE_INIT;
extern uint8_t SELFIP_s[4];
extern uint8_t DEVICETYPE_INIT;
extern uint32_t HOP_FREQ_TB_INIT[32];

#ifdef Radio_CEC

extern uint8_t NCU_NODE_ID_INIT;
void Beam_infor_update_cyclic(void);

#endif



void mgmt_mysql_init();
void mgmt_recv_web(void);
void mgmt_recv_msg(void);
void mgmt_get_msg(void);
double htond(double val);
uint8_t find_minMcs(uint8_t *arr,int size);
void updateData_init(void);

void mgmt_status_new_report(void);
void mgmt_selfcheck_report(void);
void thread_report_test(void);
void mgmt_recv_from_qkwg(void);
void mgmt_recv_from_qkcj(void);

void txpower_lookup_channels(uint16_t single, uint16_t channel_power[POWER_CHANNEL_NUM]);
void txpower_lookup_channels_be(uint16_t single, uint16_t channel_power[POWER_CHANNEL_NUM]);

#endif
