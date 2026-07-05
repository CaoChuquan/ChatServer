#pragma once

#include <string>
#include <vector>

class OfflineMessage {
public:
    // 存储用户离线消息
    void store(int id, std::string msg);
    // 删除用户离线消息
    void remove(int id);
    // 查找用户离线消息
    std::vector<std::string> query(int id);
};

