#pragma once

// Server 和 Client 的公共文件

enum EnMsgTyep {
    LOGIN_MSG = 1,    // 1.登录消息
    LOGIN_MSG_ACK,    // 2.登录响应消息
    REGISTER_MSG,     // 3.注册消息
    REGISTER_MSG_ACK, // 4.注册相应消息
    ONE_CHAT_MSG      // 5.聊天消息
};