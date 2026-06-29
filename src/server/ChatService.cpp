#include "ChatService.h"
#include "public.h"
#include <string>
#include <muduo/base/Logging.h>

// 获取单例对象
ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    msgHandlerMap_.insert({ LOGIN_MSG, 
        [this](const TcpConnectionPtr& c, json& j, Timestamp t) 
        { this->loginServer(c, j, t); } });
    msgHandlerMap_.insert({ REGISTER_MSG, 
        [this](const TcpConnectionPtr& c, json& j, Timestamp t) 
        { this->registerServer(c, j, t); } }); 
}

ChatService::MsgHandler ChatService::getHandler(int msgid) {
    if (msgHandlerMap_.find(msgid) == msgHandlerMap_.end()) {
        // 如果没有对应的消息处理器
        return [msgid](const TcpConnectionPtr&, json&, Timestamp) {
            LOG_ERROR << "msgid: " << msgid << " cannot find!";
        };
    } 
    else {
        return msgHandlerMap_[msgid];
    }
}

void ChatService::loginServer(const TcpConnectionPtr& conn, 
    json& js, Timestamp time)
{
    LOG_INFO << "do login service!";
}

void ChatService::registerServer(const TcpConnectionPtr& conn,
    json& js, Timestamp time)
{
    LOG_INFO << "do regisert service!";
}