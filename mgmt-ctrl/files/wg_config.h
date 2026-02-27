/*
 * @Author: SunDG311 sdg18252543282@163.com
 * @Date: 2025-06-23 18:21:23
 * @LastEditors: SunDG311 sdg18252543282@163.com
 * @LastEditTime: 2025-10-30 17:13:41
 * @FilePath: \files\wg_config.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
*/
#ifndef WG_CONFIG_H_
#define WG_CONFIG_H_

#include "stdint.h"

#define SC_REPORT_PORT    10001   //自检状态上报端口
#define STATUS_REPORT_PORT  10001   //参数状态信息上报端口

#define MAX_XWG_PAIRS       20

#pragma pack(push, 1)

typedef enum
{
    KYLB_MODE_OPEN=0,
    KYLB_MODE_CLOSE=1
}KYLB_MODE;

typedef enum{
    CMD_SELF_TEST            =2,
    CMD_SEFL_TEST_ACK        =3,
    CMD_EXPERIMENT_CTRL      = 4,           // 试验控制指令 (4)
    CMD_EXPERIMENT_ACK       = 5,           //试验控制指令应答 (5)
    CMD_SELF_TEST_REPORT     = 40,          //宽带台自检状态回传 (40)  
    CMD_NET_STATUS_REPORT    = 41,          //宽带台设备网络状态上报 (41)
    CMD_DEV_CONFIG          = 42,           //宽带台设备参数配置指令 (42)
    CMD_CH1_CONFIG          = 43,           //带台业务通道1参数配置指令 (43)
    CMD_CONFIG_ACK          = 44,           //宽带台配置指令执行应答 (44)
    CMD_METRICS_REPORT      = 45,           //宽带台设备指标评估 (45)
    CMD_SELF_TEST_INFO      = 46,           //宽带台设备自检状态信息 (46)
    CMD_DEV_STATUS_INFO     = 47,           //宽带台设备状态信息 (47)
    CMD_CH1_STATUS_INFO     = 48,           //宽带台业务通道1状态信息 (48)
    CMD_ROUTING_TABLE_READ  = 49,           //宽带台设备路由表回读请求 (49)
    CMD_ROUTING_STATE       = 50,           //设备路由状态(50)
}DEVICE_CMD_TYPE;

// 控制指令--试验状态
typedef enum{
    CMD_STATE_READY=1,                    //试验准备
    CMD_STATE_START,
    CMD_STATE_PAUSE,
    CMD_STATE_CONTINUE,
    CMD_STATE_END

}CMD_STATE;

/* 路由协议类型 */
typedef enum {
    KD_ROUTING_OLSR    = 1,  // OLSR协议
    KD_ROUTING_AODV    = 2,  // AODV协议
    KD_ROUTING_CROSS_LAYER = 3  // 跨层抗干扰路由协议
} kd_routing_protocol_t;

/* 时隙长度 */
typedef enum {
    KD_TIMESLOT_0_5MS  = 0,  // 0.5ms
    KD_TIMESLOT_1MS    = 1,  // 1ms
    KD_TIMESLOT_1_25MS = 2,  // 1.25ms
    KD_TIMESLOT_2MS  = 3   // 2ms
} kd_timeslot_config_t;

/* 联通状态 */
typedef enum {
    CONNECTION_DISCONNECTED = 0, // 未联通
    CONNECTION_CONNECTED    = 1, // 已入网
    CONNECTION_SYNCHRONIZED = 2  // 已同步
} conn_Status;

typedef enum{
    PTYPE_SET=9,               //宽带电台配置指令
    PTYPE_SET_ACK=10,           //宽带电台配置指令应答
    PTYPE_SC_REPORT=12,         //宽带电台指标评估上报
    PTYPE_STATUS_REPORT=13      //宽带电台状态上报
}APP_HEAD_PTYPE;

//模块状态定义
typedef enum{
    MODULE_STATUS_NORMAL=1,        //正常
    MODULE_STATUS_FAULT=2,         //故障
    MODULE_STATUS_RESERVED=3       //未知
}MUDULE_STATUS;

// 1：场景服务器 4：业务模拟 7：宽台
typedef enum{
    DEVICE_TYPE_CJ=1,               
    DEVICE_TYPE_YW=4,
    DEVICE_TYPE_KD=7
}DEVICE_TYPE;


