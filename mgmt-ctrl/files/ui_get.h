#ifndef _UIGET_H_
#define _UIGET_H_


//#include"gpsget.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<stdint.h>

#define MEM_REQUEST_PORT     7903

#define  FD_UI_UART     "/dev/ttyUL2" 
#define  MAX_UI_SIZE 1024
#define  UI_UART_BAUD   115200    
#define  MAX_RETRY_COUNT  5


#define FRAME_HEAD  0xd55d
#define FRAME_TAIL  0x5dd5
#define MESSAGE_TYPE_ACTIVE 0xFF        //主动发出
#define MESSAGE_TYPE_REPLY  0x00        //应答成功
#define MESSAGE_TYPE_ERROR  0x01        //应答错误


#pragma pack(push, 1)
 
typedef enum
{
    WORK_MODE_TYPE_DP=1,                //定频模式
    WORK_MODE_TYPE_ZSYXP=4,              //自适应选频
    WORK_MODE_TYPE_KYLB=5,              //空域滤波
}WORK_MODE_TYPE;

//命令字
typedef enum
{

    CMD_TYPE_MODE_SET=0x04,             //初始化：模式设置/参数设置
    CMD_TYPE_PARAM_SET=0x05,            //初始化：设置参数
    CMD_TYPE_MSG_STATS=0x06,            //收发消息统计
    CMD_TYP_SELFTEST=0x07,              //自检状态
    CMD_TYPE_ELEC_ENV_STATUS=0x08,      //电磁环境状态
    CMD_TYPE_NODE_LIST=0x09,            //在网节点列表
    CMD_TYPE_NODE_DETAIL=0x0a           //节点详细信息
}CMD_TYPE;

typedef struct {
    uint16_t head;//帧头
    uint8_t  cmd_no;//1：工控屏收  2：工控屏发
    uint8_t  ack_flag;//消息类型(主动发的消息ff,回复00,错误01)
    uint8_t  length;//参数数值长度
    uint32_t value_addr;// 参数地址
    uint8_t  value; //参数值
    uint16_t crc; //校验
    uint16_t tail;//帧尾



}UART_FRAME_1_BYTE;

typedef struct {
    uint16_t head;
    uint8_t  cmd_no;
    uint8_t  ack_flag;
    uint8_t  length;
    uint32_t value_addr;
    uint16_t  value;
    uint16_t crc;
    uint16_t tail;


}UART_FRAME_2_BYTE;

typedef struct {
    uint16_t  head;
    uint8_t   cmd_no;
    uint8_t   ack_flag;
    uint8_t   length;
    uint32_t  value_addr;
    uint32_t  value;
    uint16_t  crc;
    uint16_t  tail;


}UART_FRAME_4_BYTE;

typedef struct {
    uint16_t  head;
    uint8_t   cmd_no;
    uint8_t   ack_flag;
    uint8_t   length;
    uint32_t  value_addr;
    uint32_t  value[4];
    uint16_t  crc;
    uint16_t  tail;


}UART_FRAME_16_BYTE;
//  0x04 命令字
// typedef struct {
//     uint8_t  current_work_mode;      // 当前工作模式 (字节1)
//     uint8_t  cancellation_status;    // 对消状态 (字节2)
//     uint8_t  spatial_filtering;      // 空域滤波 (字节3)
//     uint8_t  co_location_mode;       // 共址模式 (字节4)
//     uint8_t  adaptive_freq_select;   // 自适应选频 (字节5)
//     uint8_t  sync_mode;              // 内外同步模式 (字节6)
//     uint8_t  service_mode;           // 业务模式 (字节7)
// } ModeConfig;   //操作-模式设置

// typedef struct {
//     uint8_t  remote_param_modify;    // 远程参数修改 (字节8)
//     uint8_t  update_param_and_join;  // 更新参数并启动入网 (字节9)
// } GeneralParamConfig;   //操作-参数设置

// typedef struct {
//     uint8_t  routing_protocol;       // 路由协议 (字节10/26/42/58)
//     uint8_t  access_protocol;        // 接入协议 (字节11/27/43/59)
//     uint8_t  time_base;              // 时间基准 (字节12/28/44/60)
//     uint8_t  hopping_mode;           // 跳频方式 (字节13/29/45/61)
    
