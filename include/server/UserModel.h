#pragma once

#include "User.h"

// 操作 User 表的类
class UserModel {
public:
    // 增加用户
    bool insert(User& user);
    
};