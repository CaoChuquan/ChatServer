#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>

class ChatServer {
public:
    ChatServer(muduo::net::EventLoop* loop, 
        const muduo::net::InetAddress& listenAddr,
        const std::string& nameArg);
    // 启动服务    
    void start() { server_.start(); }
private:
    // 与新连接相关的回调函数
    void onConnection(const muduo::net::TcpConnectionPtr&);
    // 读写事件相关回调函数
    void onMessage(const muduo::net::TcpConnectionPtr&, 
        muduo::net::Buffer*, 
        muduo::Timestamp);

    muduo::net::TcpServer server_;
    muduo::net::EventLoop* loop_;
};