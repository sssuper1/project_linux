/*
 * virt_eth_util.h
 *
 *  Created on: 2020-4-15
 *      Author: lu
 */

#ifndef VIRT_ETH_UTIL_H_
#define VIRT_ETH_UTIL_H_

#include <linux/types.h>
#define wlan_addr_eq(addr1, addr2)                        (memcmp((void*)(addr1), (void*)(addr2), 6)==0)

#define MAC_FRAME_CTRL1_MASK_TYPE		0x0C
#define MAC_FRAME_CTRL1_MASK_SUBTYPE	0xF0
//Frame types (Table 8-1)
#define MAC_FRAME_CTRL1_TYPE_MGMT		0x00
#define MAC_FRAME_CTRL1_TYPE_CTRL		0x04
#define MAC_FRAME_CTRL1_TYPE_DATA		0x08
#define MAC_FRAME_CTRL1_TYPE_RSVD		0x0C

//#define WLAN_IS_CTRL_FRAME(f) ((((mac_header_cstdd*)f)->frame_control) & MAC_FRAME_CTRL1_TYPE_CTRL)

//Frame sub-types (Table 8-1)
//Management (MAC_FRAME_CTRL1_TYPE_MGMT) sub-types
#define MAC_FRAME_CTRL1_SUBTYPE_ASSOC_REQ		(MAC_FRAME_CTRL1_TYPE_MGMT | 0x00)
#define MAC_FRAME_CTRL1_SUBTYPE_ASSOC_RESP		(MAC_FRAME_CTRL1_TYPE_MGMT | 0x10)
#define MAC_FRAME_CTRL1_SUBTYPE_REASSOC_REQ		(MAC_FRAME_CTRL1_TYPE_MGMT | 0x20)
#define MAC_FRAME_CTRL1_SUBTYPE_REASSOC_RESP	(MAC_FRAME_CTRL1_TYPE_MGMT | 0x30)
#define MAC_FRAME_CTRL1_SUBTYPE_PROBE_REQ		(MAC_FRAME_CTRL1_TYPE_MGMT | 0x40)
#define MAC_FRAME_CTRL1_SUBTYPE_PROBE_RESP		(MAC_FRAME_CTRL1_TYPE_MGMT | 0x50)
#define MAC_FRAME_CTRL1_SUBTYPE_BEACON 			(MAC_FRAME_CTRL1_TYPE_MGMT | 0x80)
#define MAC_FRAME_CTRL1_SUBTYPE_ATIM			(MAC_FRAME_CTRL1_TYPE_MGMT | 0x90)
#define MAC_FRAME_CTRL1_SUBTYPE_DISASSOC		(MAC_FRAME_CTRL1_TYPE_MGMT | 0xA0)
#define MAC_FRAME_CTRL1_SUBTYPE_AUTH  			(MAC_FRAME_CTRL1_TYPE_MGMT | 0xB0)
#define MAC_FRAME_CTRL1_SUBTYPE_DEAUTH  		(MAC_FRAME_CTRL1_TYPE_MGMT | 0xC0)
#define MAC_FRAME_CTRL1_SUBTYPE_ACTION			(MAC_FRAME_CTRL1_TYPE_MGMT | 0xD0)
//Control (MAC_FRAME_CTRL1_TYPE_CTRL) sub-types
#define MAC_FRAME_CTRL1_SUBTYPE_BLK_ACK_REQ		(MAC_FRAME_CTRL1_TYPE_CTRL | 0x80)
#define MAC_FRAME_CTRL1_SUBTYPE_BLK_ACK			(MAC_FRAME_CTRL1_TYPE_CTRL | 0x90)
#define MAC_FRAME_CTRL1_SUBTYPE_PS_POLL			(MAC_FRAME_CTRL1_TYPE_CTRL | 0xA0)
#define MAC_FRAME_CTRL1_SUBTYPE_RTS				(MAC_FRAME_CTRL1_TYPE_CTRL | 0xB0)
#define MAC_FRAME_CTRL1_SUBTYPE_CTS				(MAC_FRAME_CTRL1_TYPE_CTRL | 0xC0)
#define MAC_FRAME_CTRL1_SUBTYPE_ACK				(MAC_FRAME_CTRL1_TYPE_CTRL | 0xD0)
#define MAC_FRAME_CTRL1_SUBTYPE_CF_END			(MAC_FRAME_CTRL1_TYPE_CTRL | 0xE0)
#define MAC_FRAME_CTRL1_SUBTYPE_CF_END_CF_ACK	(MAC_FRAME_CTRL1_TYPE_CTRL | 0xF0)
//Data (MAC_FRAME_CTRL1_TYPE_DATA) sub-types
#define MAC_FRAME_CTRL1_SUBTYPE_DATA			(MAC_FRAME_CTRL1_TYPE_DATA | 0x00)
#define MAC_FRAME_CTRL1_SUBTYPE_QOSDATA			(MAC_FRAME_CTRL1_TYPE_DATA | 0x80)
#define MAC_FRAME_CTRL1_SUBTYPE_NULLDATA		(MAC_FRAME_CTRL1_TYPE_DATA | 0x40)
#define MAC_FRAME_CTRL1_SUBTYPE_L1DATA		    (MAC_FRAME_CTRL1_TYPE_DATA | 0x20)

