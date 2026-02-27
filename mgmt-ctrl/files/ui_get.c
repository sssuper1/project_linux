/*
 * @Author: SunDG311 sdg18252543282@163.com
 * @Date: 2025-07-25 11:42:28
 * @LastEditors: SunDG311 sdg18252543282@163.com
 * @LastEditTime: 2025-11-18 16:50:00
 * @FilePath: \files\ui_get.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/* get param from uart  
Description： 通过串口从UI界面获取配置指令
1.mgmt代理接收串口指令，并解析进行配置
2.实现同步显示
3.
*/

#include "mgmt_types.h" 
#include "ui_get.h"
#include "enum_uartparam_addr.h"
#include <stdbool.h>
#include "sqlite_unit.h"
#include "mgmt_netlink.h"
#include "gpsget.h"
#include "socketUDP.h"
//global struct
extern int ui_fd;


extern Global_Radio_Param g_radio_param;
extern uint8_t SELFID;
extern GPS_INFO gps_info_uart;
extern uint32_t FREQ_INIT;
Info_0x06_Statistics stat_info;

//int 转 小端
uint32_t int_to_little_endian(uint32_t value) {
    return ((value & 0x000000FF) << 24) |  // 最低字节移到最高位
           ((value & 0x0000FF00) << 8)  |  // 次低字节左移 8 位
           ((value & 0x00FF0000) >> 8)  |  // 次高字节右移 8 位
           ((value & 0xFF000000) >> 24);   // 最高字节移到最低位
}

uint16_t CRC_Check(uint8_t* CRC_Ptr, uint16_t LEN)
{
    uint16_t CRC_Value = 0xffff;
    for(int i=0;i<LEN;i++)
    {
        CRC_Value ^= CRC_Ptr[i];
        for(int j=0;j<8;j++)
        {
            if(CRC_Value & 0x0001)
                CRC_Value = (CRC_Value>>1)^0xA001;
            else
                CRC_Value = (CRC_Value >> 1);
        }
    }

    CRC_Value = ((CRC_Value>>8)  |  (CRC_Value << 8));
    return CRC_Value;
}


/* 0x04命令 */
int8_t Send_0x04(int fd,void* info,int size)
{

	int i;
	// uint8_t bw_mapping_table[]={4,3,2,1};
	Frame_04_byte frame_send ;
	memset(&frame_send,0,sizeof(Frame_04_byte));
	int BYTE04SIZE=sizeof(Frame_04_byte);

	
	uint8_t send_byte_stream[BYTE04SIZE] ;
	uint8_t byte_stream_recv[9] ;

	Node_Xwg_Pairs *param_pairs=(Node_Xwg_Pairs *)malloc(size);
	memcpy(param_pairs,(Node_Xwg_Pairs*)info,size);


	frame_send.head = htons(0xD55D);
	frame_send.dirrection = 0x04;
	frame_send.message_type = 0xFF;

	frame_send.current_work_mode = (uint8_t)get_int_value((void*)param_pairs,"macmode");      // 当前工作模式 0:宽带电台
	// printf("[UI DEBUG] 0x04 work mode %d \r\n",get_int_value((void*)param_pairs,"workmode"));
	//frame_send.cancellation_state = 0x01;     // 对消状态
	//printf("[UI DEBUG] 0x04 kylb %d \r\n",get_int_value((void*)param_pairs,"workmode"));
	frame_send.spatial_filter = 0x01;        	// 空域滤波 0：开启 1：关闭
	if(get_int_value((void*)param_pairs,"kylb")==KYLB_MODE_OPEN)
	{
		frame_send.spatial_filter = 0;      
	}
	// frame_send.co_location_mode = 0x01;       // 共址模式
	if(get_int_value((void*)param_pairs,"workmode")==WORK_MODE_TYPE_DP)
	{
		
		frame_send.Channel1.hopping_mode = 0x00; // 跳频方式 定频
	}
	if(get_int_value((void*)param_pairs,"workmode")==WORK_MODE_TYPE_ZSYXP)
	{
		frame_send.Channel1.hopping_mode = 1;      // 跳频方式 自适应选频
	}

	// frame_send.service_mode = 0x00;           // 业务模式
	// frame_send.remote_param_modify = 0x00;    // 远程参数修改

	// printf("[UI DEBUG] 0x04 router %d \r\n",get_int_value((void*)param_pairs,"router"));

	frame_send.Channel1.route_protocol = (uint8_t)get_int_value((void*)param_pairs,"router")-1;  // 路由协议
	frame_send.Channel1.access_protocol = 0x0;// 接入协议
	// frame_send.Channel1.time_reference = 0x01;// 时间基准
   
		
	// printf("[UI DEBUG] 0x04 freq %d \r\n",get_int_value((void*)param_pairs,"channel"));

    frame_send.Channel1.center_freq    = get_int_value((void*)param_pairs,"channel")*1000;// 中心频率（点频） *1000
    frame_send.Channel1.select_freq_1  = get_int_value((void*)param_pairs,"select_freq1")*1000;// 自适应-中心频率1 *1000
    frame_send.Channel1.select_freq_2  = get_int_value((void*)param_pairs,"select_freq2")*1000;// 自适应-中心频率2 *1000
    frame_send.Channel1.select_freq_3  = get_int_value((void*)param_pairs,"select_freq3")*1000;// 自适应-中心频率3 *1000
    frame_send.Channel1.select_freq_4  = get_int_value((void*)param_pairs,"select_freq4")*1000;// 自适应-中心频率4 *1000


    // frame_send.Channel1.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel1.hopping_points = 0x01;// 跳频频点个数

	// printf("[UI DEBUG] 0x04 bw %d \r\n",get_int_value((void*)param_pairs,"bw"));
    frame_send.Channel1.signal_bw = (uint8_t)get_int_value((void*)param_pairs,"bw");		// 信号带宽
    // frame_send.Channel1.mod_narrow = 0;// 窄带调制方式

	// printf("[UI DEBUG] 0x04 mcs %d \r\n",get_int_value((void*)param_pairs,"mcs"));
	frame_send.Channel1.mod_wide =(uint8_t)get_int_value((void*)param_pairs,"mcs");
	frame_send.sync_mode=(uint8_t)get_int_value((void*)param_pairs,"sync_mode");

    // frame_send.Channel1.mod_wide = param.g_rate;// 宽带调制方式
    // frame_send.Channel1.coding_rate = 0x0;// 编码效率（0.0~1.0）
    // frame_send.Channel1.tx_power_narrow = 0;// 窄带发射功率
    // frame_send.Channel1.tx_power_mimo = 0;// MIMO发射功率
    frame_send.Channel1.tx_power_spread = 0;// 发射功率
    frame_send.Channel1.tx_attenuation = 0;// 发射功率衰减


	
    // frame_send.Channel2.center_freq  = 30*1000;// 中心频率（点频） *1000
    // frame_send.Channel2.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel2.hopping_points = 0x01;// 跳频频点个数
    // frame_send.Channel3.center_freq  = 30*1000;// 中心频率（点频） *1000
    // frame_send.Channel3.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel3.hopping_points = 0x01;// 跳频频点个数
    // frame_send.Channel4.center_freq  = 30*1000;// 中心频率（点频） *1000
    // frame_send.Channel4.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel4.hopping_points = 0x01;// 跳频频点个数


	// frame_send.start_freq=htons(0x1E00);
	// frame_send.end_freq=htons(0xC409);


    frame_send.check = htons(CRC_Check(&frame_send.dirrection, BYTE04SIZE-6));					//校验
    //printf("crc = %X\n",CRC_Check(&frame_send.dirrection, BYTE04SIZE-6));
	frame_send.tail = htons(0x5DD5);
	memcpy(send_byte_stream,&frame_send,BYTE04SIZE);

	write(fd,(void*)send_byte_stream,BYTE04SIZE);
	sleep(1);
	// printf("0x04 cmd len %d \r\n",write(fd,(void*)send_byte_stream,BYTE04SIZE));
	//  printf("send 0x04 cmd,len:%d \r\n",BYTE04SIZE);
	// for(i=0;i<BYTE04SIZE;i++)
	// {
	// 	printf("%02X",send_byte_stream[i]);
	// }
	// printf("\r\n");
	// sleep(2);
	// uint8_t bytes_read=read(fd,byte_stream_recv,9);
	// for(int i=0;i<9;i++)
	// {
	// 	printf(" %#x ",byte_stream_recv[i]);
	// }
	// printf("\r\n");
	// if(bytes_read <= 0)
	// 	return -1;
	// if(byte_stream_recv[3] == MESSAGE_TYPE_REPLY)
	// {
	// 	printf("recv 0x04 info success\r\n");
	// 	return 1;

	// }
	// else if(byte_stream_recv[3] == MESSAGE_TYPE_ERROR)
	// 	return -1;

}