//     uint16_t center_freq;            // 中心频率
//     uint8_t  hopping_pattern;        // 跳频图样号
//     uint8_t  hopping_points;        // 跳频频点个数
//     uint8_t  signal_bandwidth;      //信号带宽
//     uint8_t  narrowband_mode;     // 窄带调制方式
//     uint8_t wideband_mode;       // 宽带调制方式
//     uint8_t  coding_efficiency;      // 编码效率
//     uint8_t narrowband_power;    // 窄带波形功率
//     uint8_t mimo_power;          // MIMO功率 (字节23/39/55/71)
//     uint8_t spread_hopping_power;// 扩跳功率 (字节24/40/56/72)
//     uint8_t  tx_power_attenuation;   // 发射功率衰减

// } ChannelConfig;

// // 0x04  初始化：模式设置/参数设置
// typedef struct {
//     ModeConfig        mode_config;           // 模式设置
//     GeneralParamConfig general_config;       // 通用参数设置
//     ChannelConfig     channel_config[4];     // 4个通道的参数配置
// } Param_0x04_Config;

// // 在网节点信息
// typedef struct{
//     uint8_t         id;                     //成员ID
//     uint32_t        ip;                     //IP地址
//     uint8_t         hop_count;              //与本节点跳数
//     uint8_t         rssi;                   //一跳内节点信号强度
//     uint16_t         trans_delay;           //传输延时
//     char            lon[20];                //经度
//     char            lat[20] ;               //纬度
//     uint16_t        height;                 //高度


// }Online_Node_Info;

// // 网内成员状态-在网节点
// typedef struct{
//     uint8_t                cnt;             //成员个数
//     Online_Node_Info       node_info[12];   //在网节点信息

// }Network_Member_Status_1;


// //网内成员状态-节点详细信息
// typedef struct{
//     uint16_t                id;
//     uint8_t                 workmode;               //工作模式
//     uint8_t                 cancellation_status;    // 对消状态 (字节2)
//     uint8_t                 spatial_filtering;      // 空域滤波 (字节3)
//     uint8_t                 co_location_mode;       // 共址模式 (字节4)
//     uint8_t                 adaptive_freq_select;   // 自适应选频 (字节5)
//     uint32_t                ip;

//     uint8_t                hop_mode;               //跳频方式
//     uint16_t               freq;                   //工作频率
//     uint8_t                bw;                     //带宽
//     uint8_t                work_waveform;          //工作波形
//     uint8_t                tx_power_level;         //发射功率档位
//     uint8_t                 tx_power_attenuation;   // 发射功率衰减
//     uint8_t                 routing_protocol;       //路由协议
//     uint8_t                 access_protocol;        //接入协议


// }Network_Member_Channel_Status;


/*0x04*/
typedef struct {
	uint8_t route_protocol;  // 路由协议
	uint8_t access_protocol ;// 接入协议
	//uint8_t time_reference ;// 时间基准
	uint8_t hopping_mode ;// 跳频方式 0：定频 1：自适应选频
	uint32_t center_freq ;// 中心频率（点频）
	//uint8_t hopping_pattern ;// 跳频图样号
	//uint8_t hopping_points ;// 跳频频点个数
    uint32_t select_freq_1;  //自适应-中心频率1
    uint32_t select_freq_2;  //自适应-中心频率2
    uint32_t select_freq_3;  //自适应-中心频率3
    uint32_t select_freq_4;  //自适应-中心频率4
    
	uint8_t signal_bw ;		// 信号带宽
	// uint8_t mod_narrow ;// 窄带调制方式
	uint8_t mod_wide ;// 宽带调制方式
	// uint8_t coding_rate ;// 编码效率（0.0~1.0）
	// uint8_t tx_power_narrow ;// 窄带发射功率
	// uint8_t tx_power_mimo ;// MIMO发射功率
	uint8_t tx_power_spread ;// 发射功率
	uint8_t tx_attenuation ;// 发射功率衰减
}Channel_Config;

