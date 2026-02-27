#include "gpsget.h"

/**
 * 设置串口参数
 *  fd:
 * 
 * */ 

/* global struct */

GPS_INFO gps_info_uart;

int set_opt(int fd, int bSpeed, int dBits, char parity, int stopBit) 
{
	struct termios newtio, oldtio;
	if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");
		exit(1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD; //将本地模式(CLOCAL)和串行数据接收(CREAD)设置为有效
	/*这里有两个选项应当一直打开，一个是CLOCAL，另一个是CREAD。这两个选项可以保证你的程序不会
	 变成端口的所有者，而端口所有者必须去处理发散性作业控制和挂断信号，同时还保证了串行接口驱动会读取过来的数据字节。*/

	newtio.c_cflag &= ~CSIZE; //屏蔽数据位

	switch (dBits) {
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8; //8 data bits
		break;
	}

	//设置奇偶位
	switch (parity) {
	case 'O':
		newtio.c_cflag |= PARENB; //使能奇偶校验
		newtio.c_cflag |= PARODD; //奇
		newtio.c_iflag |= (INPCK | ISTRIP); //将奇偶校验设置为有效同时从接收字串中脱去奇偶校验位
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_iflag &= ~(IXON | IXOFF | IXANY |BRKINT | ICRNL | ISTRIP );
		newtio.c_cflag &= ~PARENB;
		break;
	}

	//设置波特率
	switch (bSpeed) {
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}

	//设置停止位
	if (stopBit == 1)
		newtio.c_cflag &= ~CSTOPB;
	else if (stopBit == 2)
		newtio.c_cflag |= CSTOPB;



	newtio.c_cc[VTIME] = 1; //设置等待数据时间，单位：0.1秒
	newtio.c_cc[VMIN] = 200; //Minimum number of characters to read

	tcflush(fd, TCIFLUSH); //刷新缓冲区，让输入输出数据有效：Flush input and output buffers and make the change
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) //TCSANOW标志所有改变必须立刻生效而不用等到数据传输结束
			{
		perror("com set error");
		return -1;
	}

	return 0;
}

int powa(int n) 
{
	int i = 0;
	int ret = 10;
	if (n <= 0)
		return 1;
	for (; i < n - 1; i++)
		ret *= 10;
	return ret;
}

/* 度分格式转换成度 */
double dmconverttodeg(char* dm)
{
	double s_dm;
	int deg;
	double min;

	if(dm==NULL||strlen(dm)==0)
	{
		return 0.0;
	}
	s_dm=atof(dm);

	deg = (int)(s_dm / 100);
	min = s_dm - deg * 100;

	double ret = deg + min / 60.0;
	return ret;
}

// UTC时间转换北京时间：将hhmmss格式转换为时分秒
void convertUTCToBeijingTime(const char* utc, uint8_t* time) {

    if(strlen(utc) >= 6) {
        uint8_t hour = (utc[0]-'0')*10 + (utc[1]-'0');
        uint8_t minute = (utc[2]-'0')*10 + (utc[3]-'0');
        uint8_t second = (utc[4]-'0')*10 + (utc[5]-'0');
        
        // UTC转北京时间 (+8小时)
        hour += 8;
        if(hour >= 24) {
            hour -= 24;
        }
        
        time[0] = hour;
        time[1] = minute;
        time[2] = second;
    } else {
        time[0] = time[1] = time[2] = 0;
    }
}

