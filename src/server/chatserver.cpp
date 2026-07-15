#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <muduo/base/Logging.h>

#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

// 启动服务
void ChatServer::start()
{
    _server.start();
}

// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开链接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();

    size_t pos = 0;
    while (pos < buf.size())
    {
        size_t end = buf.find('\0', pos);
        if (end == string::npos)
        {
            end = buf.size();
        }

        string msg(buf, pos, end - pos);
        if (msg.empty())
        {
            pos = end + 1;
            if (end == buf.size()) break;
            continue;
        }

        msg = msg.substr(0, msg.find_last_not_of("\r\n\t ") + 1);
        if (msg.empty())
        {
            pos = end + 1;
            continue;
        }

        try
        {
            json js = json::parse(msg);
            auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
            msgHandler(conn, js, time);
        }
        catch (const json::exception &e)
        {
            LOG_ERROR << "json parse error: " << e.what() << " | msg: " << msg;
        }

        pos = end + 1;
        if (end == buf.size()) break;
    }
}