typedef struct {
    uint16_t head;
    uint8_t dirrection;				//命令字这里为0x04
    uint8_t message_type;			//表示消息类型(主动发的消息ff,回复00,错误01)
    uint8_t current_work_mode;      // 当前工作模式
    //uint8_t cancellation_state;     // 对消状态
    uint8_t spatial_filter;        // 空域滤波
    //uint8_t co_location_mode;       // 共址模式
    //uint8_t adaptive_freq_select;   // 自适应选频
    uint8_t sync_mode;              // 内外同步模式
    //uint8_t service_mode;           // 业务模式
    //uint8_t remote_param_modify;    // 远程参数修改
    Channel_Config Channel1;		//通道1
    // Channel_Config Channel2;		//通道2
    // Channel_Config Channel3;		//通道3
    // Channel_Config Channel4;		//通道4
    // uint16_t start_freq;            //功能-电磁环境状态-干扰监测频段-起始频率
    // uint16_t end_freq;              //功能-电磁环境状态-干扰监测频段-终止频率
    uint16_t check;					//校验
    uint16_t tail;
}Frame_04_byte;

/*0x05*/
typedef struct {
	uint8_t device_id;              // 设备ID
	uint8_t device_name[10];            // 设备名称
	uint8_t service_ip[4];             // 业务网口IP地址
	uint16_t service_port;           // 业务网口端口号
	uint8_t management_ip[4];          // 管理网口IP地址
	uint16_t management_port;        // 管理网口端口号
	uint8_t sensing_ip[4];             // 感知网口IP地址
	uint16_t sensing_port;           // 感知网口端口号
	uint8_t serial_baudrate;        // 串口波特率   0-9600/1-19200/2-38400/3-57600/4-115200/5-256000/6-460800/7-921600
	uint8_t serial_databits;        // 串口数据位   0-5位，1-6位，2-7位，3-8位
	uint8_t serial_stopbits;        // 串口停止位   0-1位，1-1.5位，2-2位
	uint8_t serial_parity;          // 串口校验位   0-None 1-Odd 2-Even 3-Mark 4-Space
	uint8_t serial_flowctrl;        // 串口流控     0-NONE 1-XON/XOFF 2-RTS/CTS 3-DTR/DSR 4-RTS/CTS/XON/XOFF 5-DTR/DSR/XON/XOFF
    uint8_t gps_auto_sync;          // 位置设置-自动获取  0:打开 1：关闭
	uint8_t longitude[10];              // 经度
	uint8_t latitude[10];               // 纬度
	uint16_t altitude;               // 高度 10uint2
	uint8_t time_auto_sync;         // 时间自动获取  0:打开 1：关闭
	uint8_t manual_hour;            // 手动设置时
	uint8_t manual_minute;          // 手动设置分
	uint8_t manual_second;          // 手动设置秒
} DeviceConfig;

typedef struct {
    uint16_t head;
    uint8_t dirrection;				//命令字这里为0x05
    uint8_t message_type;			//表示消息类型(主动发的消息ff,回复00,错误01)
    DeviceConfig value;				//携带数值
    uint16_t check;					//校验
    uint16_t tail;
}Frame_05_byte;

/*0x06*/
typedef struct {
    // 收发消息统计
    uint16_t total_tx_cnt;      // 总发送消息个数
    uint16_t total_rx_cnt;      // 总接收消息个数
    // 模拟话音统计
    uint16_t voice_tx_cnt;      // 模拟话音发送个数
    uint16_t voice_rx_cnt;      // 模拟话音接收个数
    // 以太网信息统计
    uint16_t eth_tx_cnt;      // 以太网消息发送个数
    uint16_t eth_rx_cnt;      // 以太网消息接收个数
   
    // // 各消息类型统计
    // uint16_t msg1_tx_cnt;       // 消息类型1发送个数
    // uint16_t msg1_rx_cnt;       // 消息类型1接收个数
    // uint16_t msg2_tx_cnt;       // 消息类型2发送个数
    // uint16_t msg2_rx_cnt;       // 消息类型2接收个数
    // uint16_t msg3_tx_cnt;       // 消息类型3发送个数
    // uint16_t msg3_rx_cnt;       // 消息类型3接收个数
    // uint16_t msg4_tx_cnt;       // 消息类型4发送个数
    // uint16_t msg4_rx_cnt;       // 消息类型4接收个数
    // uint16_t msg5_tx_cnt;       // 消息类型5发送个数
    // uint16_t msg5_rx_cnt;       // 消息类型5接收个数
} MsgStats;

