#include "ChatService.h"
#include "public.h"
#include "database.h"
#include "User.h"
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
    std::string name = js["name"];
    std::string password = js["password"];

    // 组装 SQL 语句
    char sql[1024] = { 0 };
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', 'offline')",
        name.c_str(), password.c_str());
    // 执行 SQL 语句
    MySQL mysql;
    if (mysql.connect() && mysql.update(sql)) {
        // 获取插入成功的用户数据生成的主键 id
        int id = mysql_insert_id(mysql.getConnection());

        // 创建 User 对象
        User user(id, name, password);

        json response;
        response["msgid"] = REGISTER_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else {
        json response;
        response["msgid"] = REGISTER_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}