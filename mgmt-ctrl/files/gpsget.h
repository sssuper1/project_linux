/*
 * @Author: SunDG311 sdg18252543282@163.com
 * @Date: 2025-09-05 10:34:04
 * @LastEditors: SunDG311 sdg18252543282@163.com
 * @LastEditTime: 2025-10-30 18:25:31
 * @FilePath: \mgmt-ctrl\files\gpsget.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef   _GPSGET_H
#define   _GPSGET_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<stdint.h>
#define MAX_GPS_SIZE 1024
#define FD_GPS_UART     "/dev/ttyUL3" 
#define FD_5G       "/dev/ttyATH0"
#define GPS_UART_BAUD    9600

#define GPS_DIFF_THRESHOLD_DOUBLE       0.001   // 经纬度阈值
#define GPS_DIFF_THRESHOLD_INT          10       // 高度阈值(米)
#pragma pack(push, 1)

typedef struct 
{
    char utc[20];
    uint8_t bj_time[3];
    char locate_mode;    //定位状态
    char latitude[20];   //纬度    
    char lat_mode;       //N，S
    char longitude[20];  //经度
    char lon_mode;       //E W
    // char rate[20];       //地面速率
    // char hangxiang[20];  //地面航向
    double lat;          //坐标格式
    double lon;          //
    uint16_t gaodu;
    // GNGGA特有字段
    char position_status[2];   // 定位质量指示
    char satellites_used[3]; // 使用卫星数量
    char hdop[6];           // 水平精度因子
    char altitude[10];      // 海拔高度
    char altitude_unit;     // 高度单位
}GPS_INFO;

#pragma pack(pop)  // 恢复默认对齐

char Latitude[13];
char Longitude[13];

int set_opt(int fd, int bSpeed, int dBits, char parity, int stopBit);
void getGPS(void);
void gps_Thread(void* arg);


#endif
