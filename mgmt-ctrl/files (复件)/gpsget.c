#include "gpsget.h"

/**
 * 设置串口参数
 *  fd:
 * 
 * */ 

 
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

	newtio.c_cc[VTIME] = 0; //设置等待数据时间，单位：0.1秒
	newtio.c_cc[VMIN] = 100; //Minimum number of characters to read

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

	s_dm=atof(dm);

	deg = s_dm / 100;
	min = s_dm - deg * 100;

	double ret = deg + min / 60.0;
	return ret;
}



void gps_getfrom_uart(int fd)
{
    int nemafd=0;
    int i=0;
    int count=0;
    char nema[MAX_GPS_SIZE];

	GPS_INFO gps_info_uart;
	memset(&gps_info_uart,0,sizeof(GPS_INFO));
    int nread;
    set_opt(fd, UART_BAUD, 8, 'N', 1);	//设置nmea串口属性
	write(fd, "$cfgsys,h01", 10);   //该指令是实现gps什么操作？  add by sdg

	char fd_uart[100];
	memset(fd_uart,0,sizeof(fd_uart));


	FILE* file_uart;
	if ((file_uart = fopen("/etc/uart_config", "r")) != NULL) 
	{
		/* read uart path from file */
		while (fgets(fd_uart, sizeof(fd_uart), file_uart) != NULL)
		{
			
		}
		fclose(file_uart);
	}

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
                printf("error:fail to read gps from uart\r\n ");
                close(fd);
				sleep(2);
                while(1)
                {
                    /* 重启串口 */
                    nemafd=open(fd_uart,O_RDWR);
                    if (nemafd == -1) 
                    {
						perror("nemafd:");
						sleep(1);
                        
							//return;
							//exit(1);
					} 
                    else 
                    {
					    set_opt(nemafd, UART_BAUD, 8, 'N', 1);		//设置nmea串口属性
						write(nemafd, "$cfgsys,h01", 10);
						break;
					}

                }
            }
            for(i=0;i<nread-5;i++)
            {
                /* 查找 $GPRMC*/
                if((nema[i] == '$') &&(nema[i + 3] == 'R')&& (nema[i + 4] == 'M')&& 
                    (nema[i + 5] == 'C')&&(nema[i + 18] == 'A') )	//&& (nema[i + 18] == 'A')
                {
			printf("find GPRMC INFO\r\n");
					char * str1=strchr(nema+7,',');
					if (str1 == NULL)
					{
						continue;
					}
					int str1_len = strlen((nema + 7)) - strlen(str1);
					//获取utc信息 hhmmss（时分秒）格式 
					memcpy(gps_info_uart.utc, nema + 7, str1_len);
					
					char* str2 = strchr(str1 + 1, ',');
					if (str2 == NULL)
					{
						continue;
					}
					int str2_len = strlen(str1 + 1) - strlen(str2);
					memcpy(&gps_info_uart.locate_mode, str1 +1,str2_len);

					char* str3 = strchr(str2 + 1, ',');
					if (str3 == NULL)
					{
						continue;
					}
					int str3_len = strlen(str2 + 1) - strlen(str3);
					//获取纬度 ：ddmm.mmmm（度分）格式
					
					memcpy(gps_info_uart.latitude, str2 +1,str3_len);
					//printf("copy lat %s ",gps_info_uart.latitude);

					char* str4 = strchr(str3 + 1, ',');
					if (str4 == NULL)
					{
						continue;
					}
					int str4_len = strlen(str3 + 1) - strlen(str4);
					//纬度半球N（北半球 "+"）或S（南半球"-"）
					memcpy(&gps_info_uart.lat_mode, str3 +1,str4_len);
					//printf("lat_mode %s ",gps_info_uart.lat_mode);
					if(gps_info_uart.lat_mode=='N')
					{
						/* 根据半球，判断纬度正负 */
						gps_info_uart.lat=dmconverttodeg(gps_info_uart.latitude);
					}
					else if(gps_info_uart.lat_mode=='S')
					{
						gps_info_uart.lat=-dmconverttodeg(gps_info_uart.latitude);

					}
					else
					{
						continue;
					}
					printf("gps lat:%.4f ",gps_info_uart.lat);					
					
					char* str5 = strchr(str4 + 1, ',');
					if (str5 == NULL)
					{
						continue;
					}
					int str5_len = strlen(str4 + 1) - strlen(str5);
					//经度 dddmm.mmmm（度分）格式
					memcpy(gps_info_uart.longitude, str4 +1,str5_len);
					//printf("lon %s ",gps_info_uart.longitude);
					char* str6 = strchr(str5 + 1, ',');
					if (str6 == NULL)
					{
						continue;
					}
					int str6_len = strlen(str5 + 1) - strlen(str6);
					//经度 半球E（东经）"+"或W（西经） "-"
					memcpy(&gps_info_uart.lon_mode, str5 +1,str6_len);
					//printf("lon_mode %s \r\n",gps_info_uart.lon_mode);
					if(gps_info_uart.lon_mode=='E')
					{/* 根据半球，判断经度正负 */
						gps_info_uart.lon=dmconverttodeg(gps_info_uart.longitude);
					}
					else if(gps_info_uart.lon_mode=='W')
					{
						gps_info_uart.lon=-dmconverttodeg(gps_info_uart.longitude);

					}
					else
					{
						continue;
					}
					printf("lon:%.4f \r\n",gps_info_uart.lon);
					char* str7 = strchr(str6 + 1, ',');
					if (str7 == NULL)
					{
						continue;
					}
					int str7_len = strlen(str6 + 1) - strlen(str7);
					//地面速率 000.0~999.9节
					memcpy(gps_info_uart.rate, str6 +1,str7_len);

					char* str8 = strchr(str7 + 1, ',');
					if (str8 == NULL)
					{
						continue;
					}
					int str8_len = strlen(str7 + 1) - strlen(str8);
					//地面航向 000.0~359.9度，以真北为参考基准
					memcpy(gps_info_uart.hangxiang, str7 +1,str8_len);

					printf("rate:%s hx:%s \r\n", gps_info_uart.rate, gps_info_uart.hangxiang);

                }
            }

        }
        else
	    printf("ERROR:read gps from uart fail\r\n");
            continue;  //获取失败，重新获取
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

	char fd_uart[100];
	memset(fd_uart,0,sizeof(fd_uart));


	FILE* file_uart;
	if ((file_uart = fopen("/etc/uart_config", "r")) != NULL) 
	{
		/* read uart path from file */
		while (fgets(fd_uart, sizeof(fd_uart), file_uart) != NULL)
		{
			
		}
		fclose(file_uart);
	}
	//memcpy(Latitude,"123.4444444",11);
	//memcpy(Longitude,"23.5555555",10);
	/*	char setled[3];
	 setled[0] = 0x55;
	 setled[2] = 0x66;
	 setled[1] = 0x88;*/

	/*atFd = open("/dev/ttyS0", O_RDWR);//打开at串口
	 if (atFd == -1)
	 {
	 perror("atFd:");
	 sleep(1);
	 return;
	 //exit(1);
	 }

	 set_opt(atFd, 4800, 8, 'N', 1);//设置at串口属性
	 memset(at, 0, 20);
	 memcpy(at, "AT+CGPS=1\r", sizeof("AT+CGPS=1\r"));
	 nwrite = write(atFd, at, strlen(at));
	 if(-1 == nwrite)
	 {
	 perror("at com write");
	 //exit(1);
	 sleep(1);
	 return;
	 }
	 */
	//printf("gpsget start system type %d\n",SYSTEM_TYPE);
	// while (1) 
    // {
		// if (SYSTEM_TYPE == SYSTEM_433) 
        // {
		// 	//nmeaFd = open("/dev/ttyMSM0", O_RDWR);	//打开nmea串口
        //     nmeaFd = open(FD_UART, O_RDWR);  //打开nmea串口
		// } 
