#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>
#include <signal.h>

using namespace std;
using namespace muduo;
using namespace muduo::net;

// 处理服务器 ctrl+c 结束后，重置 User 的状态信息
void resetHandler(int) {
    ChatService::instance()->reset();
    exit(0);
}

int main() {
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
}