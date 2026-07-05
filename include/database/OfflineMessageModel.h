#pragma once

#include <vector>
#include <string>

// 存储用户离线消息
void offlineMessageStore(int id, std::string msg);
// 删除用户离线消息
void offlineMessageRemove(int id);
// 查找用户离线消息
std::vector<std::string> offlineMessageQuery(int id);