//         else if (SYSTEM_TYPE == SYSTEM_10_10 || SYSTEM_TYPE == SYSTEM_MINI_5G) 
//         {
// //			printf("open /dev/ttyATH0\n");
// //	    	nmeaFd = open("/dev/ttyACM0", O_RDWR);//打开nmea串口
// 			// nmeaFd = open("/dev/ttyATH0", O_RDWR);	//打开nmea串口
//             nmeaFd = open(FD_5G, O_RDWR);	//打开nmea串口      

// 		} 
        // else 
        // {
		// 	printf("system_type error\n");
		// 	return;
		// }

		nmeaFd = open(fd_uart, O_RDWR);  //打开nmea串口
		if (nmeaFd == -1)
        {
			printf("open WRONG\r\n");
			perror("nmeaFd:");
			sleep(1);
		}
        // else
		// 	break;
	// }
	gps_getfrom_uart(nmeaFd);

	close(nmeaFd);
	return;

	// if (SYSTEM_TYPE == SYSTEM_433)
	// {
	// 	/* 从串口获取gps信息 */
	// 	gps_getfrom_uart(nmeaFd);
	// }
	// else if (SYSTEM_TYPE == SYSTEM_10_10 || SYSTEM_TYPE == SYSTEM_MINI_5G)
	// {
	// 	gps_getfrom_5g(nmeaFd);
	// }