/* 0x05命令 */
int8_t Send_0x05(int fd,void* info)
{
	int i;

	double t_lon,t_lat;
	uint16_t t_alit;

	Frame_05_byte frame_send ;
	int BYTE05SIZE=sizeof(Frame_05_byte);
	memset(&frame_send,0,BYTE05SIZE);
	
	uint8_t send_byte_stream[BYTE05SIZE] ;

	Global_Radio_Param param;
	memcpy(&param,(Global_Radio_Param*)info,sizeof(Global_Radio_Param));
	// Node_Xwg_Pairs *param_pairs=(Node_Xwg_Pairs *)malloc(size);
	// memcpy(param_pairs,(Node_Xwg_Pairs*)info_2,size);

	// t_lon=get_double_value(param_pairs,"longitude");
	// t_lat=get_double_value(param_pairs,"latitude ");
	// t_alit=get_int_value(param_pairs,"altitude");

	// printf("longitude:%lf,latitude:%lf ,altitude:%d \r\n",t_lon,t_lat,t_alit);




	frame_send.head = htons(0xD55D);
	frame_send.dirrection = 0x05;
	frame_send.message_type = 0xFF;
	
	 // 设备ID
	frame_send.value.device_id = SELFID;             

	// 设备名称
	char device_name[10];
	memset(device_name,0,10);
	sprintf(frame_send.value.device_name, "kddt-%d", SELFID); 		

	// 业务网口IP地址
	frame_send.value.service_ip[0]=192;
	frame_send.value.service_ip[1]=168;
	frame_send.value.service_ip[2]=2;
	frame_send.value.service_ip[3]=SELFID;


	frame_send.value.service_port = 6000;           // 业务网口端口号
	//memcpy(frame_send.value.management_ip,"192.168.2.1", sizeof(int));          // 管理网口IP地址
	// frame_send.value.management_port = 6000;        // 管理网口端口号
	//memcpy(frame_send.value.sensing_ip[4],a, sizeof(a));             // 感知网口IP地址
	// frame_send.value.sensing_port = 6000;           // 感知网口端口号
	frame_send.value.serial_baudrate = 4;        // 串口波特率
	frame_send.value.serial_databits = 3;        // 串口数据位
	frame_send.value.serial_stopbits = 0;        // 串口停止位
	frame_send.value.serial_parity = 0;          // 串口校验位
	frame_send.value.serial_flowctrl = 0;        // 串口流控

	frame_send.value.gps_auto_sync=0;			// 位置设置-自动获取
	sprintf(frame_send.value.longitude, "%.6f", gps_info_uart.lon);  // 经度
	sprintf(frame_send.value.latitude, "%.6f", gps_info_uart.lat);   // 纬度
	frame_send.value.altitude = gps_info_uart.gaodu*10;               // 高度
	// printf("longitude:%lf,latitude:%lf ,altitude:%d \r\n",atof(frame_send.value.longitude),atof(frame_send.value.latitude),frame_send.value.altitude/10);

	frame_send.value.time_auto_sync = 0;         // 时间自动获取
	frame_send.value.manual_hour = gps_info_uart.bj_time[0];            // 手动设置时
	frame_send.value.manual_minute = gps_info_uart.bj_time[1];          // 手动设置分
	frame_send.value.manual_second = gps_info_uart.bj_time[2];          // 手动设置秒
	frame_send.check = htons(CRC_Check(&frame_send.dirrection, BYTE05SIZE-6));					//校验
	frame_send.tail = htons(0x5DD5);
	memcpy(send_byte_stream,&frame_send,BYTE05SIZE);

	write(fd,(void*)send_byte_stream,BYTE05SIZE);
	// printf("[UART] CMD 0x05 %d \r\n",write(fd,(void*)send_byte_stream,BYTE05SIZE));
	sleep(1);

	// printf("send 0x05 cmd,len:%d\r\n",BYTE05SIZE);
	// for(i=0;i<BYTE05SIZE;i++)
	// {
	// 	printf("%02X",send_byte_stream[i]);
	// }
	// printf("\r\n");
	// uint8_t byte_stream_recv[9] = {0};
	// int bytes_read = read(fd,byte_stream_recv,9);
	// for(int i=0;i<9;i++)
	// {
	// 	printf(" %#x ",byte_stream_recv[i]);
	// }
	// printf("\r\n");

	// if(bytes_read <= 0)
	// 	return -1;
	// if(byte_stream_recv[3] == 0x00)
	// {
	// 	printf("recv 0x05 cmd success\r\n");
	// 	return 1;

	// }
	// else if(byte_stream_recv[3] == 0x01)
	// 	return -1;
}

/* 0x06命令 */
int8_t Send_0x06(int fd,void* buf)
{
	 int i=0;
	uint16_t BYTE06SIZE=sizeof(Frame_06_byte);

	Frame_06_byte frame_send ;
	memset(&frame_send,0,BYTE06SIZE);

	uint8_t send_byte_stream[BYTE06SIZE] ;

	static uint16_t s_rx_packet=0;
	static uint16_t s_tx_packet=0; 
	Info_0x06_Statistics *info=(Info_0x06_Statistics*)buf;
	//get_interface_stats(info);  //后台一直更新计数值

	if(info->stat_flag==1)
	{
		// 停止计数
		return 0;
	}

	else if(info->stat_flag==2)   //清零
	{
		get_interface_stats(info);
		s_rx_packet=info->eth_rx_packets;
		s_tx_packet=info->eth_tx_packets;
		// printf("[UI DEBUG] static rx %d tx %d \r\n",s_rx_packet,s_tx_packet);
		stat_info.stat_flag=0;
		
		stat_info.audio_rx_packets=stat_info.audio_tx_packets=0;
	}
	get_interface_stats(info);

	frame_send.head = htons(0xD55D);
	frame_send.dirrection = 0x06;
	frame_send.message_type = 0xFF;
	frame_send.value.eth_tx_cnt = info->eth_tx_packets-s_tx_packet;      // 以太网消息发送个数
	frame_send.value.eth_rx_cnt = info->eth_rx_packets-s_rx_packet;      // 以太网消息接收个数
	frame_send.value.voice_tx_cnt = info->audio_tx_packets;      		 // 模拟话音发送个数
	frame_send.value.voice_rx_cnt = info->audio_rx_packets;     		 // 模拟话音接收个数
	frame_send.value.total_tx_cnt = frame_send.value.eth_tx_cnt+frame_send.value.voice_tx_cnt;      // 总发送消息个数
	frame_send.value.total_rx_cnt = frame_send.value.eth_rx_cnt+frame_send.value.voice_rx_cnt;      // 总接收消息个数


	// frame_send.value.msg1_tx_cnt = 3500;       // 消息类型1发送个数
	// frame_send.value.msg1_rx_cnt = 3500;       // 消息类型1接收个数
	// frame_send.value.msg2_tx_cnt = 4000;       // 消息类型2发送个数
	// frame_send.value.msg2_rx_cnt = 4000;       // 消息类型2接收个数
	// frame_send.value.msg3_tx_cnt = 5500;       // 消息类型3发送个数
	// frame_send.value.msg3_rx_cnt = 5500;       // 消息类型3接收个数
	// frame_send.value.msg4_tx_cnt = 6000;       // 消息类型4发送个数
	// frame_send.value.msg4_rx_cnt = 6000;       // 消息类型4接收个数
	// frame_send.value.msg5_tx_cnt = 7000;       // 消息类型5发送个数
	// frame_send.value.msg5_rx_cnt = 7000;       // 消息类型5接收个数

	frame_send.check = htons(CRC_Check(&frame_send.dirrection, BYTE06SIZE-6));					//校验
	frame_send.tail = htons(0x5DD5);

	//i++;

	memcpy(send_byte_stream,&frame_send,BYTE06SIZE);

	write(fd,(void*)send_byte_stream,BYTE06SIZE);
	// printf("send 0x06 cmd,len:%d\r\n",BYTE06SIZE);
	// for(i=0;i<BYTE06SIZE;i++)
	// {
	// 	printf("%02X",send_byte_stream[i]);
	// }
	// printf("\r\n");
	sleep(1);
	return 0;
	//uint8_t byte_stream_recv[9] = {0};
	// int  bytes_read = read(fd,byte_stream_recv,9);

	// for(int i=0;i<9;i++)
	// {
	// 	printf(" %#x ",byte_stream_recv[i]);
	// }
	// printf("\r\n");

	// if(bytes_read <= 0)
	// 	return -1;
	// if(byte_stream_recv[3] == MESSAGE_TYPE_REPLY)
	// {
	// 	printf("recv 0x06 cmd succcess\r\n");
	// 	return 1;

	// }
	// else if(byte_stream_recv[3] == MESSAGE_TYPE_ERROR)
	// 	return -1;
}


