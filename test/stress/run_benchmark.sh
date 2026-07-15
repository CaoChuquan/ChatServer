#!/bin/bash
# 完整压力测试脚本 - 多并发级别
IP="127.0.0.1"
PORT="6000"
USERS=1000
MSGS=100
DURATION=10

echo "============================================================"
echo "  集群聊天服务器 - 完整压力测试"
echo "============================================================"
echo ""

CONCURRENCIES=(10 25 50 100)

for CONC in "${CONCURRENCIES[@]}"; do
    echo ""
    echo "========== 并发数: $CONC =========="

    # 重启服务
    pkill -9 ChatServer 2>/dev/null
    sleep 1
    mysql -u root chat -e "UPDATE user SET state = 'offline';" 2>/dev/null

    /workspace/bin/ChatServer $IP $PORT &
    SERVER_PID=$!
    sleep 2

    # 运行压测
    /workspace/bin/stress_test $IP $PORT $USERS $CONC $MSGS $DURATION
    EXIT_CODE=$?

    # 检查服务器是否存活
    if kill -0 $SERVER_PID 2>/dev/null; then
        echo "  [OK] 服务器存活"
    else
        echo "  [CRASH] 服务器崩溃! 退出码: $?"
    fi

    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    sleep 1
done

echo ""
echo "============================================================"
echo "  压力测试完成"
echo "============================================================"
