#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>

using namespace muduo;
using namespace muduo::net;

class ChatServer {
public:
    ChatServer(EventLoop* loop, 
        const InetAddress& listenAddr,
        const std::string& nameArg);
    // 启动服务    
    void start() { server_.start(); }
private:
    // 与新连接相关的回调函数
    void onConnection(const TcpConnectionPtr&);
    // 读写事件相关回调函数
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    TcpServer server_;
    EventLoop* loop_;
};