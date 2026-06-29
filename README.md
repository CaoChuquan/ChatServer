# chat-server

#### 分布式即时通讯服务器

&emsp;&emsp;基于 C++11、Muduo、MySQL、Redis、Nginx 实现的分布式即时通讯系统。支持用户注册、登录、好友管理、一对一聊天、群组聊天、离线消息存储等功能，并通过 Redis 发布订阅机制实现跨服务器消息通信，通过 Nginx 实现负载均衡与集群部署。

---
#### 项目特点
* 基于 Muduo 网络库开发高并发服务器
* Reactor + One Loop Per Thread 网络模型
* MySQL 持久化存储用户数据
* Redis 发布/订阅实现跨节点消息转发
* Nginx TCP 负载均衡实现集群部署
* JSON 消息序列化
* 支持离线消息存储与同步
---
#### 技术栈

|  技术   |    作用    |
| :---: | :------: |
| C++11 |  核心开发语言  |
| Mudo  |  网络通信框架  |
| MySQL |  数据持久化   |
| Redis | 跨服务器消息通信 |
| Nginx | TCP 负载均衡 |
| JSON  | 网络消息序列化  |
| Linux |   运行环境   |

---
#### 系统架构说明
1. 客户端首先连接 Nginx。

2. Nginx 将连接分发到后端 ChatServer。

3. 用户登录后连接保持在对应服务器。

4. 当消息发送给其他服务器上的用户时：


   - 发布消息到 Redis。

   - Redis 将消息转发到目标服务器。

   - 目标服务器推送消息给用户。

5. 离线用户消息存储到 MySQL。

  

---
##### 核心功能
##### 用户管理
* 用户注册
* 用户登录
* 用户注销
##### 好友系统

* 添加好友

* 查询好友列表
##### 群组系统
* 创建群组

* 加入群组

* 群聊消息
##### 消息系统

* 单聊
* 群聊
* 离线消息

---
#### Redis 集群通信

##### 问题
&emsp;&emsp;单机聊天服务器只能处理当前服务器上的用户。

例如：

用户A连接Server1

用户B连接Server2

A发送消息给B时：

Server1无法直接获取Server2中的连接对象。
##### 解决方案

利用 Redis 发布/订阅机制：
Server1：

publish(channel,msg)

↓

Redis

↓

Server2：

subscribe(channel)

↓
  
推送给用户B

实现服务器之间的消息转发。

---
#### 数据库设计
##### User


| 字段       | 说明   |

| -------- | ---- |

| id       | 用户ID |

| name     | 用户名  |

| password | 密码   |

| state    | 在线状态 |

  

##### Friend

  

| 字段       | 说明   |

| -------- | ---- |

| userid   | 用户ID |

| friendid | 好友ID |

  

##### OfflineMessage

  

| 字段      | 说明   |

| ------- | ---- |

| userid  | 用户ID |

| message | 离线消息 |

  

---

  

#### 编译运行

##### 环境

* CentOS 8.2

* GCC 11+

* CMake 3.10

* MySQL

* Redis

* Nginx

* Muduo
##### 编译

```bash

mkdir build

cd build

cmake ..

make

```
##### 启动

启动 MySQL

启动 Redis

启动 Nginx

启动多个 ChatServer 实例

```bash

./ChatServer 127.0.0.1 8000

  

./ChatServer 127.0.0.1 8001

  

./ChatServer 127.0.0.1 8002

```

客户端连接：


```bash

./ChatClient

```

  

---
#### 项目收获
* 理解 Reactor 网络编程模型

* 掌握 Muduo 网络库核心设计思想

* 熟悉 Redis 发布订阅机制

* 熟悉 MySQL 数据建模与数据访问层设计

* 掌握 Nginx TCP 负载均衡配置

* 理解分布式系统中的消息路由机制
___