// 	if (SYSTEM_TYPE == SYSTEM_433)
// 		set_opt(nmeaFd, 9600, 8, 'N', 1);	//设置nmea串口属性
// 	else if (SYSTEM_TYPE == SYSTEM_10_10 || SYSTEM_TYPE == SYSTEM_MINI_5G)
// //			set_opt(nmeaFd, 115200, 8, 'N', 1);//设置nmea串口属性
// 		set_opt(nmeaFd, 9600, 8, 'N', 1);	//设置nmea串口属性
// 	write(nmeaFd, "$cfgsys,h01", 10);   //该指令是实现gps什么操作？  add by sdg
// 	if (SYSTEM_TYPE == SYSTEM_433) {
// 		while (1) {
// 			memset(nmea, 0, 1024);
// 			nread = read(nmeaFd, nmea, 1024);	//读串口
// 			if (nread > 0) {
// 				//printf("\tGPS dataLen=%d, data:%s\n",nread,nmea);
// 				if (nread <= 8) {
// 					/* 读取字节小于8字节，数据不完整 ，连续超出5次读取失败，重启串口*/
// 					count++;
// 				} else
// 					count = 0;
// 				if (count > 5) {
// 					close(nmeaFd);
// 					sleep(2);
// 					while (1) {
// 						nmeaFd = open("/dev/ttyMSM0", O_RDWR | O_NONBLOCK);	//打开nmea串口

// 						if (nmeaFd == -1) {
// 							perror("nmeaFd:");
// 							sleep(1);
// 							//return;
// 							//exit(1);
// 						} else {
// 							set_opt(nmeaFd, 9600, 8, 'N', 1);		//设置nmea串口属性

// 							write(nmeaFd, "$cfgsys,h01", 10);
// 							break;
// 						}
// 					}
// 				}
// 				//nmea[nread] = '\0';
// 				/*  */
// 				for (i = 0; i < nread - 5; i++) {
// 					if ((nmea[i] == '$') && (nmea[i + 4] == 'L')
// 							&& (nmea[i + 5] == 'L') && (nmea[i + 47] == 'A')) {

// 						if (i + 32 > 1024)
// 							continue;
// 						/*	memset(Latitude,0,sizeof(Latitude));
// 						 memset(Longitude,0,sizeof(Longitude));
// 						 sscanf(nmea+7,"%s,N,%s,E,",Latitude,Longitude);
// 						 printf("Latitude1 = %s,Longgitude1 = %s\n",Latitude,Longitude);
// 						 */
// 						for (j = 0; j < 11; j++) {
// 							if (nmea[i + j + 7] == ',')
// 								break;
// 						}
// 						if ((j == 0) || (j < 11) || (i + j + 7 > nread))
// 							continue;

