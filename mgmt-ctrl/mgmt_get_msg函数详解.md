# mgmt_get_msg 函数详细讲解

## 函数定义
```c
void mgmt_get_msg(void)
```

## 函数职责
这是一个**管理消息获取线程函数**，是网络管理系统的核心线程，负责周期性地收集和处理网络拓扑信息。

---

## 核心功能概览

| 功能 | 说明 |
|------|------|
| **拓扑发现** | 从内核获取路由表和虚拟网卡信息 |
| **邻居统计** | 收集所有邻居节点的ID和MCS编码方式 |
| **报文构造** | 构建多层协议报文（以太网、IP、UDP、管理头） |
| **数据库更新** | 将网络信息写入SQLite数据库（系统表、链路表） |
| **角色处理** | 根据节点角色（网关/邻居）发送相应的拓扑信息 |
| **版本检查** | 验证各模块版本一致性 |

---

## 代码分段详解

### 1. 变量声明与初始化（第2150-2300行）

#### 核心数据结构
```c
struct mgmt_send self_msg;           // 自身节点的完整信息
struct routetable route_msg;         // 路由表信息
```

#### 拓扑交互
```c
Smgmt_header topo_header;            // 拓扑消息头
struct topo_data topomsg;            // 拓扑数据内容
```

#### 邻居信息数组
```c
int neighid_info[32];                // 存储最多32个邻居的节点ID
uint8_t mcs_all[NET_SIZE];          // 存储邻居的MCS编码方式（调制编码方案）
```

#### 报文缓冲区
```c
char buf[2048];                      // 多层协议报文缓冲区
int len;                             // 报文最终长度
int offset;                          // 动态偏移量（用于逐个添加邻居IP）
```

#### 协议头指针
```c
ethernet_header_t* ehdr;  // 以太网头（MAC地址等）
ip_header* iphdr;         // IP头（版本、TTL、源/目的IP等）
udp_header* udphdr;       // UDP头（源/目的端口等）
Smgmt_header* hmsg;       // 管理协议头
Snodefind* snodefind;     // 节点发现消息数据
```

#### 地址配置
```c
// MAC地址
uint8_t dstmac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};  // 广播MAC
uint8_t srcmac[6] = {0x00,0x0a,0x35,0x00,0x1e,0x54};  // 基地址
srcmac[5] = SELFID;  // 最后一字节设置为本节点ID

// IP地址
char dstip[4] = {0xc0,0xa8,0xff,0xff};  // 广播IP: 192.168.255.255
char srcip[4] = {0xc0,0xa8,0x02,0x01};  // 基IP: 192.168.2.x
srcip[3] = SELFID;   // 最后一字节设置为本节点ID
```

---

### 2. 协议头初始化（第2260-2295行）

#### 以太网头设置
```c
memcpy(ehdr->dest_mac_addr, dstmac, 6);  // 复制目的MAC（广播）
memcpy(ehdr->src_mac_addr, srcmac, 6);   // 复制源MAC
ehdr->ethertype = 0x0008;                // 协议类型为IP
```

#### IP头设置
```c
iphdr->ver_ihl = (4 << 4 | 5);           // IPv4, IHL=5(20字节头)
iphdr->tos = 0;                          // 服务类型
iphdr->ttl = 50;                         // 生存时间
iphdr->proto = IPPROTO_UDP;              // 协议类型UDP
// 源/目的地址、校验和稍后计算
```

#### UDP头设置
```c
udphdr->sport = htons(16000);            // 源端口
udphdr->dport = htons(7700);             // 目的端口
udphdr->len = 0;                         // 长度后续计算
udphdr->crc = 0x0000;                    // 校验和
```

---

### 3. 邻居信息初始化（第2297-2306行）

在开始主循环前，预初始化所有32个节点的链路表：

```c
for(j=1; j<33; j++) {
    reset_systeminfo_table(j);  // 清空系统表中该节点的记录
    
    // 清空链路表
    memset(&stlinkdata, 0, sizeof(stLink));
    stlinkdata.m_stNbInfo[j-1].nbid1 = j;
    stlinkdata.m_stNbInfo[j-1].snr1 = 0;       // 信噪比
    stlinkdata.m_stNbInfo[j-1].getlv1 = 0;     // 接收电平
    stlinkdata.m_stNbInfo[j-1].flowrate1 = 0;  // 吞吐率
    updateData_linkinfo(&stlinkdata, j-1, SELFID);  // 写入数据库
}
```

**目的**：确保数据库中所有节点记录都被初始化，避免脏数据。

---

### 4. 主循环：周期性信息收集（第2308行开始）

```c
while (TRUE) {
    // 每次循环执行以下操作：
```

#### 4.1 清空消息结构
```c
bzero(&self_msg, sizeof(struct mgmt_send));
node_num = 1;  // 初始化为1（计数本身）
offset = sizeof(ehdr) + sizeof(iphdr) + sizeof(udphdr) + sizeof(hmsg) + sizeof(snodefind);
```

#### 4.2 从内核获取实时信息
```c
// 通过netlink接口与内核通信
mgmt_netlink_get_info(0, MGMT_CMD_GET_ROUTE_INFO, NULL, (char*)&route_msg);
mgmt_netlink_get_info(0, MGMT_CMD_GET_VETH_INFO, NULL, (char*)&self_msg);
```

**获取内容**：
- 路由表信息（节点间的路由关系）
- 虚拟网卡信息（接口配置、邻居链路质量等）

#### 4.3 设置报文元数据
```c
self_msg.seqno = seqno++;  // 序列号递增（用于排序和去重）
self_msg.node_id = SELFID; // 本节点ID
```

