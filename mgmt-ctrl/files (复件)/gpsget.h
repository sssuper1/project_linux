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

#define MAX_GPS_SIZE 1024
#define FD_UART     "/dev/ttyPS1" 
#define FD_5G       "/dev/ttyATH0"
#define UART_BAUD    9600
typedef struct 
{
    char utc[20];
    char locate_mode;    //定位状态
    char latitude[20];   //纬度    
    char lat_mode;       //N，S
    char longitude[20];  //经度
    char lon_mode;       //E W
    char rate[20];       //地面速率
    char hangxiang[20];  //地面航向
    double lat;          //坐标格式
    double lon;          //
    
}GPS_INFO;


char Latitude[13];
char Longitude[13];

int set_opt(int fd, int bSpeed, int dBits, char parity, int stopBit);
void getGPS(void);
void gps_Thread(void* arg);


#endif