typedef struct {
    uint16_t head;
    uint8_t dirrection;				//命令字这里为0x06
    uint8_t message_type;			//表示消息类型(主动发的消息ff,回复00,错误01)
    MsgStats value;					//携带数值
    uint16_t check;					//校验
    uint16_t tail;
}Frame_06_byte;

/*0x07*/
typedef struct {
    // 自检状态模块
    uint8_t self_test_status;                     // 自检状态
    uint8_t battery_remaining_capacity;           // 电池剩余电量
    uint16_t battery_cycle_count;                  // 电池剩余循环次数
    uint8_t battery_self_test_status;             // 电池自检状态

    // 综合信息处理模块
    uint8_t info_processor_temp;                  // 综合信息处理温度
    uint8_t fan_speed_status;                     // 风机转速状态
    uint8_t nav_lock_status;                      // 卫导锁定状态
    uint8_t clock_selection;                      // 时钟选择状态
    uint8_t adc_status;                           // ADC状态

    // 时钟频率源模块
    uint8_t clock_source_temp;                    // 时钟频率源温度
    uint16_t freq_word_send_count;                 // 频率字下发次数计数
    uint8_t comm_sensing_status;                  // 通信感知状态
    uint8_t ref_clock1_status;                    // 输出参考时钟1状态
    uint8_t ref_clock2_status;                    // 输出参考时钟2状态
    uint8_t lo1_output_status;                    // 本振1输出状态
    uint8_t lo2_output_status;                    // 本振2输出状态
    uint8_t lo3_output_status;                    // 本振3输出状态
    uint8_t lo4_output_status;                    // 本振4输出状态

    // 电源变换模块
    int8_t power_conversion_temp;             // 电源变换温度(取值范围-127~127)
    uint8_t power_on_fault_indicator;             // 上电/故障指示
    uint8_t power_supply_control;                 // 各路加电控制
    uint16_t ac220_power_consumption;              // AC220功耗
    uint16_t dc24v_power_consumption;              // DC24V功耗

	int8_t rf_channel_temp;                      // 通道1温度(取值范围-127~127)
    uint8_t rf_channel_tx_power_status;               // 发射功率检波状态
    uint8_t rf_channel_antenna_l_vswr;                // 天线L口驻波状态
    uint8_t rf_channel_antenna_h1_vswr;               // 天线H-1口驻波状态
    uint8_t rf_channel_antenna_h2_vswr;               // 天线H-2口驻波状态
    uint8_t rf_channel_tx_rx_status;                  // 收发状态指示
    uint16_t rf_Channel_antenna_select;                // 天线选择控制状态
    uint8_t rf_channel_comm_sensing;                  // 通信感知状态
    uint8_t rf_channel_power_control;                 // 加电控制状态
    uint8_t rf_channel_power_level;                   // 通道功率等级
} SystemSelfTestStatus;

typedef struct {
    uint16_t rf_channel_rf_power_detect;               // 射频功率检波(单位0.1V，无符号数，范围0-50V)
    uint16_t rf_channel_if_power_detect;               // 中频功率检波(单位0.1V，无符号数，范围0-50V)
    uint16_t rf_channel_current_freq;                  // 当前频点
    uint8_t rf_channel_agc_attenuation;               // AGc衰减值
    uint8_t rf_channel_signal_bandwidth;              // 通道1信号带宽
    uint8_t rf_channel_attenuation;                   // 通道1衰减量
} RFChannelStatus;

typedef struct {
    uint16_t head;
    uint8_t dirrection;				//命令字这里为0x07
    uint8_t message_type;			//表示消息类型(主动发的消息ff,回复00,错误01)
    SystemSelfTestStatus value;					//携带数值
    RFChannelStatus Channel1;
    RFChannelStatus Channel2;
    RFChannelStatus Channel3;
    RFChannelStatus Channel4;
    uint16_t check;					//校验
    uint16_t tail;
}Frame_07_byte;