// 报文头
typedef struct{
    uint8_t     head;               //报文标识  240
    uint8_t     len;                //报头长度   30
    uint16_t    info_len;           //正文长度： 包括应用层头和应用层正文的长度
    uint8_t     packet_type;        //报文类型
    uint8_t     activity_type;      // 活动类型  1：试验，2：训练，3：演习
    uint16_t    send_type;          //发送端类型标识  1：场景服务器 4：业务模拟  7：宽台
    uint16_t    send_id;            //发送端设备ID标识
    uint32_t    timestamp_1;        //时间编码 (秒)
    uint16_t    timestamp_2;        //时间编码 (毫秒)
    uint8_t     packet_type_2;      //报文子类
    uint8_t     flag;               //报文压缩标志
    uint16_t    recv_type;          //接收端类型标识  1：场景服务器 4：业务模拟 7：宽台
    uint16_t     recv_id;            //接收端设备ID     0
    uint32_t    seq;                //包序号
    uint32_t    data_len;           //数据域长度
    
 

}APP_HEAD;

/* 自检指令 */
typedef struct {
    uint16_t time;    
    uint16_t type;      //6:宽带电台
}CMD_SC;

// 波形软件自检状态数据
typedef struct {
    uint8_t dev_type;            /* 设备类型 */
    uint8_t dev_id;              /* 设备编号 */
    uint8_t info_processing;     /* 综合信息处理模块 */
    uint8_t rf_frontend;         /* 射频前端 */
    uint8_t clock_source;        /* 时钟频率源 */
    uint8_t power_module;        /* 电源变换模块 */
    uint8_t battery;             /* 便携电池 */
    uint8_t interf_cancel;       /* 干扰对消设备 */
    uint8_t reserved;           /* 预留字段 */

}SELFCHECK_STATUS_INFO;


/*  试验控制指令    */ 
typedef struct{
    uint8_t state;              /* 试验状态 1：试验准备；,2：试验开始；,3：试验暂停；4：试验继续；5：试验结束。 */
    uint8_t mode;              //决策模式
    uint8_t scenario;          // 试验场景  1：外场试验训练；2：通信对抗注入式仿真；3：数字博弈仿真
    uint8_t reserved;          //预留位
}CMD_INFO;

/* 试验控制指令应答 */ 
typedef struct {
    uint8_t  type;          //指令类型    1：试验控制指令   2：预留
    uint8_t  state;         // 执行情况   0：成功
}CMD_ACK;

/* 设备网络状态 */
typedef struct{
    uint8_t dev_type;            /* 设备类型 1：宽带设备,2：设备*/
    uint16_t dev_id;             /* 设备ID */
    uint8_t connectivity;        /* 联通状态 1：在网,其它：未联通*/
    uint8_t kylb;               /* 空域滤波 0：开启 1：关闭*/
    uint8_t zsyxp;               /* 自适应选频 0：开启 1：关闭*/
    uint8_t neighbor_info[64];   /* 邻居信息 */
    double  longitude;           /* 经度 */
    double  latitude;            /* 纬度 */
    double  height;             /* 高度 */
    uint8_t reserved[4];             /* 预留字段 */    
}DEVCIE_NETWORK;

// 设备参数配置指令数据
typedef struct {
    uint8_t routing_prot;           //设备路由协议
    uint8_t work_mode;             //工作模式
    uint8_t decision_mode;           //决策模式
    uint16_t freq_set;              //自适应选频频率集                           
    uint8_t cluster_config;        //簇首配置
    uint8_t center_node;            //自主决策中心节点配置
    double  lon;                    /* 经度 */
    double  lat;                    /* 纬度 */
    double  height;                 /* 高度 */
    uint8_t network_role;           //网络角色        
    uint8_t reserved[4];
}DEVICE_PARAM_SET;

// 业务通道1参数配置指令数据
typedef struct {
    uint8_t     wave_type;              //波形体制  1:ofdm
    uint8_t     mcs_mode;               //调制编码方式  0-7 
    uint8_t     work_mode;              //工作模式    1:定频  4 自适应选频  6 硬件测试模式
    uint8_t     kylb;                   //空域滤波  0：停用，1启用
    uint8_t     bw;                     //带宽        
    uint16_t    freq;                   //工作频率    225MHz-2.5GHz，
    uint8_t     hop_rate;               //跳频速率    
    uint16_t     select_freq1;            //自适应选频1
    uint16_t     select_freq2;            //自适应选频2
    uint16_t     select_freq3;            //自适应选频3
    uint16_t     select_freq4;            //自适应选频4
    uint8_t     transmode;             //传输模式
    uint8_t     multi_access;           //多址方式
    uint8_t     tx_power;               //发射功率      0：2.5W、1：5W、2：10 W、3：20W
    uint8_t     tx_power_atten;         //发射功率衰减   0-90dB衰减，步进1dB
    uint8_t     slot_len;               //时隙长度
    uint8_t     sync_mode;              //    0：内同步，1：外同步
    uint8_t     reserved[4];  

}CHANNEL_PARAM_SET;

