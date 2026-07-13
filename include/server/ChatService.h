#pragma once

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "GroupModel.h"
#include "json.hpp"

using nlohmann::json;
using namespace muduo;
using namespace muduo::net;

// 聊天服务器业务类
class ChatService {
public:
    using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

    // 获取唯一实例
    static ChatService* instance();

    // 处理登录和注册业务
    void loginServer(const TcpConnectionPtr&, json&, Timestamp);
    void registerServer(const TcpConnectionPtr&, json&, Timestamp);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseExpection(const TcpConnectionPtr&);
    // 用于服务器异常后业务重置
    void reset();

    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr&, json&, Timestamp);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr&, json&, Timestamp);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr&, json&, Timestamp);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr&, json&, Timestamp);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr&, json&, Timestamp);
private:
    ChatService();

    // 储存业务 id 和其对应业务处理方法
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    // 存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> userConnMap_;

    std::mutex mutex_;

    GroupModel groupModel_;
};