/*0x08  开机显示参数*/
typedef struct { 
    uint16_t head;
    uint8_t dirrection;				//命令字这里为0x06
    uint8_t message_type;			//表示消息类型(主动发的消息ff,回复00,错误01)

    uint8_t  current_time[3];       // 当前时间
    uint8_t  net_join_state;        // 入网/未入网  
    uint8_t  mcs;                   // 波形档位
    uint8_t  bw;                    
    uint8_t  workmode;
    uint32_t center_freq;
    uint32_t select_freq1;
    uint32_t select_freq2;
    uint32_t select_freq3;
    uint32_t select_freq4;
    uint8_t  kylb;                  //空余滤波 0：开启，1：关闭
    uint8_t  sync_mode;             //内外同步模式 0：内同步 1：外同步
	uint8_t  tx_power_spread ;      // 发射功率
	uint8_t  tx_attenuation ;       // 发射功率衰减
    uint16_t check;					//校验
    uint16_t tail;

} Frame_08_byte;

/*0x09*/
typedef struct {
    uint8_t member_id;                      // 成员ID
    uint8_t ip_address[4];                     // IP地址
    uint8_t hop_count;                      // 与本节点跳数
    int8_t signal_strength;                // 一跳内节点信号强度
    uint16_t transmission_delay;             // 传输延时
    uint8_t longitude[10];                      // 位置-经度
    uint8_t latitude[10];                       // 位置-纬度
    uint16_t altitude;                       // 位置-高度
} NetworkNodeStatus;

/*0x0A*/

typedef struct {
    uint8_t ch_frequency_hopping;          // 跳频方式
    uint32_t ch_working_freq;               // 工作频率
    uint8_t ch_signal_bandwidth;           // 信号带宽
    uint8_t ch_waveform;                   // 工作波形(调制方式)
    uint8_t ch_power_level;                // 发射功率挡位
    uint8_t ch_power_attenuation;          // 发射功率衰减
    uint8_t ch_routing_protocol;           // 路由协议
    uint8_t ch_access_protocol;            // 接入协议
} ChannelStatus;

typedef struct {
    uint8_t member_id;                      // 成员ID
    // uint8_t working_mode;                   // 工作模式
    // uint8_t cancellation_status;            // 对消状态
    uint8_t spatial_filter_status;          // 空域滤波状态
    // uint8_t colocation_mode;                // 共址模式
    // uint8_t adaptive_frequency;             // 自适应选频
    uint8_t ip_address[4];                     // IP地址
    ChannelStatus channel1;
	// ChannelStatus channel2;
	// ChannelStatus channel3;
	// ChannelStatus channel4;
} NodeBasicInfo;

typedef struct {
    uint16_t head;
    uint8_t dirrection;					//命令字这里为0x0A
    uint8_t message_type;				//表示消息类型(主动发的消息ff,回复00,错误01)
    NodeBasicInfo member;
//    ChannelStatus channel1;
//    ChannelStatus channel2;
//    ChannelStatus channel3;
//    ChannelStatus channel4;
    uint16_t check;
    uint16_t tail;
}Frame_0A_byte;

/*收发业务消息统计*/
typedef struct 
{
    uint8_t  stat_flag;         //0:开始统计  1：停止统计 2：清零
    uint16_t eth_rx_packets;     //
    uint16_t eth_tx_packets;
    uint16_t eth_rx_bytes;
    uint16_t eth_tx_bytes;
    uint16_t audio_rx_packets;
    uint16_t audio_tx_packets;
}Info_0x06_Statistics;


/* 网内成员节点信息请求 */
typedef struct 
{
   uint16_t head;
   uint8_t  type;    //0:请求，1：应答
   uint8_t  src_id;
   uint8_t  dst_id;
   uint16_t tail;  
}MEM_REQUEST_FRAME;

/* 网内成员节点信息应答 */
typedef struct 
{
   uint16_t head;
   uint8_t  type;
   uint8_t  src_id;
   uint8_t  dst_id;
   NodeBasicInfo member;
   uint16_t tail;  
}MEM_REPLY_FRAME;



#pragma pack(pop)  // 恢复默认对齐

void send_member_request(uint8_t id);
void process_member_info(char* info,int size);

int uart_init(void);

void get_ui_Thread(void* arg) ;
void write_ui_Thread(void* arg);

#endif