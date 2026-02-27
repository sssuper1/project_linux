/*
 * @Author: SunDG311 sdg18252543282@163.com
 * @Date: 2025-09-05 10:34:04
 * @LastEditors: SunDG311 sdg18252543282@163.com
 * @LastEditTime: 2025-11-10 16:34:13
 * @FilePath: \mgmt-ctrl\files\sqlite_unit.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * sqlite_unit.h
 *
 *  Created on: Apr 2, 2024
 *      Author: slb
 */

#ifndef SQLITE_UNIT_H_
#define SQLITE_UNIT_H_

#include <stdbool.h>

#define SYSTEMINFO_PARAM_NUM 105
#define SQLDATALEN           1024
extern uint8_t  rate_auto;
int sqlite_set_param(void);
stInData* getSystemInfo(void);
int sqliteinit(void);
void updateData_systeminfo(stInData data);
void updateData_linkinfo(stLink *data,int cnt,int selfid);
void updateData_timeslotinfo(unsigned char           data, int selfid);

//add by sdg
void updateData_systeminfo_qk(const char* name,const int value);
void updateData_meshinfo_qk(const char* name,const int value);
bool persist_test_db(void);


#endif /* SQLITE_UNIT_H_ */
