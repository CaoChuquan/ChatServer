#include "ChatService.h"
#include "public.h"
#include "UserModel.h"
#include "OfflineMessageModel.h"
#include "FriendModel.h"
#include <string>
#include <muduo/base/Logging.h>

using std::string;
using std::vector;

// 获取单例对象
ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

// 注册消息以及对应的事件回调
ChatService::ChatService() {
    msgHandlerMap_.insert({ LOGIN_MSG, 
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->loginServer(c, j, t); } });
    msgHandlerMap_.insert({ REGISTER_MSG, 
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->registerServer(c, j, t); } });
    msgHandlerMap_.insert({ ONE_CHAT_MSG, 
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->oneChat(c, j, t); } });
    msgHandlerMap_.insert({ ADD_FRIEND_MSG,
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->addFriend(c, j, t); } });
    msgHandlerMap_.insert({ CREATE_GROUP_MSG,
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->createGroup(c, j, t); } });
    msgHandlerMap_.insert({ ADD_GROUP_MSG,
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->addGroup(c, j, t); } });
    msgHandlerMap_.insert({ GROUP_CHAT_MSG,
        [this](const TcpConnectionPtr& c, json& j, Timestamp t)
        { this->groupChat(c, j, t); } });
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
        // 登录成功，更新用户状态
        userUpdateState(enter_id, true);

        // 记录用户连接信息
        {
            std::lock_guard<std::mutex> lock(mutex_);
            userConnMap_.insert({enter_id, conn});
        }

        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 0;
        // 查询用户是否有离线消息
        vector<std::string> vec = offlineMessageQuery(enter_id);
        if (!vec.empty()) {
            response["offlinemsg"] = vec;
            // 读取该用户离线消息后，把该用户的离线消息删除
            offlineMessageRemove(enter_id);
        }
        // 查询用户的好友信息并返回
        vector<User> userVec = friendQuery(enter_id);
        if (!userVec.empty()) {
            vector<string> tempVec;
            for (auto user : userVec) {
                json js;
                js["id"] = user.id_;
                js["name"] = user.name_;
                js["state"] = user.state_;
                tempVec.push_back(js.dump());
            }
            response["friends"] = tempVec;
        }

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

    // 注册
    int id = userRegister(name, password);
    if (id != -1) {
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
        userUpdateState(id, false);
    }
}

void ChatService::oneChat(const TcpConnectionPtr& conn, 
    json& js, Timestamp time)
{
    int to_id = js["to"].get<int>();

    bool userState = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = userConnMap_.find(to_id);
        if (it != userConnMap_.end()) {
            // to_id 在线，直接转发消息
            (it->second)->send(js.dump());
            return;
        }
    }

    // to_id 不在线，存储离线消息
    offlineMessageStore(to_id, js.dump());
}

void ChatService::reset() {
    // 把 online 状态的用户设置成 offline
    userResetState();
}

void ChatService::addFriend(const TcpConnectionPtr& conn, 
    json& js, Timestamp time) 
{
    int user_id = js["id"].get<int>();
    int friend_id = js["friendid"].get<int>();

    // 存储好友信息
    friendInsert(user_id, friend_id);
}

void ChatService::createGroup(const TcpConnectionPtr& conn, 
json& js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if (groupModel_.createGroup(group)) {
        groupModel_.addGroup(user_id, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, 
json& js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["groupid"].get<int>();
    groupModel_.addGroup(user_id, group_id, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr& conn, 
json& js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["groupid"].get<int>();
    vector<int> useridVec = groupModel_.queryGroupUsers(user_id, group_id);

    std::lock_guard<std::mutex> lock(mutex_);
    for (int id : useridVec) {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end()) {
            // 转发群消息
            it->second->send(js.dump());
        }
        else {
            // 存储离线消息
            offlineMessageStore(id, js.dump());
        }
    }
}