int8_t Send_0x07(int fd,void* info)
{
	int i=0;
	Frame_07_byte frame_send ;
	int BYTE07SIZE=sizeof(Frame_07_byte);
	memset(&frame_send,0,BYTE07SIZE);

	uint8_t send_byte_stream[BYTE07SIZE] ;

	DEVICE_SC_STATUS_REPORT amp_param;
	memcpy(&amp_param,(DEVICE_SC_STATUS_REPORT*)info,sizeof(DEVICE_SC_STATUS_REPORT));
	
	frame_send.head = htons(0xD55D);
	frame_send.dirrection = 0x07;
	frame_send.message_type = 0xFF;
	frame_send.value.self_test_status = 0;                     // 自检状态
    frame_send.value.battery_remaining_capacity = amp_param.battery_level;           // 电池剩余电量
    frame_send.value.battery_cycle_count = 1000;                  // 电池剩余循环次数
    frame_send.value.battery_self_test_status = amp_param.battery_self_test;             // 电池自检状态
    frame_send.value.info_processor_temp = amp_param.temperature;                  // 综合信息处理温度
    frame_send.value.fan_speed_status = amp_param.fan_status;                     // 风机转速状态
    frame_send.value.nav_lock_status = amp_param.nav_lock_status;                      // 卫导锁定状态
    frame_send.value.clock_selection = amp_param.sync_status;                      // 时钟选择状态
    frame_send.value.adc_status = amp_param.sense_adc_status;                           // ADC状态
    frame_send.value.clock_source_temp = amp_param.freq_temperature;                    // 时钟频率源温度
    frame_send.value.freq_word_send_count = amp_param.freq_word_count;                 // 频率字下发次数计数
    frame_send.value.comm_sensing_status = amp_param.rf_sense_status;                  // 通信感知状态
    frame_send.value.ref_clock1_status = 0x01;                    // 输出参考时钟1状态
    frame_send.value.ref_clock2_status = 0x01;                    // 输出参考时钟2状态
    frame_send.value.lo1_output_status = (amp_param.freq_lo_ready >> 0) & 0x01;                    // 本振1输出状态
    frame_send.value.lo2_output_status = (amp_param.freq_lo_ready >> 1) & 0x01;                    // 本振2输出状态
    frame_send.value.lo3_output_status = (amp_param.freq_lo_ready >> 2) & 0x01;                    // 本振3输出状态
    frame_send.value.lo4_output_status = (amp_param.freq_lo_ready >> 3) & 0x01;                    // 本振4输出状态
    frame_send.value.power_conversion_temp = amp_param.power_temperature;                // 电源变换温度
    frame_send.value.power_on_fault_indicator = amp_param.power_power_fault;             // 上电/故障指示
    frame_send.value.power_supply_control = amp_param.power_ch_power_status;                 // 各路加电控制
    frame_send.value.ac220_power_consumption = amp_param.power_ac220_power;              // AC220功耗
    frame_send.value.dc24v_power_consumption = amp_param.power_dc24v_power;              // DC24V功耗

    frame_send.value.rf_channel_temp = amp_param.rf_ch1_temp1;                          // 通道1温度
    frame_send.value.rf_channel_tx_power_status = amp_param.rf_tx_power_status;               // 发射功率检波状态
    frame_send.value.rf_channel_antenna_l_vswr = amp_param.rf_antenna_l_vswr;                // 天线L口驻波状态
    frame_send.value.rf_channel_antenna_h1_vswr = amp_param.rf_antenna_h1_vswr;               // 天线H-1口驻波状态
    frame_send.value.rf_channel_antenna_h2_vswr =  amp_param.rf_antenna_h2_vswr;               // 天线H-2口驻波状态
    frame_send.value.rf_channel_tx_rx_status = amp_param.rf_tx_rx_status;                  // 收发状态指示
    frame_send.value.rf_Channel_antenna_select = amp_param.rf_antenna_select;                // 天线选择控制状态
    frame_send.value.rf_channel_comm_sensing = amp_param.rf_sense_status;                  // 通信感知状态
    frame_send.value.rf_channel_power_control = amp_param.power_ch_power_status;                 // 加电控制状态
    frame_send.value.rf_channel_power_level = amp_param.rf_ch_power_level;                   // 通道功率等级

    frame_send.Channel1.rf_channel_rf_power_detect = amp_param.rf_ch1_rf_power;               // 射频功率检波
    frame_send.Channel1.rf_channel_if_power_detect = amp_param.rf_ch1_if_power;               // 中频功率检波
    frame_send.Channel1.rf_channel_current_freq = amp_param.rf_ch1_freq;                  // 当前频点
    frame_send.Channel1.rf_channel_agc_attenuation = amp_param.rf_ch1_agc_atten;               // AGc衰减值
    frame_send.Channel1.rf_channel_signal_bandwidth = amp_param.rf_ch1_bandwidth;              // 通道1信号带宽
    frame_send.Channel1.rf_channel_attenuation = amp_param.rf_ch1_agc_atten;                   // 通道1衰减量

	frame_send.Channel2.rf_channel_rf_power_detect = amp_param.rf_ch2_rf_power;               // 射频功率检波
	frame_send.Channel2.rf_channel_if_power_detect = amp_param.rf_ch2_if_power;               // 中频功率检波
	frame_send.Channel2.rf_channel_current_freq = amp_param.rf_ch2_freq;                  // 当前频点
	frame_send.Channel2.rf_channel_agc_attenuation = amp_param.rf_ch2_agc_atten;               // AGc衰减值
	frame_send.Channel2.rf_channel_signal_bandwidth = amp_param.rf_ch2_bandwidth;              // 通道1信号带宽
	frame_send.Channel2.rf_channel_attenuation = amp_param.rf_ch2_agc_atten;                   // 通道1衰减量

	frame_send.Channel3.rf_channel_rf_power_detect = amp_param.rf_ch3_rf_power;               // 射频功率检波
	frame_send.Channel3.rf_channel_if_power_detect = amp_param.rf_ch3_if_power;               // 中频功率检波
	frame_send.Channel3.rf_channel_current_freq = amp_param.rf_ch3_freq;                  // 当前频点
	frame_send.Channel3.rf_channel_agc_attenuation = amp_param.rf_ch3_agc_atten;               // AGc衰减值
	frame_send.Channel3.rf_channel_signal_bandwidth = amp_param.rf_ch3_bandwidth;              // 通道1信号带宽
	frame_send.Channel3.rf_channel_attenuation = amp_param.rf_ch3_agc_atten;                   // 通道1衰减量

	frame_send.Channel4.rf_channel_rf_power_detect = amp_param.rf_ch4_rf_power;               // 射频功率检波
	frame_send.Channel4.rf_channel_if_power_detect = amp_param.rf_ch4_if_power;               // 中频功率检波
	frame_send.Channel4.rf_channel_current_freq = amp_param.rf_ch4_freq;                  // 当前频点
	frame_send.Channel4.rf_channel_agc_attenuation = amp_param.rf_ch4_agc_atten;               // AGc衰减值
	frame_send.Channel4.rf_channel_signal_bandwidth = amp_param.rf_ch4_bandwidth;              // 通道1信号带宽
	frame_send.Channel4.rf_channel_attenuation = amp_param.rf_ch4_agc_atten;                   // 通道1衰减量

	frame_send.check = htons(CRC_Check(&frame_send.dirrection, BYTE07SIZE-6));					//校验
	frame_send.tail = htons(0x5DD5);
	memcpy(send_byte_stream,&frame_send,BYTE07SIZE);
	
	
	write(fd,(void*)send_byte_stream,BYTE07SIZE);
	// printf("send 0x07 cmd len:%d\r\n",BYTE07SIZE);
	// for(i=0;i<BYTE07SIZE;i++)
	// {
	// 	printf("%02X",send_byte_stream[i]);
	// }
	// printf("\r\n");
	// printf("[UART] cmd 0x07 %d \r\n",write(fd,(void*)send_byte_stream,BYTE07SIZE));
	sleep(1);

	// uint8_t byte_stream_recv[9] = {0};
	// int bytes_read = read(fd,byte_stream_recv,9);
	// for(int i=0;i<9;i++)
	// {
	// 	printf(" %#x ",byte_stream_recv[i]);
	// }
	// printf("\r\n");

	// if(bytes_read <= 0)
	// 	return -1;
	// if(byte_stream_recv[3] == 0x00)
	// {
	// 	printf("recv 0x07 cmd success\r\n");
	// 	return 1;

	// }
}