////////////////////////////////////////////////////////////////////////
#define TX_FRAME_INFO_FLAGS_REQ_TO                               0x01
#define TX_FRAME_INFO_FLAGS_FILL_TIMESTAMP                       0x02
#define TX_FRAME_INFO_FLAGS_FILL_DURATION                        0x04
#define TX_FRAME_INFO_FLAGS_WAIT_FOR_LOCK						 0x10
#define TX_FRAME_INFO_FLAGS_FILL_UNIQ_SEQ                        0x20


#define DHCP_BOOTP_FLAGS_BROADCAST                         0x8000
#define DHCP_MAGIC_COOKIE                                  0x63825363
#define DHCP_OPTION_TAG_TYPE                               53
#define DHCP_OPTION_TYPE_DISCOVER                          1
#define DHCP_OPTION_TYPE_OFFER                             2
#define DHCP_OPTION_TYPE_REQUEST                           3
#define DHCP_OPTION_TYPE_ACK                               5
#define DHCP_OPTION_TAG_IDENTIFIER                         61
#define DHCP_OPTION_END                                    255
#define DHCP_HOST_NAME                                     12

#define IPV4_PROT_UDP                                      0x11
#define IPV4_PROT_TCP                                      0x06
#define IPV4_PROT_ICMP                                     0x01
#define MANAGEMENT                                         0xff


#define UDP_SRC_PORT_BOOTPC                                68
#define UDP_SRC_PORT_BOOTPS                                67

#define UDP_DST_PORT_MANAGEMENT                            50001
#define UDP_DST_PORT_9361                                  50002

#define UDP_PORT_54321                                     54321
#define UDP_PORT_54322                                     54322
#define UDP_PORT_UCDS                                      60000


#define ETH_TYPE_CTRL                                      0x0101
#define ETH_TYPE_ARP                                       0x0608
#define ETH_TYPE_IP                                        0x0008
#ifdef Docker_Qualnet
#define ETH_TYPE_BAT                                       0x0543
#elif defined Zynq_Platform
#define ETH_TYPE_BAT                                       0x4573
#endif
#define BATADV_IV_OGM                                      0x00
#define BATADV_UNICAST                                     0x40
#define BATADV_BCAST                                       0x01
#define BATADV_SNR                                         0x06
#define BATADV_JGK                                         0x08

#define BATADV_VERSION                                     0x0f

#define LLC_SNAP                                           0xAA
#define LLC_CNTRL_UNNUMBERED                               0x03
//#define LLC_TYPE_ARP                                       0x0608
//#define LLC_TYPE_IP                                        0x0008
//#define LLC_TYPE_WLAN_LTG                                  0x9090              // Non-standard type for LTG packets
//#define LLC_TYPE_BAT	                                   0x4573


#define LLC_TYPE_ARP                                    0x06
#define LLC_TYPE_IP                                     0x00
#define LLC_TYPE_WLAN_LTG                               0x90
#define LLC_TYPE_BAT                                    0x45



#ifdef Docker_Qualnet
#define ETH_PAYLOAD_OFFSET								   ( sizeof(mac_header_cstdd) + sizeof(llc_header_t) )
#elif defined Zynq_Platform
#define ETH_PAYLOAD_OFFSET								   ( sizeof(mac_header_cstdd) + sizeof(llc_header_t) - sizeof(ethernet_header_t) )
#endif

#define UCDS_NUM 64

#define NIPQUAD(addr) \
        ((unsigned char*)&addr)[0],\
        ((unsigned char*)&addr)[1],\
        ((unsigned char*)&addr)[2],\
        ((unsigned char*)&addr)[3]


typedef struct {
    u8                       dest_mac_addr[ETH_ADDR_SIZE];                      // Destination MAC address
    u8                       src_mac_addr[ETH_ADDR_SIZE];                       // Source MAC address
    u16                      ethertype;                                        // EtherType
}__attribute__((packed)) ethernet_header_t;

typedef struct{
	u8  op;
	u8  htype;
	u8  hlen;
	u8  hops;
	u32 xid;
	u16 secs;
	u16 flags;
	u8  ciaddr[4];
	u8  yiaddr[4];
	u8  siaddr[4];
	u8  giaddr[4];
	u8  chaddr[MAC_ADDR_LEN];
	u8  chaddr_padding[10];
	u8  padding[192];
	u32 magic_cookie;
}__attribute__((packed)) dhcp_packet;

typedef struct {
	u8 icmp_type; //
	u8 icmp_code; //
	u16 icmp_checksum; 
	u16 icmp_id; 
	u16 icmp_sequence; 
}__attribute__((packed)) icmp_header_t;


