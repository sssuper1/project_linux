# recipes-apps/mgmt-ctrl 分析

`recipes-apps/mgmt-ctrl` 是一个用户空间应用程序，用于管理和控制无线网状网络节点。

## 核心功能

该应用程序作为一个多线程守护进程运行，负责节点的各种管理任务。

## 主要功能模块

1.  **多线程架构 (`main.c`)**:
    *   `mgmt_get_msg`: 负责定期上报节点状态。
    *   `mgmt_recv_web`: 处理来自 Web 界面的消息和指令。
    *   `mgmt_recv_msg`: 接收其他类型的消息。
    *   `sqlite_set_param`: 通过 SQLite 数据库设置系统参数。
    *   `gps_Thread`: 获取并处理 GPS 数据。
    *   `get_ui_Thread`/`write_ui_Thread`: 通过 UART 与外部 UI 设备交互。
    *   `audio_thread`/`play_audio_thread`: 处理音频相关功能。

2.  **内核交互 (`mgmt_netlink.c`)**:
    *   使用 **Generic Netlink** 套接字与自定义的内核模块进行通信。
    *   **获取信息**: 从内核查询路由表、虚拟以太网设备统计信息以及 MAC/PHY 层状态（如 RSSI, SNR, MCS）。
    *   **设置参数**: 向内核发送配置指令，如修改频率、带宽、发射功率、MCS 方案和工作模式。

3.  **配置管理**:
    *   通过执行 `sed` 命令来修改 `/etc/node_xwg` 文件，实现配置的持久化。

4.  **数据库**:
    *   使用 `SQLite` 进行本地参数存储。
    *   集成了 `libmysqlclient`，具备与 MySQL 服务器交互的能力。

5.  **构建系统 (`mgmt-ctrl.bb`)**:
    *   这是一个 BitBake recipe，用于在 Yocto/Petalinux 环境中编译和打包该应用程序。
    *   依赖项包括 `libmysqlclient` 和 `libnl`。

## 总结

`mgmt-ctrl` 是一个综合性的管理程序，通过 Netlink 实现了用户空间与内核空间的高效通信，结合多线程、数据库和配置文件管理，完成了对无线节点的全面监控和控制。