// 配置指令执行应答数据
typedef struct {
    uint8_t type;                       //指令类型      0：设备参数配置指令，2：业务通道1配置指令
    uint8_t state;                      //执行情况      0：成功
}PARAM_SET_ACK;

//设备指标评估数据
typedef struct{
    uint8_t ber[64];                    //误码率    
    uint32_t throughput[64];             //吞吐量
    int8_t  snr[64];                    //
    uint32_t  total_tx_cnt[64];         //总发射信息计数
    uint32_t  total_rx_cnt[64];         //总接收信息计数
   // uint8_t   reserved[4];
}DEVICE_EVALUATION_REPORT;

//设备自检状态信息数据
typedef struct{
    /* 综合模块状态 */
    int8_t temperature;               /* 综合模块温度 有符号char型。精度为℃，取值范围-127到﹢127    */
    uint8_t voltage;                   /* 综合模块电压  相对12V的偏移量，LSB表示0.1V，有符号数。例如，0x01表示12.1V，0x81表示11.9V*/
    uint8_t fan_status;                /* 综合模块风机转速状态 取值0-100，表示当前为0-100%*/
    uint8_t nav_lock_status;           /* 综合模块卫导锁定状态 0x00：已锁定，0x01：未锁定*/
    uint8_t sync_status;               /* 综合模块内外同步状态 0x00：外同步  0x01：内同步 */
    uint8_t clock_switch_status;       /* 综合模块内外时钟切换状态 0x00：板载时钟 0x01：外供时钟 */
    uint16_t panel_rs232_rx_count;      /* 综合模块与面板RS232接收消息记数 无符号数，取值范围0-65535  */
    uint16_t panel_rs232_tx_count;     /* 综合模块与面板RS232发送消息记数 */
    uint16_t module_power_rs422_rx_count;     /* 综合模块与电源变换RS422接收消息记数 无符号数，取值范围0-65535。*/
    uint16_t module_power_rs422_tx_count;     /* 综合模块与电源变换RS422发送消息记数 */
    uint16_t module_freq_rs422_rx_count;      /* 综合模块与频率源RS422接收消息记数 无符号数，取值范围0-65535*/
    uint16_t module_freq_rs422_tx_count;      /* 综合模块与频率源RS422发送消息记数 */
    uint16_t rf_rs422_rx_count;        /* 综合模块与射频前端RS422接收消息记数 无符号数，取值范围0-65535*/
    uint16_t rf_rs422_tx_count;        /* 综合模块与射频前端RS422发送消息记数 */
    uint8_t soc1_online;              /* 综合模块SOC1在线 0x00：程序未正常运行，0x01：程序正常运*/
    uint8_t soc2_online;               /* 综合模块SOC2在线 */
    uint8_t soc3_online;               /* 综合模块SOC3在线 */
    
    /* ADC/DAC状态 */
    uint8_t adc_ch1_status;            /* 综合模块通道1ADC状态 0x00：正常，0x01：异常*/
    uint8_t adc_ch2_status;            /* 综合模块通道2ADC状态 */
    uint8_t adc_ch3_status;            /* 综合模块通道3ADC状态 */
    uint8_t adc_ch4_status;            /* 综合模块通道4ADC状态 */
    uint8_t dac_ch1_status;            /* 综合模块通道1DAC状态 */
    uint8_t dac_ch2_status;            /* 综合模块通道2DAC状态 */
    uint8_t dac_ch3_status;            /* 综合模块通道3DAC状态 */
    uint8_t dac_ch4_status;            /* 综合模块通道4DAC状态 */
    uint8_t sense_adc_status;          /* 综合模块感知通道ADC状态  */
    
    /* 频率源状态 */
    uint8_t freq_power_fault;          /* 频率源上电/故障指示    无符号char，1表示故障，0表示正常，默认为0。*/
    uint8_t freq_ref1_ready;           /* 频率源输出参考时钟1就绪 无符号char，1表示就绪，0表示失锁，默认为0*/
    uint8_t freq_ref2_ready;           /* 频率源输出参考时钟2就绪 */
    uint8_t freq_sense_status;         /* 频率源通信感知状态指示 */
    uint8_t freq_12v_voltage;          /* 频率源12V供电电压 对12V的偏移量，LSB表示0.1V，有符号数。例如，0x01表示12.1V，0x81表示11.9V。*/
    int8_t freq_temperature;            /* 频率源温度 有符号char型。精度为℃，取值范围-127到﹢127*/
    uint16_t freq_rs422_rx_count;      /* 频率源RS422接收消息记数 */
    uint16_t freq_rs422_tx_count;      /* 频率源RS422发送消息记数 */
    uint16_t freq_word_count;          /* 频率源频率字下发次数记数 */
    uint16_t freq_pulse_count;         /* 频率源频率更新脉冲下发次数记数 */
    uint8_t freq_ch_power_status;      /* 频率源各通道加电状态指示 */
    uint8_t freq_lo_ready;             /* 频率源各通道输出本振就绪 */
    uint16_t freq_lo1_freq;            /* 频率源本振1对应频点 */
    uint16_t freq_lo2_freq;            /* 频率源本振2对应频点 */
    uint16_t freq_lo3_freq;            /* 频率源本振3对应频点 */
    uint16_t freq_lo4_freq;            /* 频率源本振4对应频点 */
    
    /* 电源变换状态 */
    uint8_t power_power_fault;         /* 电源变换上电/故障指示 */
    uint8_t power_temperature;         /* 电源变换温度 */
    uint16_t power_ac220_power;        /* 电源变换AC220输入功耗 */
    uint16_t power_dc24v_power;        /* 电源变换DC24V输入功耗 */
    uint16_t power_rs422_rx_count;     /* 电源变换RS422接收消息记数 */
    uint16_t power_rs422_tx_count;     /* 电源变换RS422发送消息记数 */
    uint8_t power_ch_power_status;     /* 电源变换各路加电控制状态 */
    uint8_t power_fault_status;        /* 电源变换各路输出故障指示 */
    uint8_t power_overload_status;     /* 电源变换各路过载状态指示 */
    
    /* 射频前端状态 */
    uint8_t rf_power_fault;            /* 射频前端上电/故障指示 */
    uint8_t rf_28v_voltage;            /* 射频前端28V供电电压 */
    uint8_t rf_12v_voltage;            /* 射频前端12V供电电压 */
    uint8_t rf_tx_power_status;        /* 射频前端发射功率检波状态 */
    uint8_t rf_antenna_l_vswr;         /* 射频前端天线L口驻波状态 */
    uint8_t rf_antenna_h1_vswr;        /* 射频前端天线H-1口驻波状态 */
    uint8_t rf_antenna_h2_vswr;        /* 射频前端天线H-2口驻波状态 */
    uint8_t rf_tx_rx_status;           /* 射频前端收发状态指示 */
    uint8_t rf_antenna_select;         /* 射频前端天线选择控制状态 */
    uint8_t rf_sense_status;           /* 射频前端通信感知状态 */
    uint8_t rf_power_status;           /* 射频前端加电状态指示 */
    uint8_t rf_ch_power_level;         /* 射频前端各通道功率等级 */
    
    /* 射频前端通道1 */
    uint8_t rf_ch1_rf_power;           /* 射频前端通道1接收射频功率检波 */
    uint8_t rf_ch1_if_power;           /* 射频前端通道1接收中频功率检波 */
    uint8_t rf_ch1_temp1;              /* 射频前端通道1温度1 */
    uint16_t rf_ch1_freq;              /* 射频前端通道1当前工作频点 */
    uint8_t rf_ch1_agc_atten;          /* 射频前端通道1当前AGC衰减值 */
    uint16_t rf_ch1_rs422_rx_count;    /* 射频前端通道1RS422接收消息记数 */
    uint16_t rf_ch1_rs422_tx_count;    /* 射频前端通道1RS422发送消息记数 */
    uint16_t rf_ch1_freq_word_count;   /* 射频前端通道1频率字下发次数记数 */
    uint16_t rf_ch1_agc_count;         /* 射频前端通道1AGC控制下发次数记数 */
    uint16_t rf_ch1_freq_pulse_count;  /* 射频前端通道1频率更新脉冲下发次数记数 */
    uint16_t rf_ch1_power_consumption; /* 射频前端通道1通道功耗 */
    uint8_t rf_ch1_bandwidth;          /* 射频前端通道1当前信号带宽 */
    uint8_t rf_ch1_power_adjust;       /* 射频前端通道1功率调整量 */
    
    /* 射频前端通道2 */
    uint8_t rf_ch2_rf_power;
    uint8_t rf_ch2_if_power;
    uint8_t rf_ch2_temp1;
    uint16_t rf_ch2_freq;
    uint8_t rf_ch2_agc_atten;
    uint16_t rf_ch2_rs422_rx_count;
    uint16_t rf_ch2_rs422_tx_count;
    uint16_t rf_ch2_freq_word_count;
    uint16_t rf_ch2_agc_count;
    uint16_t rf_ch2_freq_pulse_count;
    uint16_t rf_ch2_power_consumption;
    uint8_t rf_ch2_bandwidth;
    uint8_t rf_ch2_power_adjust;
    
    /* 射频前端通道3 */
    uint8_t rf_ch3_rf_power;
    uint8_t rf_ch3_if_power;
    uint8_t rf_ch3_temp1;
    uint16_t rf_ch3_freq;
    uint8_t rf_ch3_agc_atten;
    uint16_t rf_ch3_rs422_rx_count;
    uint16_t rf_ch3_rs422_tx_count;
    uint16_t rf_ch3_freq_word_count;
    uint16_t rf_ch3_agc_count;
    uint16_t rf_ch3_freq_pulse_count;
    uint16_t rf_ch3_power_consumption;
    uint8_t rf_ch3_bandwidth;
    uint8_t rf_ch3_power_adjust;
    
    /* 射频前端通道4 */
    uint8_t rf_ch4_rf_power;
    uint8_t rf_ch4_if_power;
    uint8_t rf_ch4_temp1;
    uint16_t rf_ch4_freq;
    uint8_t rf_ch4_agc_atten;
    uint16_t rf_ch4_rs422_rx_count;
    uint16_t rf_ch4_rs422_tx_count;
    uint16_t rf_ch4_freq_word_count;
    uint16_t rf_ch4_agc_count;
    uint16_t rf_ch4_freq_pulse_count;
    uint16_t rf_ch4_power_consumption;
    uint8_t rf_ch4_bandwidth;
    uint8_t rf_ch4_power_adjust;
    
    /* 电池状态 */
    uint8_t battery_level;             /* 电池当前电量 */
    uint8_t battery_self_test;         /* 电池自检状态 */
    uint8_t battery_voltage;           /* 电池当前电压 */
    uint8_t battery_temperature;       /* 电池温度 */
    uint16_t battery_rs422_rx_count;   /* 电池RS422接收消息记数 */
    uint16_t battery_rs422_tx_count;   /* 电池RS422发送消息记数 */
    
    //uint8_t reserved;              /* 预留 */
}DEVICE_SC_STATUS_REPORT;