#### 4.4 构造节点发现报文
```c
snodefind->selfid = htons(SELFID);     // 本节点ID（网络字节序）
snodefind->selfip = iphdr->saddr;      // 本节点IP地址
printf("node_%d has %d neigh\r\n", SELFID, self_msg.neigh_num);
```

#### 4.5 遍历邻居并构造报文
```c
for (i = 0; i < self_msg.neigh_num; i++) {
    if (self_msg.msg[i].mcs != 0x0f && self_msg.msg[i].node_id != SELFID) {
        node_num++;  // 有效邻居计数
        
        // 在报文缓冲区中添加邻居IP地址
        ipaddr = (int*)(buf + offset);
        *ipaddr = htonl(0xc0a80200 + self_msg.msg[i].node_id);  // 192.168.2.x
        offset += sizeof(int);
        
        // 保存邻居信息用于后续处理
        neighid_info[i] = self_msg.msg[i].node_id;  // 邻居ID
        mcs_all[i] = self_msg.msg[i].mcs;            // MCS编码方式
    }
}
```

**过滤条件解析**：
- `mcs != 0x0f`：MCS有效（0x0f表示无效链路）
- `node_id != SELFID`：排除自己

#### 4.6 计算报文长度并设置各层头部
```c
snodefind->node_num = htons(node_num);  // 总节点数（包括本身）
len = offset;  // 最终报文长度

// 从后向前计算各层长度
hmsg->mgmt_len = htons(len - (ehdr_size + iphdr_size + udphdr_size + hmsg_size));
udphdr->len = htons(len - ehdr_size - iphdr_size);
iphdr->tlen = htons(len - ehdr_size);
iphdr->crc = ipCksum((void*)iphdr, 20);  // 计算IP校验和
```

#### 4.7 根据节点角色发送拓扑信息
```c
// 网关节点向网管发送
if (is_conned == 1) {
    send_topo_msg(wg_addr, self_msg);    // 发送拓扑数据
    send_topo_request();                  // 请求邻居拓扑
}

// 邻居节点向网关发送
if (gotRequest == 1) {
    send_topo_msg(gate_addr, self_msg);  // 响应网关请求
}
```

#### 4.8 版本一致性检查
```c
// 检查veth（虚拟网卡）模块版本
sprintf(version_compare, "V%d.%d.%d", 
    self_msg.veth_version[1], 
    self_msg.veth_version[2], 
    self_msg.veth_version[3]);
if (strcmp(version, version_compare) != 0) {
    // 版本不一致处理
}

// 类似检查agent（代理）和ctrl（控制）模块版本
```

---

## 关键流程图

```
main() 启动线程
    ↓
mgmt_get_msg()
    ↓
初始化协议头、MAC/IP地址、邻居表
    ↓
┌─────────────────────────────────────────┐
│       WHILE(TRUE) 主循环                 │
├─────────────────────────────────────────┤
│ 1. 从内核获取拓扑信息 (netlink)        │
│ 2. 构造以太网/IP/UDP/管理报文          │
│ 3. 遍历邻居，添加IP地址到报文          │
│ 4. 计算各层报文长度和校验和            │
│ 5. 根据角色发送拓扑消息                │
│ 6. 更新数据库（系统表、链路表）        │
│ 7. 版本检查                            │
│ 8. 延时后回到步骤1                     │
└─────────────────────────────────────────┘
```

---

## 数据流向

```
内核网络栈 (netlink)
    ↓
mgmt_netlink_get_info()
    ↓
self_msg (本节点信息 + 邻居列表)
    ↓
┌─ 本节点信息 → 报文头
│
└─ 邻居列表 → 报文数据区（邻居IP地址）
    ↓
完整报文构造
    ↓
发送 (UDP 7700 端口)
    ↓
同步到SQLite数据库
```

---

## 重要常量

| 常量 | 值 | 含义 |
|------|-----|------|
| `HEAD` | 0x4C4A | 管理报文头标识 |
| `NET_SIZE` | 56/112等 | 根据设备类型的最大节点数 |
| `MGMT_NODEFIND` | - | 节点发现消息类型 |
| `7700` | UDP端口 | 拓扑信息发送端口 |
| `16000` | UDP端口 | 管理信息源端口 |
| `192.168.255.255` | 广播IP | 组播/广播地址 |
| `0x0f` | MCS值 | 无效链路标记 |

---

## 线程特性

- **周期循环**：持续运行，不主动退出
- **实时性**：每个循环周期约为数百毫秒（因netlink查询延时）
- **并发安全**：
  - 使用 `sqlite3_mutex1` 保护数据库访问
  - 所有邻居信息通过 `Lock/Unlock` 机制保护
- **内存使用**：栈空间约 2KB（主要是 2048B 的报文缓冲区）

---

## 与其他模块的交互

| 模块 | 交互方式 | 用途 |
|------|---------|------|
| **mgmt_netlink** | 函数调用 | 获取内核网络信息 |
| **sqlite_unit** | 函数调用 | 更新数据库 |
| **wg_config** | 数据引用 | 使用宽带系统配置 |
| **socket模块** | 隐式 | 内核通过netlink传递数据 |

---

## 故障排查

### 常见问题

1. **邻居数量为0**
   - 检查netlink是否正常工作
   - 确认内核驱动已加载

2. **报文发送失败**
   - 检查UDP端口7700是否被占用
   - 确认网络接口配置

3. **数据库写入缓慢**
   - 检查SQLite锁是否有竞争
   - 可调整 `sqlite3_busy_timeout`

4. **版本不一致警告**
   - 可能是模块升级不同步
   - 不影响功能，仅作提示