int8_t Send_0x08(int fd,void* info,int size)
{
	int i;
	// uint8_t bw_mapping_table[]={4,3,2,1};
	Frame_08_byte frame_send ;
	memset(&frame_send,0,sizeof(Frame_08_byte));
	int BYTE08SIZE=sizeof(Frame_08_byte);

	
	uint8_t send_byte_stream[BYTE08SIZE] ;
	uint8_t byte_stream_recv[9] ;

	Node_Xwg_Pairs *param_pairs=(Node_Xwg_Pairs *)malloc(size);
	memcpy(param_pairs,(Node_Xwg_Pairs*)info,size);


	frame_send.head = htons(0xD55D);
	frame_send.dirrection = 0x08;
	frame_send.message_type = 0xFF;
	memcpy(frame_send.current_time,gps_info_uart.bj_time,3) ;
	frame_send.net_join_state=0;
	
	//frame_send.cancellation_state = 0x01;     // 对消状态
	//printf("[UI DEBUG] 0x04 kylb %d \r\n",get_int_value((void*)param_pairs,"workmode"));
	
	frame_send.kylb = 0x01;        	// 空域滤波 0：开启 1：关闭
	if(get_int_value((void*)param_pairs,"kylb")==KYLB_MODE_OPEN)
	{
		frame_send.kylb = 0;     
		// frame_send.workmode = 0x00; 
	}

	// printf("[UI DEBUG] 08-kylb %d \r\n ",frame_send.kylb);
	// frame_send.co_location_mode = 0x01;       // 共址模式
	if(get_int_value((void*)param_pairs,"workmode")==WORK_MODE_TYPE_DP)
	{
		
		frame_send.workmode = 0x00; // 跳频方式 定频
	}
	if(get_int_value((void*)param_pairs,"workmode")==WORK_MODE_TYPE_ZSYXP)
	{
		frame_send.workmode = 1;      // 跳频方式 自适应选频
	}

	// frame_send.sync_mode = 0x00;              // 内外同步模式
	// frame_send.service_mode = 0x00;           // 业务模式
	// frame_send.remote_param_modify = 0x00;    // 远程参数修改

	// printf("[UI DEBUG] 0x08 router %d \r\n",get_int_value((void*)param_pairs,"router"));

	// frame_send.Channel1.route_protocol = (uint8_t)get_int_value((void*)param_pairs,"router")-1;  // 路由协议
	// frame_send.Channel1.access_protocol = 0x00;// 接入协议
	// frame_send.Channel1.time_reference = 0x01;// 时间基准
   
		
	// printf("[UI DEBUG] 0x08 freq %d \r\n",get_int_value((void*)param_pairs,"channel"));

    frame_send.center_freq    = get_int_value((void*)param_pairs,"channel")*1000;// 中心频率（点频） *1000
    frame_send.select_freq1  = get_int_value((void*)param_pairs,"select_freq1")*1000;// 自适应-中心频率1 *1000
    frame_send.select_freq2  = get_int_value((void*)param_pairs,"select_freq2")*1000;// 自适应-中心频率2 *1000
    frame_send.select_freq3  = get_int_value((void*)param_pairs,"select_freq3")*1000;// 自适应-中心频率3 *1000
    frame_send.select_freq4  = get_int_value((void*)param_pairs,"select_freq4")*1000;// 自适应-中心频率4 *1000
	
	frame_send.sync_mode =  (uint8_t)get_int_value((void*)param_pairs,"sync_mode");
	// printf("[UI DEBUG] 08-syncmode %d \r\n ",frame_send.sync_mode);
    // frame_send.Channel1.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel1.hopping_points = 0x01;// 跳频频点个数

	// printf("[UI DEBUG] 0x04 bw %d \r\n",get_int_value((void*)param_pairs,"bw"));
    frame_send.bw = (uint8_t)get_int_value((void*)param_pairs,"bw");		// 信号带宽
    // frame_send.Channel1.mod_narrow = 0;// 窄带调制方式

	// printf("[UI DEBUG] 0x04 mcs %d \r\n",get_int_value((void*)param_pairs,"mcs"));
	frame_send.mcs =(uint8_t)get_int_value((void*)param_pairs,"mcs");
    // frame_send.Channel1.mod_wide = param.g_rate;// 宽带调制方式
    // frame_send.Channel1.coding_rate = 0x0;// 编码效率（0.0~1.0）
    // frame_send.Channel1.tx_power_narrow = 0;// 窄带发射功率
    // frame_send.Channel1.tx_power_mimo = 0;// MIMO发射功率
    frame_send.tx_power_spread = 0;// 发射功率
    frame_send.tx_attenuation = 0;// 发射功率衰减


	
    // frame_send.Channel2.center_freq  = 30*1000;// 中心频率（点频） *1000
    // frame_send.Channel2.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel2.hopping_points = 0x01;// 跳频频点个数
    // frame_send.Channel3.center_freq  = 30*1000;// 中心频率（点频） *1000
    // frame_send.Channel3.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel3.hopping_points = 0x01;// 跳频频点个数
    // frame_send.Channel4.center_freq  = 30*1000;// 中心频率（点频） *1000
    // frame_send.Channel4.hopping_pattern = 0x01;// 跳频图样号
    // frame_send.Channel4.hopping_points = 0x01;// 跳频频点个数


	// frame_send.start_freq=htons(0x1E00);
	// frame_send.end_freq=htons(0xC409);


    frame_send.check = htons(CRC_Check(&frame_send.dirrection, BYTE08SIZE-6));					//校验
    //printf("crc = %X\n",CRC_Check(&frame_send.dirrection, BYTE04SIZE-6));
	frame_send.tail = htons(0x5DD5);
	memcpy(send_byte_stream,&frame_send,BYTE08SIZE);

	write(fd,(void*)send_byte_stream,BYTE08SIZE);
	sleep(1);
	// printf("send 0x08 cmd len:%d\r\n",BYTE08SIZE);
	// for(i=0;i<BYTE08SIZE;i++)
	// {
	// 	printf("%02X",send_byte_stream[i]);
	// }
	// printf("\r\n");	
	
}

int8_t Send_0x09(int fd,void* info)
{

	int neigh_num=0;
	int size=0;
	int i;

	// char test_buf[]={0xD5,0x5D,0x09,0xFF,0x01,0x05,0xC0,0xA8,0x01,0x01,
	// 	0x01,0x20,0x70,0x17,0x45,0x31,0x32,0x2E,0x33,0x34,0x35,0x36,0x00,
	// 	0x00,0x4E,0x31,0x32,0x2E,0x33,0x34,0x35,0x36,0x00,0x00,0x50,0x00,0x44,0xE4,0x5D,0xD5};

	int NODESTATUSSIZE = sizeof(NetworkNodeStatus);
	int BYTE0ASIZE=sizeof(Frame_0A_byte);

	// Node_Xwg_Pairs params[] = {
    //     {"channel", 0, 0},{"power", 0, 0},{"bw", 0, 0},{"mcs", 0, 0},
    //     {"macmode", 0, 0},{"slotlen", 0, 0},{"router", 0, 0},{"workmode", 0, 0}
    // };

	//  read_node_xwg_file("/etc/node_xwg",params,8);


	struct mgmt_send self_msg;
	//memset((void*)&self_msg,0,sizeof(mgmt_send));

	memcpy((void*)&self_msg,(struct mgmt_send*)info,sizeof(self_msg));

	neigh_num=self_msg.neigh_num;

	if(neigh_num==0||neigh_num>4)
	{
		return 0;
	}
	//printf("[UART] neigh num %d ",neigh_num);
	size=NODESTATUSSIZE*neigh_num;
	//printf("size %d \r\n",size);

	NetworkNodeStatus *net_info=(NetworkNodeStatus*)malloc(size);
	memset(net_info,0,size);
	uint8_t send_byte_stream[size+9] ;

	for(int i=0;i<neigh_num;i++)
	{
		net_info[i].member_id=self_msg.msg[i].node_id;
		net_info[i].ip_address[0]=192;
		net_info[i].ip_address[1]=168;
		net_info[i].ip_address[2]=2;
		net_info[i].ip_address[3]=self_msg.msg[i].node_id;
	
		//sprintf(net_info[i].ip_address,"192.168.2.%d",self_msg.msg[i].node_id);
		net_info[i].hop_count=1;
		net_info[i].signal_strength=0-self_msg.msg[i].rssi;
		net_info[i].transmission_delay=self_msg.msg[i].time_jitter;
		// net_info[i].longitude="100.101";
		// net_info[i].latitude="23.123";
		// net_info[i].altitude="1000";

//update 0x0a info;
		// Frame_0A_byte frame_0A=init_0x0A_info(self_msg.msg[i].node_id,(void*)&params,sizeof(Node_Xwg_Pairs)*8);
		// printf("send  0x0a cmd len %d \r\n",write(fd,(char*)&frame_0A,BYTE0ASIZE));

	}



	send_byte_stream[0]=0xD5;		
	send_byte_stream[1]=0x5D;		
	send_byte_stream[2]=0x09;		
	send_byte_stream[3]=0xFF;		
	send_byte_stream[4]=neigh_num;		
	 

	memcpy(send_byte_stream+5,net_info,size);
	uint16_t check = CRC_Check(send_byte_stream+2, size+3);
	send_byte_stream[size+5]=check >> 8;
	send_byte_stream[size+6]=check & 0xff;
	send_byte_stream[size+7]=0x5d;
	send_byte_stream[size+8]=0xd5;


	int cmd_len=write(fd,(void*)send_byte_stream,size+9);
	//int cmd_len=write(fd,(void*)test_buf,sizeof(test_buf));

	
	// printf("send 0x09 cmd len-%d\r\n",cmd_len);
	// for(i=0;i<sizeof(send_byte_stream);i++)
	// {
	// 	printf("%02X",send_byte_stream[i]);
	// }
	// printf("\r\n");
	sleep(3);
	// uint8_t byte_stream_recv[9] = {0};
	// int bytes_read = read(fd,byte_stream_recv,9);
	// for(int i=0;i<9;i++)
	// {
	// 	printf(" %#x ",byte_stream_recv[i]);
	// }
	// printf("\r\n");

	// if(bytes_read <= 0)
	// 	return -1;
	// if(byte_stream_recv[3] == 0x00)
	// {
	// 	printf("recv 0x09 cmd success\r\n");
	// 	return 1;

	// }
}