// 						memset(Latitude, 0, sizeof(Latitude));
// 						/*bzero(a,sizeof(a));
// 						 memcpy(a,nmea+i+7,sizeof(a));
// 						 du = atof(a);
// 						 sscanf(nmea+i+7+2,"%d.%d,",&fen,&miao);
// 						 gps = du+fen/60.0+miao/60.0/powa(j-5);
// 						 sprintf(Latitude,"%lf",gps);*/
// 						memcpy(Latitude, nmea + i + 7, j);
// 						//Latitude[2] = '.';
// 						//memcpy(Latitude+3,nmea+i+7+2,2);
// 						//memcpy(Latitude+5,nmea+i+7+2+3,j-5);

// 						for (j = 0; j < 12; j++) {
// 							if (nmea[i + j + 21] == ',')
// 								break;
// 						}
// 						if ((j == 0) || (j < 12) || (i + j + 21 > nread))
// 							continue;
// 						memset(Longitude, 0, sizeof(Longitude));
// 						/*bzero(b,sizeof(b));
// 						 memcpy(b,nmea+i+20,sizeof(b));
// 						 du = atof(b);
// 						 sscanf(nmea+i+20+3,"%d.%d,",&fen,&miao);
// 						 gps = du+fen/60.0+miao/60.0/powa(j-6);
// 						 sprintf(Longitude,"%lf",gps);*/
// 						memcpy(Longitude, nmea + i + 21, j);
// 						//Longitude[3] = '.';
// 						//memcpy(Longitude+4,nmea+i+20+3,2);
// 						//memcpy(Longitude+5,nmea+i+20+3+3,j-6);
// 						//printf( "ll N %s\n", Latitude); //输出所读取的数据
// 						//printf( "ll E %s\n", Longitude); //输出所读取的数据
// 						break;
// 					} else if ((nmea[i] == '$') && (nmea[i + 4] == 'M')
// 							&& (nmea[i + 5] == 'C') && (nmea[i + 18] == 'A')) {
// 						if (i + 44 > 1024)
// 							continue;
// 						/*	memset(Latitude,0,sizeof(Latitude));
// 						 memset(Longitude,0,sizeof(Longitude));
// 						 sscanf(nmea+7,"%s,N,%s,E,",Latitude,Longitude);
// 						 printf("Latitude2 = %s,Longgitude2 = %s\n",Latitude,Longitude);
// 						 */
// 						for (j = 0; j < 11; j++) {
// 							if (nmea[i + j + 20] == ',')
// 								break;
// 						}
// 						if ((j == 0) || (j < 11) || (i + j + 20 > nread))
// 							continue;
// 						for (k = 0; k < j; k++) {
// 							if (((nmea[k + i + 20] < '0')
// 									|| (nmea[k + i + 20] > '9'))
// 									&& (nmea[k + i + 20] != '.')) {
// //printf("gsp mc %c\n",nmea[k+i+19]);
// 								break;
// 							}
// 						}
// 						if (k != j)
// 							continue;
// 						memset(Latitude, 0, sizeof(Latitude));
// 						memcpy(Latitude, nmea + i + 20, j);
// 						//memcpy(Latitude,nmea+i+19,2);
// 						//Latitude[2] = '.';
// 						//memcpy(Latitude+3,nmea+i+19+2,2);
// 						//memcpy(Latitude+5,nmea+i+19+2+3,j-5);
// 						/*bzero(a,sizeof(a));
// 						 memcpy(a,nmea+i+19,sizeof(a));
// 						 du = atof(a);
// 						 sscanf(nmea+i+19+2,"%d.%d,",&fen,&miao);
// 						 gps = du+fen/60.0+miao/60.0/powa(j-5);
// 						 sprintf(Latitude,"%lf",gps);*/

