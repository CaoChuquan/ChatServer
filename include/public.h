#pragma once

// Server 和 Client 的公共文件

enum EnMsgTyep {
    LOGIN_MSG = 1,   // 登录消息
    LOGIN_MSG_ACK,   // 登录响应消息
    REGISTER_MSG,    // 注册消息
    REGISTER_MSG_ACK // 注册相应消息
};