int8_t Send_0x0A(int fd,void* param,int size)
{
	
	uint8_t i=0;
	Frame_0A_byte frame_0A;
	int BYTE0ASIZE=sizeof(Frame_0A_byte);
	memset(&frame_0A,0,BYTE0ASIZE);
	uint8_t send_byte_stream[BYTE0ASIZE] ;

	Node_Xwg_Pairs *param_pairs=(Node_Xwg_Pairs *)malloc(size);
	memcpy(param_pairs,(Node_Xwg_Pairs*)param,size);

	MEM_REPLY_FRAME *member_info=(MEM_REPLY_FRAME*)param;
	

	frame_0A.head=htons(0xD55D);
	frame_0A.dirrection=0x0A;
	frame_0A.message_type=0x0;

	memcpy((void*)&frame_0A.member,(void*)&member_info->member,sizeof(NodeBasicInfo));


	frame_0A.check = htons(CRC_Check(&frame_0A.dirrection, BYTE0ASIZE-6));	
	frame_0A.tail =  htons(0x5DD5);

	memcpy(send_byte_stream,&frame_0A,BYTE0ASIZE);
	printf("send cmd 0x0a,len:%d\r\n",write(fd,(void*)send_byte_stream,BYTE0ASIZE));
	
	
	// write(fd,(void*)send_byte_stream,BYTE0ASIZE);
	sleep(1);

	
	for(i=0;i<sizeof(send_byte_stream);i++)
	{
		printf("%02X",send_byte_stream[i]);
	}
	printf("\r\n");	
	
	return 0;
}
/* 处理参数内容 配置电台参数 */
void process_cmd_info(uint32_t cmd_addr, uint32_t cmd_value)
{
	int ret=0;
	uint32_t t_freq=0;
	// uint8_t uart_bw_table[]={0,3,2,1,0};
	
	static uint8_t uart_mcs_code=0;    //编码效率
//update node_xwg	
	bool isset =FALSE;
	char bcast_buf[1000];
	uint8_t cmd[200];
	INT8 buffer[sizeof(Smgmt_header) + sizeof(Smgmt_set_param)];
	INT32 buflen = sizeof(Smgmt_header) + sizeof(Smgmt_set_param);
	Smgmt_header* mhead = (Smgmt_header*)buffer;
	Smgmt_set_param* mparam = (Smgmt_set_param*)mhead->mgmt_data;
	bzero(buffer, buflen);
	memset(cmd,0,sizeof(cmd));
	mhead->mgmt_head = htons(HEAD);
	mhead->mgmt_len = sizeof(Smgmt_set_param);
	mhead->mgmt_type = 0;

	/*static wg param init*/
	static uint8_t s_sync_mode=0;
	static uint8_t s_workmode=WORK_MODE_TYPE_DP;
	static uint8_t s_kylb=1;   //默认关闭
	static uint8_t s_bw=0;
	static uint8_t s_mcs=0;   //调制方式
	static uint8_t s_transmode=0;
	static uint32_t s_freq,s_select_freq1,s_select_freq2,s_select_freq3,s_select_freq4;
	s_freq=FREQ_INIT;
	// s_select_freq1=s_select_freq2=s_select_freq3=s_select_freq4=0;


//update test.db
	// stInData stsysteminfodata;
	// memset((char*)&stsysteminfodata,0,sizeof(stsysteminfodata));

	switch (cmd_addr)
	{

	case PARAM_0A_REQUEST_ADDR:
		send_member_request((uint8_t)cmd_value);
	break;
	case PARAM_OP_MODE_CURRENT_WORK_MODE:
		printf("[UI DEBUG] transmode :%d",cmd_value);
		isset=TRUE;
		mhead->mgmt_type |= MGMT_SET_TEST_MODE;
		mparam->mgmt_mac_work_mode=htons((uint16_t)cmd_value);
		s_transmode=cmd_value;
	break;
	case PARAM_OP_MODE_SPATIAL_FILTERING:     //空域滤波
		printf("[UI DEBUG] kylb :%d \r\n", (uint8_t)cmd_value);
		s_kylb=cmd_value;
		if(cmd_value==0)
		{
			if(s_workmode==WORK_MODE_TYPE_ZSYXP)
			{
				printf("DT workmode is zsyxp,fail to set kylb\r\n");
				break;
			}
			isset=TRUE;
			mhead->mgmt_type |= MGMT_SET_WORKMODE;
			mparam->mgmt_net_work_mode.NET_work_mode=WORK_MODE_TYPE_KYLB;
			//printf("[UART] set kylb 0--open \r\n ");
			
			// updateData_meshinfo_qk("workmode",WORK_MODE_TYPE_KYLB);
			sprintf(cmd,
				"sed -i \"s/kylb .*/kylb %d/g\" /etc/node_xwg",
				KYLB_MODE_OPEN);		
			system(cmd);
						
		}
		else
		{
			sprintf(cmd,
				"sed -i \"s/kylb .*/kylb %d/g\" /etc/node_xwg",
				KYLB_MODE_CLOSE);		
			system(cmd);

		}
		break;

	// 	break;
	case PARAM_CH1_FREQ_HOPPING_MODE:  //跳频方式
		printf("[UI DEBUG] work mode: %d \r\n",cmd_value);
		if(cmd_value==0)
		{
			printf("[UI DEBUG]set work mode dp \r\n ");
			isset=TRUE;
			mhead->mgmt_type |= MGMT_SET_WORKMODE;
			mparam->mgmt_net_work_mode.NET_work_mode=WORK_MODE_TYPE_DP;
			mhead->mgmt_type |= MGMT_SET_FREQUENCY;
			mparam->mgmt_mac_freq = htonl(s_freq);

			s_workmode=WORK_MODE_TYPE_DP;

			sprintf(cmd,
				"sed -i \"s/workmode .*/workmode %d/g\" /etc/node_xwg",
				WORK_MODE_TYPE_DP);		
			system(cmd);		
			
			// updateData_meshinfo_qk("workmode",WORK_MODE_TYPE_DP);

		}
		else if(cmd_value==1)
		{
			/*自适应选频*/
			if(s_select_freq1==0||s_select_freq2==0||s_select_freq3==0||s_select_freq4==0)
			{
				printf("[UI DEBUG] set zsyxp error , freq info :%d %d %d %d \r\n",s_select_freq1,
				s_select_freq2,s_select_freq3,s_select_freq4);
				break;
			}
			printf("[UI DEBUG]set work mode zsyxp \r\n ");
			isset=TRUE;
			mhead->mgmt_type |= MGMT_SET_WORKMODE;
			mparam->mgmt_net_work_mode.NET_work_mode=WORK_MODE_TYPE_ZSYXP;

			mparam->mgmt_net_work_mode.fh_len=4;
			mparam->mgmt_net_work_mode.hop_freq_tb[0]=s_select_freq1;
			mparam->mgmt_net_work_mode.hop_freq_tb[1]=s_select_freq2;
			mparam->mgmt_net_work_mode.hop_freq_tb[2]=s_select_freq3;
			mparam->mgmt_net_work_mode.hop_freq_tb[3]=s_select_freq4;


			s_workmode=WORK_MODE_TYPE_ZSYXP;
			sprintf(cmd, "sed -i \"s/workmode .*/workmode %d/; \
						s/select_freq1 .*/select_freq1 %d/; \
						s/select_freq2 .*/select_freq2 %d/; \
						s/select_freq3 .*/select_freq3 %d/; \
						s/select_freq4 .*/select_freq4 %d/\" /etc/node_xwg", 
					WORK_MODE_TYPE_ZSYXP, s_select_freq1, s_select_freq2, s_select_freq3, s_select_freq4);
			system(cmd);


		}
		// updateData_meshinfo_qk("workmode",WORK_MODE_TYPE_ZSYXP);
	break;

	case PARAM_CH1_ROUTING_PROTOCOL:    //路由协议
		printf("[UI DEBUG] router :%d \r\n", (uint8_t)cmd_value);	 
		switch(cmd_value)
		{
			case 0:   //olsr
				printf("[UI DEBUG] set route olsr \r\n");
				ret = system("/home/root/cs_olsr.sh");
				if(ret == -1) printf("change olsr failed\r\n");
				sprintf(cmd,
					"sed -i \"s/router .*/router %d/g\" /etc/node_xwg",
				KD_ROUTING_OLSR);		
				system(cmd);
				
				// updateData_meshinfo_qk("m_route",1);
			break;
			case 1:  //aodv
				printf("[UI DEBUG] set route aodv \r\n");
				ret = system("/home/root/cs_aodv.sh");
				if(ret == -1) printf("change batman failed\r\n");

				sprintf(cmd,
					"sed -i \"s/router .*/router %d/g\" /etc/node_xwg",
				KD_ROUTING_AODV);		
				system(cmd);

				// updateData_meshinfo_qk("m_route",2);
			break;
			case 2:  //batman 
				printf("[UI DEBUG] set route batman \r\n");
				ret = system("/home/root/cs_batman.sh");
				if(ret == -1) printf("change batman failed\r\n");
				sprintf(cmd,
					"sed -i \"s/router .*/router %d/g\" /etc/node_xwg",
				KD_ROUTING_CROSS_LAYER);		
				system(cmd);

				// updateData_meshinfo_qk("m_route",3);
			break;
			default:
			break;
		}
		break;
	case PARAM_CH1_FIXED_FREQ_CENTER:    //定频-中心频点
		printf("[UI DEBUG]fix freq :%d workmode :%d \r\n", cmd_value/1000,s_workmode);
		s_freq=t_freq=cmd_value/1000;
		if(t_freq<=225) s_freq=225;
		if(t_freq>=2500) s_freq=2500;
		if(s_workmode==WORK_MODE_TYPE_ZSYXP)
		{
			break;
		}
		isset=TRUE;
		mhead->mgmt_type |= MGMT_SET_WORKMODE;
		mparam->mgmt_net_work_mode.NET_work_mode =WORK_MODE_TYPE_DP;
		mhead->mgmt_type |= MGMT_SET_FREQUENCY;
		mparam->mgmt_mac_freq = htonl(s_freq);

		updateData_systeminfo_qk("rf_freq",s_freq);
		// updateData_meshinfo_qk("rf_freq",s_freq);
		break;

	case PARAM_CH1_SELECTED_FREQ_1:
		printf("[UI DEBUG]select freq-1 :%d \r\n", cmd_value/1000);
		t_freq=cmd_value/1000;
		s_select_freq1=t_freq;
		if(t_freq<=225) s_select_freq1=225;
		if(t_freq>=2500) s_select_freq1=2500;
		// updateData_meshinfo_qk("m_select_freq1",s_select_freq1);

	break;
	case PARAM_CH1_SELECTED_FREQ_2:
		printf("[UI DEBUG]select freq-2 :%d  \r\n", cmd_value/1000);
		t_freq=cmd_value/1000;
		s_select_freq2=t_freq;
		if(t_freq<=225) s_select_freq2=225;
		if(t_freq>=2500) s_select_freq2=2500;
		// updateData_meshinfo_qk("m_select_freq2",s_select_freq2);
	break;
	case PARAM_CH1_SELECTED_FREQ_3:
		printf("[UI DEBUG]select freq-3 :%d \r\n", cmd_value/1000);
		t_freq=cmd_value/1000;
		s_select_freq3=t_freq;
		if(t_freq<=225) s_select_freq3=225;
		if(t_freq>=2500) s_select_freq3=2500;
		// updateData_meshinfo_qk("m_select_freq3",s_select_freq3);

	break;
	case PARAM_CH1_SELECTED_FREQ_4:
		printf("[UI DEBUG]select freq-4 :%d \r\n", cmd_value/1000);
		t_freq=cmd_value/1000;
		s_select_freq4=t_freq;
		if(t_freq<=225) s_select_freq4=225;
		if(t_freq>=2500) s_select_freq4=2500;
		// updateData_meshinfo_qk("m_select_freq4",s_select_freq4);
	break;		

	case PARAM_CH1_SIGNAL_BANDWIDTH:    //带宽
		printf("[UI DEBUG]bw: %d \r\n",cmd_value);
		isset=TRUE;
		mhead->mgmt_type |= MGMT_SET_BANDWIDTH;
		mparam->mgmt_mac_bw=cmd_value;
		
		updateData_systeminfo_qk("m_chanbw",mparam->mgmt_mac_bw);
		// updateData_meshinfo_qk("m_chanbw",mparam->mgmt_mac_bw);
		break;
	case PARAM_CH1_MODULATION_WIDEBAND:    //宽带 调制方式
		printf("[UI DEBUG] mcs: %d \r\n",cmd_value);
		isset=TRUE;
		mhead->mgmt_type |= MGMT_SET_UNICAST_MCS;

		s_mcs=cmd_value;
		mparam->mgmt_virt_unicast_mcs=s_mcs;
		updateData_systeminfo_qk("m_rate",mparam->mgmt_virt_unicast_mcs);
		// updateData_meshinfo_qk("m_rate",mparam->mgmt_virt_unicast_mcs);
		break;
	case PARAM_OP_MODE_SYNC_MODE:
		s_sync_mode=cmd_value;
		sprintf(cmd,
			"sed -i \"s/sync_mode .*/sync_mode %d/g\" /etc/node_xwg",
			cmd_value);		
		system(cmd);
		break;

	case PARAM_CH1_TX_POWER_ATTENUATION:    //发射功率衰减
		printf("[UI DEBUG] power_atten: %d \r\n",cmd_value);

		break;
	case PARAM_TXRX_INFO_OPERATION:     //消息统计操作
		printf("[UI DEBUG] 0x06 cmd info operation: %d\r\n",cmd_value);
		stat_info.stat_flag=cmd_value;

		break;	
	default:
		break;
	}

	if(isset)
	{
		isset=FALSE;
		mhead->mgmt_type = htons(mhead->mgmt_type);
		mhead->mgmt_keep = htons(mhead->mgmt_keep);
		mgmt_netlink_set_param(buffer, buflen,NULL);	
		sleep(1);
		if (!persist_test_db()) {
			printf("[ui_get] persist test.db failed after UI command\n");
		}
	}
}