void gps_getfrom_uart(int fd)
{
    int nemafd=0;
    int i=0;
    int count=0;
    char nema[MAX_GPS_SIZE];
	// static double s_lon,s_lat;
	// static uint16_t s_gaodu;

	
	memset(&gps_info_uart,0,sizeof(GPS_INFO));
    int nread;
    set_opt(fd, GPS_UART_BAUD, 8, 'N', 1);	//设置nmea串口属性
	write(fd, "$cfgsys,h01", 10);   //该指令是实现gps什么操作？  add by sdg
	//char time_str[20];

	uint8_t cmd[200];
	// char fd_uart[100];
	// memset(fd_uart,0,sizeof(fd_uart));


	// FILE* file_uart;
	// if ((file_uart = fopen("/etc/uart_config", "r")) != NULL) 
	// {
	// 	/* read uart path from file */
	// 	while (fgets(fd_uart, sizeof(fd_uart), file_uart) != NULL)
	// 	{
			
	// 	}
	// 	fclose(file_uart);
	// }

    while(1)
    {
        
        memset(nema,0,sizeof(nema));
        

		/*获取gps信息*/
        nread=read(fd,nema,MAX_GPS_SIZE); //从串口获取gps信息
        if(nread>0)
        {
            if(nread<=8)
            {
                /* 读取字节小于8字节，数据不完整 ，连续超出5次读取失败，重启串口*/
                count++;
            }
            else
            {
                count=0;
            }

            if(count>5)
            {
                printf("error:fail to read gps from uart,reopen gps uart\r\n ");
                close(fd);
				sleep(2);
                while(1)
                {
                    /* 重启串口 */
                    nemafd=open(FD_GPS_UART,O_RDWR);
                    if (nemafd == -1) 
                    {
						perror("nemafd:");
						sleep(1);
                        
							//return;
							//exit(1);
					} 
                    else 
                    {
					    set_opt(nemafd, GPS_UART_BAUD, 8, 'N', 1);		//设置nmea串口属性
						write(nemafd, "$cfgsys,h01", 10);
						break;
					}

                }
            }
  
		int gps_size = sizeof(nema);
		char print_gps[gps_size];
		for (i = 0; i < sizeof(nema)-5; i++)
		{
			/* 查找 $GNGGA */
			// if ((nema[i] == '$') && (nema[i + 3] == 'G') && (nema[i + 4] == 'G') &&
			// 	(nema[i + 5] == 'A') )  // 检查海拔高度单位'M' && (nema[i + 43] == 'M')
			if(strstr(nema + i, "$GNGGA") == nema + i)
			{
				//memcpy(print_gps,nema,gps_size);
				// printf("find GNGGA INFO  \r\n");

				// 字段1: UTC时间
				char* str1 = strchr(nema + i +7, ',');
				if (str1 == NULL) continue;
				int str1_len = strlen(nema + i+7) - strlen(str1);
				memcpy(gps_info_uart.utc, nema + i+7, str1_len);
				// printf("UTC Time: %s \r\n", gps_info_uart.utc);
				convertUTCToBeijingTime(gps_info_uart.utc,gps_info_uart.bj_time);  //UTC时间转北京时间
				// 字段2: 纬度
				char* str2 = strchr(str1 + 1, ',');
				if (str2 == NULL) continue;
				int str2_len = strlen(str1 + 1) - strlen(str2);
				memcpy(gps_info_uart.latitude, str1 + 1, str2_len);
				//printf("Lat: %s \r\n", gps_info_uart.latitude);
				// 字段3: 纬度半球
				char* str3 = strchr(str2 + 1, ',');
				if (str3 == NULL) continue;
				int str3_len = strlen(str2 + 1) - strlen(str3);
				memcpy(&gps_info_uart.lat_mode, str2 + 1, str3_len);

				// 计算纬度
				if (gps_info_uart.lat_mode == 'N') {
					gps_info_uart.lat = dmconverttodeg(gps_info_uart.latitude);
				}
				else if (gps_info_uart.lat_mode == 'S') {
					gps_info_uart.lat = -dmconverttodeg(gps_info_uart.latitude);
				}
				else;
				

				// 字段4: 经度
				char* str4 = strchr(str3 + 1, ',');
				if (str4 == NULL) continue;
				int str4_len = strlen(str3 + 1) - strlen(str4);
				memcpy(gps_info_uart.longitude, str3 + 1, str4_len);
				//printf("Lon: %s \r\n", gps_info_uart.longitude);
				// 字段5: 经度半球
				char* str5 = strchr(str4 + 1, ',');
				if (str5 == NULL) continue;
				int str5_len = strlen(str4 + 1) - strlen(str5);
				memcpy(&gps_info_uart.lon_mode, str4 + 1, str5_len);

				// 计算经度
				if (gps_info_uart.lon_mode == 'E') {
					gps_info_uart.lon = dmconverttodeg(gps_info_uart.longitude);
				}
				else if (gps_info_uart.lon_mode == 'W') {
					gps_info_uart.lon = -dmconverttodeg(gps_info_uart.longitude);
				}

				

				// 字段6: 定位质量指示
				char* str6 = strchr(str5 + 1, ',');
				if (str6 == NULL) continue;
				int str6_len = strlen(str5 + 1) - strlen(str6);
				memcpy(gps_info_uart.position_status, str5 + 1, str6_len);
				if(atoi(gps_info_uart.position_status)==0)
				{
					//printf("[gps debug] invalid gps info\r\n");
					continue;
				}

				// 字段7: 使用卫星数量
				char* str7 = strchr(str6 + 1, ',');
				if (str7 == NULL) continue;
				int str7_len = strlen(str6 + 1) - strlen(str7);
				memcpy(gps_info_uart.satellites_used, str6 + 1, str7_len);

				// 字段8: 水平精度因子
				char* str8 = strchr(str7 + 1, ',');
				if (str8 == NULL) continue;
				int str8_len = strlen(str7 + 1) - strlen(str8);
				memcpy(gps_info_uart.hdop, str7 + 1, str8_len);

				// 字段9: 海拔高度
				char* str9 = strchr(str8 + 1, ',');
				if (str9 == NULL) continue;
				int str9_len = strlen(str8 + 1) - strlen(str9);
				memcpy(gps_info_uart.altitude, str8 + 1, str9_len);
				//printf("height:%s \r\n",gps_info_uart.altitude);
				gps_info_uart.gaodu=atoi(gps_info_uart.altitude);
				// 字段10: 高度单位 (应该是'M')
				char* str10 = strchr(str9 + 1, ',');
				if (str10 == NULL) continue;
				int str10_len = strlen(str9 + 1) - strlen(str10);
				memcpy(&gps_info_uart.altitude_unit, str9 + 1, str10_len);

				// printf("[GPS DEBUG] lat:%.4f,lon:%.4f,height:%d \r\n",
				// gps_info_uart.lat,gps_info_uart.lon,gps_info_uart.gaodu);


				/*update  node_xwg */

				// s_lon  =	gps_info_uart.lon;
				// s_lat  =    gps_info_uart.lat;
				// s_gaodu=    gps_info_uart.gaodu;
				

				// if(  fabs(gps_info_uart.lon - s_lon) >= GPS_DIFF_THRESHOLD_DOUBLE ||
   				// 	 fabs(gps_info_uart.lat - s_lat) >= GPS_DIFF_THRESHOLD_DOUBLE ||
				// 	abs(gps_info_uart.gaodu - s_gaodu) >= GPS_DIFF_THRESHOLD_INT)
				// {
				// 	memset(cmd, 0, sizeof(cmd));
				// 	sprintf(cmd,
				// 		"sed -i 's/longitude .*/longitude %d/g; "
				// 		"s/latitude  .*/latitude %d/g; "
				// 		"s/altitude  .*/altitude %d/g' /etc/node_xwg",
				// 		gps_info_uart.lon, 
				// 		gps_info_uart.lat, 
				// 		gps_info_uart.gaodu);

				// 	system(cmd);
				// }	

				break;
			}
		}


        }
        else
	    	printf("[GPS DEBUG]read gps info error \r\n");
            // continue;  //获取失败，重新获取

		sleep(10); // 添加10s休眠
    }

}