// 						for (j = 0; j < 12; j++) {
// 							if (nmea[i + j + 34] == ',')
// 								break;
// 						}
// 						if ((j == 0) || (j < 12) || (i + j + 34 > nread))
// 							continue;
// 						memset(Longitude, 0, sizeof(Longitude));
// 						memcpy(Longitude, nmea + i + 34, j);
// 						//memcpy(Longitude,nmea+i+32,3);
// 						//Longitude[3] = '.';
// 						//memcpy(Longitude+4,nmea+i+32+3,2);
// 						//memcpy(Longitude+6,nmea+i+32+3+3,j-6);
// 						/*bzero(b,sizeof(b));
// 						 memcpy(b,nmea+i+32,sizeof(b));
// 						 du = atof(b);
// 						 sscanf(nmea+i+32+3,"%d.%d,",&fen,&miao);
// 						 gps = du+fen/60.0+miao/60.0/powa(j-6);
// 						 sprintf(Longitude,"%lf",gps);*/
// 						//printf( "mc N %s\n", Latitude); //输出所读取的数据
// 						//printf( "mc E %s\n", Longitude); //输出所读取的数据
// 						break;
// 					}

// 				}

// 			}
// 			break;
// 		}
// 	} else if (SYSTEM_TYPE == SYSTEM_10_10 || SYSTEM_TYPE == SYSTEM_MINI_5G) {
// 		while (1) {
// 			memset(nmea, 0, 1024);
// 			nread = read(nmeaFd, nmea, 1024);						//读串口
// //			printf("gps len %d\n",nread);
// //			printf("%s\n",nmea);
// 			if (nread > 0) {
// 				//printf("\tGPS dataLen=%d, data:%s\n",nread,nmea);
// 				if (nread <= 8) {
// 					count++;
// 				} else
// 					count = 0;
// 				if (count > 5) {
// 					close(nmeaFd);
// 					sleep(2);
// 					while (1) {
// 						nmeaFd = open("/dev/ttyATH0", O_RDWR);		//打开nmea串口
// 						if (nmeaFd == -1) {
// 							printf("GPS OPEN WRONG\n");
// 							perror("nmeaFd:");
// 							sleep(1);
// 							//return;
// 							//exit(1);
// 						} else {
// 							set_opt(nmeaFd, 9600, 8, 'N', 1);		//设置nmea串口属性

// 							write(nmeaFd, "$cfgsys,h01", 10);
// 							break;
// 						}
// 					}
// 				}
// 				//nmea[nread] = '\0';

// 				for (i = 0; i < nread - 5; i++) {

