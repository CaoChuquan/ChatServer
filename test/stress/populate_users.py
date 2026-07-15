# 压力测试预处理脚本: 向数据库批量插入测试用户
import mysql.connector
import sys
import time

DB_CONFIG = {
    'host': '127.0.0.1',
    'port': 3306,
    'user': 'root',
    'password': '',
    'database': 'chat_server'
}

def insert_users(count, batch_size=500):
    try:
        conn = mysql.connector.connect(**DB_CONFIG, unix_socket='/var/run/mysqld/mysqld.sock')
        cursor = conn.cursor()

        t0 = time.time()
        print(f"正在批量插入 {count} 个用户...")
        total = 0

        for start in range(1, count + 1, batch_size):
            end = min(start + batch_size, count + 1)
            values = []
            for i in range(start, end):
                values.append(f"('stressuser{i}', '123456', 'offline')")

            sql = f"INSERT INTO User (name, password, state) VALUES {','.join(values)}"
            cursor.execute(sql)
            conn.commit()
            total += (end - start)

            progress = min(total, count)
            pct = 100.0 * progress / count
            elapsed = time.time() - t0
            qps = progress / max(elapsed, 0.001)
            print(f"  进度: {progress}/{count} ({pct:.1f}%) - {qps:.0f} users/sec", end='\r')

        cursor.close()
        conn.close()
        elapsed = time.time() - t0
        print(f"\n插入完成: {count} 用户, 耗时 {elapsed:.2f}秒, "
              f"速率: {count/elapsed:.0f} users/sec")

    except Exception as e:
        print(f"错误: {e}")
        sys.exit(1)

if __name__ == '__main__':
    count = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
    batch = int(sys.argv[2]) if len(sys.argv) > 2 else 500
    insert_users(count, batch)