/* 处理工控屏指令 */
uint16_t  reply_uart_info(int fd,void* ack_info, uint16_t len)
{
	return write(fd,ack_info,len);
}

void process_uart_info(int fd,char* info, int len)
{

	uint8_t uart_buf[MAX_UI_SIZE];
	uint8_t cmd_type = 0;
	uint16_t cmd_len = 0;
	uint8_t  param_len = 0;

	UART_FRAME_1_BYTE recv_frame_1;
	UART_FRAME_2_BYTE recv_frame_2;
	UART_FRAME_4_BYTE recv_frame_4;
	UART_FRAME_16_BYTE recv_frame_16;

	memset(&recv_frame_1,0,sizeof(UART_FRAME_1_BYTE));
	memset(&recv_frame_2,0,sizeof(UART_FRAME_2_BYTE));
	memset(&recv_frame_4,0,sizeof(UART_FRAME_4_BYTE));
	memset(&recv_frame_16,0,sizeof(UART_FRAME_16_BYTE));

	if (info == NULL || len <= 0)
	{
		printf("ERROR:uart info is null\r\n");
		return;
	}

	memset(uart_buf, 0, MAX_UI_SIZE);
	memcpy(uart_buf, info, len);
	
	// printf("[UART] info :");

	// for(int i=0;i<len;i++)
	// {
	// 	printf("%x ",uart_buf[i]);
	// }
	// printf("\r\n");
	
	if (uart_buf[0] != 0xd5 || uart_buf[1] != 0x5d)
	{
		printf("ERROR:uart head error\r\n");
		return;
	}

	if (uart_buf[len - 2] != 0x5d || uart_buf[len - 1] != 0xd5)
	{
		printf("ERROR:uart tail error\r\n");
		return;

	}

	cmd_type = uart_buf[2];   //命令字

	if(cmd_type==0x0a)
	{
		/* 处理0a命令 网内节点详细信息查询 */
		process_cmd_info(PARAM_0A_REQUEST_ADDR,uart_buf[4]);
		return;
	}

	cmd_len = uart_buf[4];   //参数数值长度
	uint32_t addr = (uart_buf[5] << 24) | (uart_buf[6] << 16) | (uart_buf[7] << 8) | (uart_buf[8]);
	addr = htonl(addr);
	param_len = cmd_len - 1 - 4;

	//printf("param len %d \r\n");


	switch (param_len)
	{
		case 1:
			memcpy(&recv_frame_1, info, len);
			process_cmd_info(addr, recv_frame_1.value);
			//ack
			recv_frame_1.cmd_no = 0x02;
			recv_frame_1.ack_flag = MESSAGE_TYPE_REPLY;
			recv_frame_1.crc = htons(CRC_Check(&recv_frame_1.cmd_no, sizeof(recv_frame_1)-6));					//校验
			reply_uart_info(fd,(void*)&recv_frame_1,sizeof(recv_frame_1));

			break;
		case 2:
			memcpy(&recv_frame_2, info, len);
			process_cmd_info(addr, recv_frame_2.value);

			recv_frame_2.cmd_no = 0x02;
			recv_frame_2.ack_flag = MESSAGE_TYPE_REPLY;
			recv_frame_2.crc = htons(CRC_Check(&recv_frame_2.cmd_no, sizeof(recv_frame_2)-6));					//校验
			reply_uart_info(fd,(void*)&recv_frame_2,sizeof(recv_frame_2));

			break;
		case 4:
			memcpy(&recv_frame_4, info, len);
			//printf("htons %d",recv_frame_4.value);
			process_cmd_info(addr, recv_frame_4.value);

			//ack
			recv_frame_4.cmd_no = 0x02;
			recv_frame_4.ack_flag = MESSAGE_TYPE_REPLY;
			recv_frame_4.crc = htons(CRC_Check(&recv_frame_4.cmd_no, sizeof(recv_frame_4)-6));					//校验

			reply_uart_info(fd,(void*)&recv_frame_4,sizeof(recv_frame_4));
			
		// case 16:
		// 	memcpy(&recv_frame_16, info, len);
		// 	//printf("htons %d",recv_frame_16.value);
		// 	process_cmd_info(addr, recv_frame_16.value);

		// 	//ack
		// 	recv_frame_16.cmd_no = 0x02;
		// 	recv_frame_16.ack_flag = MESSAGE_TYPE_REPLY;
		// 	recv_frame_16.crc = htons(CRC_Check(&recv_frame_16.cmd_no, sizeof(recv_frame_16)-6));					//校验

		// 	reply_uart_info(fd,(void*)&recv_frame_16,sizeof(recv_frame_16));
			
		// 	break;
		default:
			break;

	}


}