// 					if((nmea[i] == '$')&&(nmea[i+4] == 'L')&&(nmea[i+5] == 'L')&&(nmea[i+47] == 'A'))
// 					{
// 						if(i + 32 > 1024)
// 							continue;
// 						for(j = 0; j < 11; j ++)
// 						{
// 							if(nmea[i + j+7] == ',')
// 								break;
// 						}
// 						if((j == 0)||(j < 11)||(i + j+7 > nread))
// 							continue;
// 						memset(Latitude,0,sizeof(Latitude));
// 						memcpy(Latitude,nmea+i+7,j);
// 						//memcpy(Latitude,nmea+i+7,2);
// 						//Latitude[2] = '.';
// 						//memcpy(Latitude+3,nmea+i+7+2,2);
// 						//memcpy(Latitude+5,nmea+i+7+2+3,j-5);
// 						/*bzero(a,sizeof(a));
// 						memcpy(a,nmea+i+7,sizeof(a));
// 						du = atof(a);
// 						sscanf(nmea+i+7+2,"%d.%d,",&fen,&miao);
// 						gps = du+fen/60.0+miao/60.0/powa(j-5);
// 						sprintf(Latitude,"%lf",gps);*/
// 						for(j = 0; j < 12; j ++)
// 						{
// 							if(nmea[i + j+21] == ',')
// 								break;
// 						}
// 						if((j == 0)||(j < 12)||(i+j+21 > nread))
// 							continue;
// 						memset(Longitude,0,sizeof(Longitude));
// 						memcpy(Longitude,nmea+i+21,j);
// 						//memcpy(Longitude,nmea+i+20,3);
// 						//Longitude[3] = '.';
// 						//memcpy(Longitude+4,nmea+i+20+3,2);
// 						//memcpy(Longitude+5,nmea+i+20+3+3,j-6);
// 						/*bzero(b,sizeof(b));
// 						memcpy(b,nmea+i+20,sizeof(b));
// 						du = atof(b);
// 						sscanf(nmea+i+20+3,"%d.%d,",&fen,&miao);
// 						gps = du+fen/60.0+miao/60.0/powa(j-6);
// 						sprintf(Longitude,"%lf",gps);*/
// 						//printf( "N %s\n", Latitude); //输出所读取的数据
// 						//printf( "E %s\n", Longitude); //输出所读取的数据
// //						printf("1????????????N %s\n", Latitude); //输出所读取的数据
// //						printf("1????????????E %s\n", Longitude); //输出所读取的数据
// 						break;
// 					}
// 					else if((nmea[i] == '$')&&(nmea[i+4] == 'M')&&(nmea[i+5] == 'C')&&(nmea[i+18] == 'A'))
// 					{
// 						if(i + 44 > 1024)
// 							continue;
// 						for(j = 0; j < 11; j ++)
// 						{
// 							if(nmea[i + j+20] == ',')
// 								break;
// 						}
// 						if((j == 0)||(j < 11)||(i + j+20 > nread))
// 							continue;
// 						for(k = 0; k < j;k ++)
// 						{
// 							if(((nmea[k+i+20]<'0')||(nmea[k+i+20]>'9'))&&(nmea[k+i+20]!='.'))
// 							{
// 								//printf("gsp mc %c\n",nmea[k+i+19]);
// 								break;
// 							}
// 						}
// 						if(k != j)
// 							continue;
// 						memset(Latitude,0,sizeof(Latitude));
// 						memcpy(Latitude,nmea+i+20,j);
// 						//memcpy(Latitude,nmea+i+19,2);
// 						//Latitude[2] = '.';
// 						//memcpy(Latitude+3,nmea+i+19+2,2);
// 						//memcpy(Latitude+5,nmea+i+19+2+3,j-5);
// 						/*bzero(a,sizeof(a));
// 						memcpy(a,nmea+i+19,sizeof(a));
// 						du = atof(a);
// 						sscanf(nmea+i+19+2,"%d.%d,",&fen,&miao);
// 						gps = du+fen/60.0+miao/60.0/powa(j-5);
// 						sprintf(Latitude,"%lf",gps);*/
// 						for(j = 0; j < 12; j ++)
// 						{
// 							if(nmea[i + j+34] == ',')
// 								break;
// 						}
// 						if((j == 0)||(j < 12)||(i + j+34 > nread))
// 							continue;
// 						memset(Longitude,0,sizeof(Longitude));
// 						memcpy(Longitude,nmea+i+34,j);
// 						//memcpy(Longitude,nmea+i+32,3);
// 						//Longitude[3] = '.';
// 						//memcpy(Longitude+4,nmea+i+32+3,2);
// 						//memcpy(Longitude+6,nmea+i+32+3+3,j-6);
// 						/*bzero(b,sizeof(b));
// 						memcpy(b,nmea+i+32,sizeof(b));
// 						du = atof(b);
// 						sscanf(nmea+i+32+3,"%d.%d,",&fen,&miao);
// 						gps = du+fen/60.0+miao/60.0/powa(j-6);
// 						sprintf(Longitude,"%lf",gps);*/
// 						//printf( "N %s\n", Latitude); //输出所读取的数据
// 						//printf( "E %s\n", Longitude); //输出所读取的数据
// //						printf("2????????????N %s\n", Latitude); //输出所读取的数据
// //						printf("2????????????E %s\n", Longitude); //输出所读取的数据
// 						break;
// 					}

// 				}

// 			}
// 			break;
// 			/*if(setled[1] == 0x8d)
// 			 {
// 			 setled[1] += 0x10;
// 			 }
// 			 else
// 			 setled[1] += 1;
// 			 if(setled[1] == 0xdd)
// 			 setled[1]
// 			 write(nmeaFd,setled,sizeof(setled));*/
// 		}
// 	}
	//close(atFd);
	// close(nmeaFd);
	// return;
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
