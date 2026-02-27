/*
 * sqlite_unit.h
 *
 *  Created on: Apr 2, 2024
 *      Author: slb
 */

#ifndef SQLITE_UNIT_H_
#define SQLITE_UNIT_H_

#define SYSTEMINFO_PARAM_NUM 105
#define SQLDATALEN           1024
extern uint8_t  rate_auto;
int sqlite_set_param(void);
stInData* getSystemInfo(void);
int sqliteinit(void);
void updateData_systeminfo(stInData data);
void updateData_linkinfo(stLink *data,int cnt,int selfid);
void updateData_timeslotinfo(unsigned char           data, int selfid);


#endif /* SQLITE_UNIT_H_ */