static void read_xwg_info(char *info,int size)
{
/* temp param */
	uint8_t  t_workmode=0;
	uint32_t t_freq;
	uint8_t  t_bw;
	uint8_t  t_mcs;
	uint8_t  t_routing;
	uint8_t  t_power_level;
	uint8_t  t_power_atten;
	uint32_t t_select_freq1,t_select_freq2,t_select_freq3,t_select_freq4;
	uint8_t  t_kylb;
	uint8_t  t_sync_mode;
// read /etc/node_xwg
	Node_Xwg_Pairs param_pairs[] = {
        {"channel", 0, 0},{"power", 0, 0},{"bw", 0, 0},{"mcs", 0, 0},
        {"macmode", 0, 0},{"slotlen", 0, 0},{"router", 0, 0},{"workmode", 0, 0},
        {"select_freq1", 0, 0},{"select_freq2", 0, 0},{"select_freq3", 0, 0},{"select_freq4", 0, 0},
		{"sync_mode", 0, 0},{"kylb", 0, 0}
		// {"longitude ",0,0},{"latitude  ",0,0},{"altitude ",0,0}
    };

	//sleep(30);
	read_node_xwg_file("/etc/node_xwg",param_pairs,MAX_XWG_PAIRS);

	NodeBasicInfo node_info;
	memset(&node_info,0,sizeof(NodeBasicInfo));

	node_info.member_id=SELFID;
	// sprintf(node_info.ip_address,"192.168.2.%d",SELFID);
	node_info.ip_address[0]=192;
	node_info.ip_address[1]=168;
	node_info.ip_address[2]=2;
	node_info.ip_address[3]=SELFID;



	t_workmode=(uint8_t)get_int_value((void*)param_pairs,"workmode");

	t_kylb=(uint8_t )get_int_value((void*)param_pairs,"kylb");
	t_freq=get_int_value((void*)param_pairs,"channel")*1000;
	t_select_freq1=get_int_value((void*)param_pairs,"select_freq1");
	t_select_freq2=get_int_value((void*)param_pairs,"select_freq2");
	t_select_freq3=get_int_value((void*)param_pairs,"select_freq3");
	t_select_freq4=get_int_value((void*)param_pairs,"select_freq4");
	t_sync_mode=(uint8_t)get_int_value((void*)param_pairs,"sync_mode");
	t_bw=(uint8_t)get_int_value((void*)param_pairs,"bw");
	t_mcs=(uint8_t)get_int_value((void*)param_pairs,"mcs");
	t_routing=(uint8_t)get_int_value((void*)param_pairs,"router");
	// t_power_level
	// t_power_atten

	node_info.spatial_filter_status=1;  //默认空余滤波关
	node_info.channel1.ch_frequency_hopping=0; //默认定频

	if(t_kylb==KYLB_MODE_OPEN)
	{
		node_info.spatial_filter_status=0;
	}
	if(t_workmode==5)
	{
		node_info.channel1.ch_frequency_hopping=1;
	}


	node_info.channel1.ch_working_freq=t_freq*1000;
	node_info.channel1.ch_waveform=t_mcs;
	node_info.channel1.ch_signal_bandwidth=t_bw;
	node_info.channel1.ch_routing_protocol=t_routing-1;
	

	memcpy(info,(void*)&node_info,size);
    
}

void send_member_request(uint8_t id)
{
	int s_request;
	int ret=0;
	char dest_ip[20];

	memset(dest_ip,0,sizeof(dest_ip));
	snprintf(dest_ip, sizeof(dest_ip), "192.168.2.%d", id);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	
	s_request=createUdpClient(&addr,dest_ip,MEM_REQUEST_PORT);

	MEM_REQUEST_FRAME info;
	memset(&info,0,sizeof(MEM_REQUEST_FRAME));
	info.head=htons(0x4c4a);
	info.src_id=SELFID;
	info.dst_id=id;
	info.type=0;
	info.tail=htons(0x6467);

	ret=SendUDPClient(s_request,(char*)&info,sizeof(MEM_REQUEST_FRAME),&addr);
	if(ret<=0)
	{
		printf("[UI DEBUG]send member request fail\r\n");
	}
	printf("send member request \r\n");
	sleep(1);
	close(s_request);
}


/* 处理网内成员信息单播包 */
void process_member_info(char* info,int size)
{
	if(info==NULL||size<=0)
	{
		return;
	}

	int reply_s;
	int ret=0;
	char dest_ip[20];
	memset(dest_ip,0,sizeof(dest_ip));

	
	MEM_REQUEST_FRAME *request_info=(MEM_REQUEST_FRAME*)info;

	if(ntohs(request_info->head)!=0x4c4a)
	{
		printf("[ERROR] member request head error\r\n");
		return;
	}

	MEM_REPLY_FRAME reply_info;
	memset(&reply_info,0,sizeof(MEM_REPLY_FRAME));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));	


	switch(request_info->type)
	{
		case 0:
		/* 回复成员信息单播请求 */

			printf("recv member request info\r\n");
			reply_info.head=htons(0x4c4a);
			reply_info.type=1;
			reply_info.src_id=SELFID;
			reply_info.dst_id=request_info->src_id;
			reply_info.tail=htons(0x6467);

			read_xwg_info((void*)&reply_info.member,sizeof(NodeBasicInfo));

			snprintf(dest_ip, sizeof(dest_ip), "192.168.2.%d", request_info->src_id);

			reply_s=createUdpClient(&addr,dest_ip,MEM_REQUEST_PORT);
			ret=SendUDPClient(reply_s,(char*)&reply_info,sizeof(MEM_REPLY_FRAME),&addr);
			if(ret<=0)
			{
				printf("[ERROR]send member reply info fail\r\n");
			}
			sleep(1);
			close(reply_s);
		break;
		case 1:
		/* 收到回复，直接上报0x0A */
			printf("recv member reply info\r\n");
			memcpy(&reply_info,info,size);
			//增加帧头帧尾判断
			Send_0x0A(ui_fd,(void*)info,size);

		break;
		
		default:
		break;
	}

}




/* 初始化工控屏 */
int uart_init(void)
{
	int ui_Fd;
	uint8_t cnt=0;

	while(cnt<MAX_RETRY_COUNT)
	{
		ui_Fd = open(FD_UI_UART, O_RDWR);  //打开串口屏串口   |O_NOCTTY
		if(ui_Fd!=-1)
			break;
		cnt++;
		if(cnt<MAX_RETRY_COUNT)
			sleep(1);		
	}
	if(ui_Fd==-1)
	{
		printf("ERROR:open ui uart fail\r\n");
		return -1;
	}
	printf("[UI DEBUG]uart fd : %d \r\n",ui_Fd);
     set_opt(ui_Fd, UI_UART_BAUD, 8, 'N', 1);	//设置nmea串口属性
//     struct termios tty;
//     memset(&tty, 0, sizeof(tty));
// // 设置波特率
//     cfsetospeed(&tty, B115200);
//     cfsetispeed(&tty, B115200);
    
//     // 8N1
//     tty.c_cflag &= ~PARENB;
//     tty.c_cflag &= ~CSTOPB;
//     tty.c_cflag &= ~CSIZE;
//     tty.c_cflag |= CS8;
    
//     // 禁用流控制
//     tty.c_cflag &= ~CRTSCTS;
    
//     // 启用接收
//     tty.c_cflag |= CREAD | CLOCAL;
    
//     // 禁用回显等功能
//     tty.c_lflag &= ~ICANON;
//     tty.c_lflag &= ~ECHO;
//     tty.c_lflag &= ~ECHOE;
//     tty.c_lflag &= ~ECHONL;
//     tty.c_lflag &= ~ISIG;
    
//     // 禁用输入处理
//     tty.c_iflag &= ~(IXON | IXOFF | IXANY);
//     tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    
//     // 原始输出
//     tty.c_oflag &= ~OPOST;
//     tty.c_oflag &= ~ONLCR;
    
//     // 超时设置
//     tty.c_cc[VTIME] = 10;    // 1秒超时
//     tty.c_cc[VMIN] = 100;

//     if (tcsetattr(ui_Fd, TCSANOW, &tty) != 0) {
//         perror("设置串口属性失败");
//         close(ui_Fd);
//         return -1;
//     }
    
//     tcflush(ui_Fd, TCIOFLUSH);
	return ui_Fd;
}