void getGPS(void)
 {
	int i = 0, j = 0, k = 0;
	int atFd, nmeaFd, nset1, nwrite, nread;
	char at[20], nmea[1024];
	char flag = 0;
	int count = 0;
	char a[2], b[3];
	double gps = 0, du = 0;
	int fen = 0, miao = 0;

	// char fd_uart[100];
	// memset(fd_uart,0,sizeof(fd_uart));


	// FILE* file_uart;
	// if ((file_uart = fopen("/etc/uart_config", "r")) != NULL) 
	// {
	// 	/* read uart path from file */
	// 	while (fgets(fd_uart, sizeof(fd_uart), file_uart) != NULL)
	// 	{
			
	// 	}
	// 	fclose(file_uart);
	// }

		nmeaFd = open(FD_GPS_UART, O_RDWR);  //打开nmea串口
		if (nmeaFd == -1)
        {
			printf("[GPS DEBUG]open gps \r\n");
			perror("nmeaFd:");
			sleep(1);
		}
        // else
		// 	break;
	// }
	gps_getfrom_uart(nmeaFd);

	close(nmeaFd);
	return;
}


void gps_Thread(void* arg) 
{
	printf("thread:get gps info\r\n");
	while (1) 
	{
		getGPS();
//		printf("gps recyle\n");
		sleep(1);
	}
}