//设备状态信息数据
typedef struct 
{
    uint8_t routing_prot;           //设备路由协议
    uint8_t work_mode;             //工作模式
    uint8_t decision_mode;           //决策模式
    uint16_t freq_set;              //自适应选频频率集                           
    uint8_t cluster_config;        //簇首配置
    uint8_t center_node;            //自主决策中心节点配置
    double  lon;                    /* 经度 */
    double  lat;                    /* 纬度 */
    double  height;                 /* 高度 */
    uint8_t network_role;           //网络角色        
    uint8_t reserved[4];
}DEVICE_STATUS_REPORT;


// 业务通道1状态信息上报
typedef struct {
    uint8_t     wave_type;              //波形体制  1:ofdm
    uint8_t     mcs_mode;               //调制编码方式  0-7 
    uint8_t     work_mode;              //工作模式    1:定频  4 自适应选频  6 硬件测试模式
    uint8_t     kylb;                   //空域滤波  0：停用，1启用
    uint8_t     bw;                     //带宽        
    uint16_t    freq;                   //工作频率    225MHz-2.5GHz，
    uint8_t     hop_rate;               //跳频速率    
    uint16_t     select_freq1;            //自适应选频1
    uint16_t     select_freq2;            //自适应选频2
    uint16_t     select_freq3;            //自适应选频3
    uint16_t     select_freq4;            //自适应选频4
    uint8_t     transmode;             //传输模式
    uint8_t     multi_access;           //多址方式
    uint8_t     tx_power;               //发射功率      0：2.5W、1：5W、2：10 W、3：20W
    uint8_t     tx_power_atten;         //发射功率衰减   0-90dB衰减，步进1dB
    uint8_t     slot_len;               //时隙长度
    uint8_t     sync_mode;              //同步模式
    uint8_t     reserved[4];    

}CHANNEL_PARAM_REPORT;


/* xwg 内容 */ 
typedef struct {
    char key[25];
    char value[100];
    int found;
} Node_Xwg_Pairs;



void read_node_xwg_file(const char* filename,Node_Xwg_Pairs* xwg_info,int num);
const char *get_value(Node_Xwg_Pairs* pairs,const char *key) ;
int get_int_value(Node_Xwg_Pairs* pairs,const char *key) ;
double get_double_value(Node_Xwg_Pairs* pairs,const char *key);



#pragma pack(pop)  // 恢复默认对齐
#endif