void get_ui_info(int fd)
 {
	int i = 0, j = 0, k = 0;
	int ui_Fd;
	char  ui_info[1024];
	int len;
	uint8_t cnt=0;
	char fd_uart[100];
	memset(fd_uart,0,sizeof(fd_uart));

	ui_Fd=fd;


	// // FILE* file_uart;
	// // if ((file_uart = fopen(UI_UART_PATH, "r")) != NULL) 
	// // {
	// // 	/* read uart path from file */
	// // 	while (fgets(fd_uart, sizeof(fd_uart), file_uart) != NULL)
	// // 	{
			
	// // 	}
	// // 	fclose(file_uart);
	// // }

	// while(cnt<MAX_RETRY_COUNT)
	// {
	// 	ui_Fd = open(FD_UI_UART, O_RDWR);  //打开串口屏串口   |O_NOCTTY
	// 	if(ui_Fd!=-1)
	// 		break;
	// 	cnt++;
	// 	if(cnt<MAX_RETRY_COUNT)
	// 		sleep(1);		
	// }
	// if(ui_Fd==-1)
	// {
	// 	printf("ERROR:open ui uart fail\r\n");
	// 	//return;
	// }
	// printf("uart fd : %d \r\n",ui_Fd);
    // set_opt(ui_Fd, UI_UART_BAUD, 8, 'N', 1);	//设置nmea串口属性

	while(1)
	{
		len=read(ui_Fd,ui_info,MAX_UI_SIZE-1); //从串口获取gps信息
		if(len>0)
		{
			/* 处理串口数据 */
			//printf("rx uart info %d \r\n",len);
			// for(int i=0;i<len;i++)
			// {
			// 	printf("-%x",ui_info[i]);
			// }
			// printf("\r\n");
			process_uart_info(ui_Fd,ui_info,len);
		}


	}

	
	//gps_getfrom_uart(nmeaFd);

	close(ui_Fd);
	return;

}


//统计eth0统计信息
int get_network_stats(const char *interface_name,Info_0x06_Statistics* info_stat) {
    FILE *fp;
    char buffer[1024];
    unsigned long rx_bytes, rx_packets, tx_bytes, tx_packets;
    // 初始化返回值
    //unsigned long rx_bytes, tx_bytes;
    // 打开/proc/net/dev文件
    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL) {
        perror("fopen /proc/net/dev failed");
        return -1;
    }
    
    // 跳过前两行标题
    fgets(buffer, sizeof(buffer), fp); // 第一行标题
    fgets(buffer, sizeof(buffer), fp); // 第二行标题
    
    // 遍历每一行查找目标接口
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char ifname[32];
        
        // 解析接口名（去掉冒号）
        if (sscanf(buffer, " %[^:]:", ifname) == 1) {
            // 去掉接口名末尾的空格
            size_t len = strlen(ifname);
            while (len > 0 && ifname[len-1] == ' ') {
                ifname[--len] = '\0';
            }
            
            // 检查是否为目标接口
            if (strcmp(ifname, interface_name) == 0) {
                // 解析统计信息：只需要前两个字段(rx_bytes, rx_packets)和第九个字段(tx_bytes, tx_packets)
                
                int fields=sscanf(buffer, " %*[^:]: %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu",
					   &rx_bytes,&rx_packets,&tx_bytes,&tx_packets);
				if(fields==4)
				{
					info_stat->eth_rx_bytes=rx_bytes;
					info_stat->eth_tx_bytes=tx_bytes;
					info_stat->eth_rx_packets=rx_packets;
					info_stat->eth_tx_packets=tx_packets;
				}
                break;
            }
        }
    }
    
    fclose(fp);
    return 0;
}


int get_interface_stats(Info_0x06_Statistics* info_stat) {
	FILE *fp;
    char buffer[1024];
    char command[128];
    unsigned long rx_packets = 0;
    unsigned long tx_packets = 0;
	uint8_t find_tx,find_rx;

	find_tx=find_rx=0;
    // 这里修改想要读取的网络配置
    snprintf(command, sizeof(command), "ifconfig eth0");
    
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 修改想要读取的参数
        if (strstr(buffer, "RX packets") != NULL) {
            // 使用 sscanf 从行中提取 RX packets 的数值
            // ifconfig 输出中 "RX packets" 后的数字就是包数量
            if (sscanf(buffer, " RX packets : %lu", &rx_packets) == 1) {
				find_rx=1;
                //printf("RX packets: %lu\n", rx_packets);
            } 
			// else {
            //     char *packet_str = strstr(buffer, "packets");
            //     if (packet_str) {
            //         if (sscanf(packet_str, "packets %lu", &rx_packets) == 1) {
            //             printf("RX packets: %lu\n", rx_packets);
            //         } else {
            //             printf("Failed to parse RX packets from: %s", buffer);
            //         }
            //     }
            // }
			
            //break; // 找到目标行后可以跳出循环
        }
        if (strstr(buffer, "TX packets") != NULL) {
            // 使用 sscanf 从行中提取 RX packets 的数值
            // ifconfig 输出中 "RX packets" 后的数字就是包数量
            if (sscanf(buffer, " TX packets : %lu", &tx_packets) == 1) {
				find_tx=1;
                //printf("TX packets: %lu\n", tx_packets);
            } 
			// else {
            //     char *packet_str = strstr(buffer, "packets");
            //     if (packet_str) {
            //         if (sscanf(packet_str, "packets %lu", &rx_packets) == 1) {
            //             printf("RX packets: %lu\n", rx_packets);
            //         } else {
            //             printf("Failed to parse RX packets from: %s", buffer);
            //         }
            //     }
            // }
			
            //break; // 找到目标行后可以跳出循环
        }		

		if(find_tx==1&&find_rx==1)
		{
			info_stat->eth_rx_packets=(uint16_t)rx_packets;
			info_stat->eth_tx_packets=(uint16_t)tx_packets;
			break;

		}
    }
    
    // 关闭管道
    pclose(fp);
    return 0;
}

void write_ui_Thread(void* arg)
{
	int ui_Fd=(int )arg;
	printf("create thread: report uart  info uart fd :%d \r\n",ui_Fd);

	struct mgmt_send self_msg;
	memset(&self_msg,0,sizeof(self_msg));

	Node_Xwg_Pairs param_pairs[] = {
        {"channel", 0, 0},{"power", 0, 0},{"bw", 0, 0},{"mcs", 0, 0},
        {"macmode", 0, 0},{"slotlen", 0, 0},{"router", 0, 0},{"workmode", 0, 0},
        {"select_freq1", 0, 0},{"select_freq2", 0, 0},{"select_freq3", 0, 0},{"select_freq4", 0, 0},
		{"sync_mode",0,0},{"kylb",0,0}
		// {"longitude ",0,0},{"latitude  ",0,0},{"altitude ",0,0},
    };

	//sleep(30);
	 read_node_xwg_file("/etc/node_xwg",param_pairs,MAX_XWG_PAIRS);
	// sleep(1);
	Send_0x04(ui_Fd,(void*)&param_pairs,sizeof(Node_Xwg_Pairs)*MAX_XWG_PAIRS);
	memset(&stat_info,0,sizeof(Info_0x06_Statistics));


	while(1)
	{

		read_node_xwg_file("/etc/node_xwg",param_pairs,MAX_XWG_PAIRS);
	 	Send_0x08(ui_Fd,(void*)&param_pairs,sizeof(Node_Xwg_Pairs)*MAX_XWG_PAIRS);


		mgmt_netlink_get_info(0, MGMT_CMD_GET_VETH_INFO, NULL, (char*)&self_msg);
		//printf("send cmd 05 07 09 \r\n");
		Send_0x05(ui_Fd,&g_radio_param);
		//sleep(1);

		  Send_0x06(ui_Fd,(void*)&stat_info);
		  Send_0x07(ui_Fd,&self_msg.amp_infomation);    //0x07 自检
		// //sleep(1);

		 Send_0x09(ui_Fd,&self_msg);
		// //sleep(1);

		sleep(1);
	}

}

void get_ui_Thread(void* arg) 
{
	

	int fd=(int)arg;
	printf("create thread: read uart info uart fd %d \r\n",fd);
	while (1) 
	{
		get_ui_info(fd);
//		printf("gps recyle\n");
		usleep(50000);
	}
}
