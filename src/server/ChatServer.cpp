#include "ChatServer.h"
#include "json.hpp"
#include "ChatService.h"

using nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, 
    const InetAddress& listenAddr,
    const std::string& nameArg)
    : server_(loop, listenAddr, nameArg)
    , loop_(loop)
{
    // 注册连接回调
    server_.setConnectionCallback([this](const TcpConnectionPtr& t) {
      this->onConnection(t);
    });

    // 注册消息回调
    server_.setMessageCallback([this] (const TcpConnectionPtr& p, Buffer* b, Timestamp t) 
        { this->onMessage(p, b, t); });

    // 设置线程数量
    server_.setThreadNum(4);    
}

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {

    } else {
        conn->shutdown(); // 释放资源
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) {
    std::string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf);  // 把接收到的数据反序列化为 json 对象
    auto msghandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息对应的处理器，执行相应的业务
    msghandler(conn, js, time);
}