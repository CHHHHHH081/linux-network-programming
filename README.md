# Linux 网络编程（C++）

基于 C++ 的 Linux 网络编程学习仓库，从 UDP/TCP 基础 Socket 编程开始，逐步实践各种并发模型，并计划延伸到 select / poll / epoll 多路复用，对比不同 I/O 模型的实现与差异。

> 本仓库用于个人学习记录，会持续更新。

## 目录结构

```
linux-network-programming
├── TCP
│   ├── TCPNetCal          # TCP 网络计算器（进行中）
│   └── TCPSocket          # TCP C/S 远程命令执行（带白名单）
└── UDP
    ├── UDPEchoServer      # UDP 回显服务器
    ├── UDPTranslateSever  # UDP 翻译服务器（英译中）
    └── UDPChatRoom        # UDP 聊天室
```

## 项目介绍

### 1. TCP / TCPSocket —— 远程命令执行

一个 TCP C/S 架构的远程命令执行程序：客户端连接后发送命令，服务端通过 `popen` 执行并返回结果。

- **白名单机制**：仅允许执行 `ls`、`pwd`、`whoami` 等白名单内的安全命令，其余命令返回提示，避免任意命令执行风险。
- **并发处理**：基于线程池 + 互斥量 + 条件变量，服务端可同时服务多个客户端。
- 架构图见 [TCP/TCPSocket/README.md](TCP/TCPSocket/README.md)。

```bash
cd TCP/TCPSocket
make                      # 生成 tcpclient / tcpserver
./tcpserver <port>        # 启动服务端
./tcpclient <ip> <port>   # 启动客户端，输入命令
```

### 2. UDP / UDPEchoServer —— 回显服务器

最基础的 UDP 通信示例：客户端发送消息，服务端返回 `Hello, <消息>`。

```bash
cd UDP/UDPEchoServer
make
./udpserver <port>
./udpclient <server_ip> <server_port>
```

### 3. UDP / UDPTranslateSever —— 翻译服务器

在回显服务器基础上增加业务逻辑：服务端启动时从 `dictionary.txt` 加载词表到哈希表，客户端发送英文单词，服务端返回对应中文释义。

```bash
cd UDP/UDPTranslateSever
make
./udpserver <port>
./udpclient <server_ip> <server_port>   # 输入英文单词，返回中文
```

> `dictionary.txt` 格式为 `english: 中文`，可自行扩充词条。

### 4. UDP / UDPChatRoom —— 聊天室

一个简单的 UDP 聊天室：服务端维护在线用户列表，将任一用户的消息广播给所有在线用户，并结合线程池处理并发消息。发送 `QUIT` 退出。

```bash
cd UDP/UDPChatRoom
make
./udpserver <port>
./udpclient <server_ip> <server_port>
```

### 5. TCP / TCPNetCal —— 网络计算器（进行中）

基于 Socket 抽象接口设计的一个更完整的 TCP 项目，引入 `Protocal`（请求/响应的序列化与反序列化）、`InetAddr` 等分层封装，服务端使用 `fork` 多进程处理连接。当前 `.cc` 实现尚未完成，处于开发中。

## 组件

各项目复用的基础组件，随着学习深入逐步沉淀：

| 模块 | 说明 |
| --- | --- |
| `log.hpp` | 日志模块，基于策略模式，支持控制台/文件输出、日志等级过滤 |
| `InetAddr.hpp` | `sockaddr_in` 地址封装，提供 IP/端口解析与字符串化 |
| `Mutex.hpp` / `Cond.hpp` | POSIX 互斥量、条件变量的 RAII 封装 |
| `Thread.hpp` | 基于 `pthread` 的线程封装 |
| `threadpool.hpp` | 通用线程池模板 |
| `jsoncpp` | 第三方 JSON 解析库，用于协议序列化/反序列化（即将引入）|

## 技术栈

- **语言 / 标准**：C++17，`g++`
- **系统调用**：`socket`、`bind`、`listen`、`accept`、`connect`、`recvfrom`、`sendto` 等 POSIX Socket API
- **协议**：TCP（`SOCK_STREAM`）、UDP（`SOCK_DGRAM`）
- **并发**：多进程（`fork`）、多线程（`pthread`）、线程池、互斥量、条件变量
- **编译**：`Makefile`，`-std=c++17 -lpthread`

## 学习路线（以网络为主，并发为辅）

**网络主线**

- [x] UDP / TCP 基础 Socket 通信
- [ ] select / poll / epoll 多路复用
- [ ] Reactor 模型与高性能服务器

**并发处理（支撑网络的连接管理）**

- [x] 多进程（`fork`）模型
- [x] 多线程、线程池、同步与互斥