typedef struct {
    u8                       version_ihl;                                      // [7:4] Version; [3:0] Internet Header Length
    u8                       dscp_ecn;                                         // [7:2] Differentiated Services Code Point; [1:0] Explicit Congestion Notification
    u16                      total_length;                                     // Total Length (includes header and data - in bytes)
    u16                      identification;                                   // Identification
    u16                      fragment_offset;                                  // [15:14] Flags;   [13:0] Fragment offset
    u8                       ttl;                                              // Time To Live
    u8                       protocol;                                         // Protocol
    u16                      header_checksum;                                  // IP header checksum
    u32                      src_ip_addr;                                      // Source IP address (big endian)
    u32                      dest_ip_addr;                                     // Destination IP address (big endian)
}__attribute__((packed)) ipv4_header_t;

typedef struct {
    u16                      htype;                                            // Hardware Type
    u16                      ptype;                                            // Protocol Type
    u8                       hlen;                                             // Length of Hardware address
    u8                       plen;                                             // Length of Protocol address
    u16                      oper;                                             // Operation
    u8                       sender_haddr[ETH_ADDR_SIZE];                       // Sender hardware address
    u8                       sender_paddr[IP_ADDR_SIZE];                        // Sender protocol address
    u8                       target_haddr[ETH_ADDR_SIZE];                       // Target hardware address
    u8                       target_paddr[IP_ADDR_SIZE];                        // Target protocol address
}__attribute__((packed)) arp_ipv4_packet_t;

typedef struct {
    u16                      src_port;                                         // Source port number
    u16                      dest_port;                                        // Destination port number
    u16                      length;                                           // Length of UDP header and UDP data (in bytes)
    u16                      checksum;                                         // Checksum
}__attribute__((packed)) udp_header_t;

typedef struct  
{  
    u16 src_port;    //源端口号  
    u16 dst_port;    //目的端口号  
    u32 seq_no;        //序列号  
    u32 ack_no;        //确认号  
    
    u8 thl;        //tcp头部长度  

    u8 flag;       //6位标志 
//    unsigned char thl:4;        //tcp头部长度  
//    unsigned char reserved_1:4; //保留6位中的4位首部长度  
//    unsigned char reseverd_2:2; //保留6位中的2位  
//    unsigned char flag:6;       //6位标志   

    u16 wnd_size;    //16位窗口大小  
    u16 chk_sum;     //16位TCP检验和  
    u16 urgt_p;      //16为紧急指针  
}__attribute__((packed)) tcp_header_t; 
#define TH_FIN 0x01   
#define TH_SYN 0x02   
#define TH_RST 0x04   
#define TH_PUSH 0x08   
#define TH_ACK 0x10   
#define TH_URG 0x20  


typedef struct{
	u8                       type;
	u8                       version;
	u8                       ttl;
	u8                       ttvn;
	u8                       dest[ETH_ADDR_SIZE];
}__attribute__((packed)) bat_unicast_header_t;

typedef struct  {
	u8     packet_type;
	u8     version;  /* batman version field */
	u8     ttl;
	u8     reserved;
	u32    seqno;
	u8     orig[ETH_ADDR_SIZE];
	/* "4 bytes boundary + 2 bytes" long to make the payload after the
	 * following ethernet header again 4 bytes boundary aligned
	 */
}__attribute__((packed)) bat_bcast_header_t;

typedef struct{
	u8                       type;
	u8                       version;
	u8                       ttl;
	u8                       flags;
	u32                      len;
}__attribute__((packed)) bat_snr;

typedef struct{
	u8                       type;
	u8                       version;
	u8                       ttl;
	u8                       flags;
	u32                      len;
}__attribute__((packed)) bat_jgk;

typedef struct{
	u8  frame_control_1;
	u8  frame_control_2;
	u16 duration_id;
	u8  address_1[MAC_ADDR_LEN];
	u8  address_2[MAC_ADDR_LEN];
	u8  address_3[MAC_ADDR_LEN];
	u16 sequence_control;
	//u8  address_4[MAC_ADDR_LEN];
}__attribute__((packed)) mac_header_80211;


typedef struct{
	u8 id;
	u8 dcs;
	u8 mcs[UCDS_NUM];
}__attribute__((packed)) ucds_pkt;;

u8 virt_eth_util_create_data_frame(struct sk_buff *skb,struct virt_eth_priv *veth_priv, llc_header_t *llc,u8* h_source,u8* h_dest,u16 tx_length);
u8 virt_eth_util_encap(struct sk_buff *skb,struct net_device *dev/*,  u32 eth_rx_len*/);
u8 virt_eth_util_send_data(void* mpdu, u16 length, u8 pre_llc_offset,u8* src_mac,u8* dst_mac,struct net_device *dev);
u8 virt_eth_util_rx_process(struct net_device *dev,u8* data);
u8 virt_eth_util_rx_process_test(struct net_device *dev,u8* data);


#endif /* VIRT_ETH_UTIL_H_ */
