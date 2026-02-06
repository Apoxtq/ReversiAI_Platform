# Network Module for ReversiAI_Platform

## 模块说明

v0.5.0 网络对战功能模块，基于Egaroucid GGS协议设计。

## 目录结构

```
network/
├── include/
│   └── network/
│       ├── message.hpp              # 消息类型定义
│       ├── networkclient.hpp        # TCP客户端
│       ├── networkserver.hpp        # P2P服务器
│       ├── networkdiscovery.hpp     # LAN发现
│       ├── gamesynchronizer.hpp     # 状态同步
│       ├── heartbeatmanager.hpp      # 心跳管理
│       ├── roommanager.hpp          # 房间管理
│       ├── reconnectionmanager.hpp   # 断线重连
│       ├── errorhandler.hpp         # 错误处理
│       ├── latencymonitor.hpp       # 延迟监控
│       └── chatmanager.hpp          # 聊天管理
├── src/
│   ├── message.cpp
│   ├── networkclient.cpp
│   ├── networkserver.cpp
│   ├── networkdiscovery.cpp
│   ├── gamesynchronizer.cpp
│   ├── heartbeatmanager.cpp
│   ├── roommanager.cpp
│   ├── reconnectionmanager.cpp
│   ├── errorhandler.cpp
│   ├── latencymonitor.cpp
│   └── chatmanager.cpp
└── CMakeLists.txt
```

## 参考项目

- **Egaroucid**: `src/console/ggs.hpp`, `src/console/gtp_command.hpp`
- **edax-reversi**: `src/ggs.c`
- **nlohmann/json**: `src/console/lib/json.hpp`

## License

本模块代码原创，参考代码遵循原项目GPL-3.0/MIT License。

