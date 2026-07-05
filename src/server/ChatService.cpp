#include "ChatService.h"
#include "public.h"
#include "UserModel.h"
#include <string>
#include <muduo/base/Logging.h>

using std::string;

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
    int enter_id = js["id"];
    string enter_pwd = js["password"];

    string user_pwd;
    string user_stat;
    // 查找用户
    char sql[100] = { 0 };
    sprintf(sql, "select * from User where id = %d", enter_id);
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr) {
                user_pwd = row[2];
                user_stat = row[3];
            }
            else {
                // 待定
                LOG_ERROR << "mysql_fetch_row() return nullptr";
            }
        }
        else {
            LOG_ERROR << "usesr_id:" << enter_id << " not found";
        }
    }
    else {
        LOG_FATAL << "Failed to connect to the database";
    }

    // 检测用户是否已经登录
    if (user_stat == "online") {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 2;
        response["errmsg"] = "该账号已登录";
        conn->send(response.dump());
    }

    // 查找到用户后，匹配密码
    if (enter_pwd == user_pwd) {
        // 更新用户状态
        UserUpdateState(enter_id, true);

        // 记录用户连接信息
        {
            std::lock_guard<std::mutex> lock(mutex_);
            userConnMap_.insert({enter_id, conn});
        }

        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 0;
        // response["id"] =
        // response["name"] =
        conn->send(response.dump());
    }
    else {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::registerServer(const TcpConnectionPtr& conn,
    json& js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

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
        // User user(id, name, password);

        json response;
        response["msgid"] = REGISTER_MSG_ACK;
        response["errno"] = 0;
        response["id"] = id;
        conn->send(response.dump());
    }
    else {
        json response;
        response["msgid"] = REGISTER_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::clientCloseExpection(const TcpConnectionPtr& conn) {
    int id = 0;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& it : userConnMap_) {
            // 从 map 表删除用户的连接信息
            if (it.second == conn) {
                id = it.first;
                userConnMap_.erase(id);
                break;
            }
        }
    }

    // 更新用户信息
    if (id != -1) {
        UserUpdateState(id, false);
    }
}