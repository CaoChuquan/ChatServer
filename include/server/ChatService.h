#pragma once

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include "json.hpp"

using nlohmann::json;
using namespace muduo;
using namespace muduo::net;

// 聊天服务器业务类
class ChatService {
public:
    using MsgHandler = std::function<void(const TcpConnectionPtr&, json& js, Timestamp)>;

    // 处理登录和注册业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void reg(const TcpConnection& conn, json& js, Timestamp time);

    static ChatService* instance();
private:
    ChatService(); 

    // 储存业务 id 和其对应业务处理方法
    std::unordered_map<int, MsgHandler> msgHandlerMap_;
};

