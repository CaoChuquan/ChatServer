#pragma once

#include "User.h"
#include <vector>

// 添加好友关系
void friendInsert(int user_id, int friend_id);
// 返回用户的好友列表
std::vector<User> friendQuery(int user_id);