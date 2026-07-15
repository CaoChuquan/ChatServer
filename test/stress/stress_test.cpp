/**
 * 集群聊天服务器 - 完整压力测试工具 v3
 * 每个测试场景独立运行，避免资源竞争
 */
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

using namespace std;
using namespace std::chrono;

// ===================== 配置 =====================
struct Config {
    string ip = "127.0.0.1";
    int port = 6000;
    int users = 1000;
    int concurrency = 50;
    int msgs = 100;
    int seconds = 10;
} g_cfg;

// ===================== 统计 =====================
struct BenchResult {
    int64_t total = 0;
    int64_t success = 0;
    int64_t fail = 0;
    double duration_sec = 0;
    double qps = 0;
    double avg_ms = 0, p50_ms = 0, p90_ms = 0, p95_ms = 0, p99_ms = 0, min_ms = 0, max_ms = 0;
};

mutex g_lat_mutex;
vector<double> g_lats;

static double now_ms() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

// ===================== 网络工具 =====================
static int connectSrv(double timeout = 3.0) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct timeval tv; tv.tv_sec = (int)timeout; tv.tv_usec = ((int)(timeout*1e6))%1000000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    int opt = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct linger lg = {1, 0}; setsockopt(fd, SOL_SOCKET, SO_LINGER, &lg, sizeof(lg));
    sockaddr_in addr = {}; addr.sin_family = AF_INET; addr.sin_port = htons(g_cfg.port);
    addr.sin_addr.s_addr = inet_addr(g_cfg.ip.c_str());
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return -1; }
    return fd;
}

static bool sendStr(int fd, const string& s) {
    size_t n = s.size(), sent = 0;
    while (sent < n) { ssize_t r = send(fd, s.c_str()+sent, n-sent, 0); if (r <= 0) return false; sent += r; }
    return true;
}

static string recvStr(int fd, double timeout = 5.0) {
    struct timeval tv; tv.tv_sec = (int)timeout; tv.tv_usec = ((int)(timeout*1e6))%1000000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    char buf[65536]; ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) return "";
    buf[n] = 0; return string(buf);
}

static string loginReq(int uid) {
    return "{\"msgid\":1,\"id\":"+to_string(uid)+",\"password\":\"123456\"}\0";
}
static string regReq(const string& name) {
    return "{\"msgid\":4,\"name\":\""+name+"\",\"password\":\"123456\"}\0";
}
static string chatReq(int from, int to, int seq) {
    return "{\"msgid\":6,\"id\":"+to_string(from)+",\"toid\":"+to_string(to)+",\"msg\":\"t"+to_string(seq)+"\"}\0";
}

static void addLat(double ms) { lock_guard<mutex> lk(g_lat_mutex); g_lats.push_back(ms); }
static void resetStats() { lock_guard<mutex> lk(g_lat_mutex); g_lats.clear(); }

static BenchResult calcResult(int64_t total, int64_t success, int64_t fail, double dur) {
    BenchResult r; r.total = total; r.success = success; r.fail = fail; r.duration_sec = dur;
    r.qps = success / max(dur, 0.001);
    lock_guard<mutex> lk(g_lat_mutex);
    if (!g_lats.empty()) {
        sort(g_lats.begin(), g_lats.end());
        double sum = 0; for (auto v : g_lats) sum += v;
        size_t n = g_lats.size();
        r.avg_ms = sum/n; r.min_ms = g_lats[0]; r.max_ms = g_lats[n-1];
        r.p50_ms = g_lats[(size_t)(n*0.50)]; r.p90_ms = g_lats[(size_t)(n*0.90)];
        r.p95_ms = g_lats[(size_t)(n*0.95)]; r.p99_ms = g_lats[(size_t)(n*0.99)];
    }
    return r;
}

static void printResult(const BenchResult& r, const string& title) {
    cout << fixed << setprecision(2);
    cout << "--- " << title << " ---" << endl;
    cout << "  请求数: " << r.total << "  成功: " << r.success << " (" << (r.success*100.0/max(r.total,(int64_t)1)) << "%)" << "  失败: " << r.fail << endl;
    cout << "  耗时: " << r.duration_sec << "秒  QPS: " << r.qps << endl;
    if (!g_lats.empty()) {
        cout << "  延迟(ms): avg=" << r.avg_ms << " min=" << r.min_ms << " p50=" << r.p50_ms << " p90=" << r.p90_ms << " p95=" << r.p95_ms << " p99=" << r.p99_ms << " max=" << r.max_ms << endl;
    }
    cout << endl;
}

static void section(const string& s) { cout << "\n" << string(60,'=') << "\n  " << s << "\n" << string(60,'=') << endl; }

// ===================== 测试场景 =====================

// 1. 登录压测
BenchResult benchLogin() {
    resetStats();
    atomic<int64_t> ok{0}, fail{0};
    int batch = g_cfg.users / g_cfg.concurrency;

    double t0 = now_ms();
    vector<thread> ths;
    for (int t = 0; t < g_cfg.concurrency; t++) {
        ths.emplace_back([&ok, &fail, t, batch]() {
            for (int i = 0; i < batch; i++) {
                int uid = 1 + t * batch + i;
                int fd = connectSrv(); if (fd < 0) { fail++; continue; }
                double ts = now_ms();
                if (!sendStr(fd, loginReq(uid))) { fail++; close(fd); continue; }
                string r = recvStr(fd);
                double te = now_ms();
                if (!r.empty() && r.find("\"errno\":0") != string::npos) {
                    ok++; addLat(te-ts);
                } else { fail++; }
                close(fd);
            }
        });
    }
    for (auto& th : ths) th.join();
    double dur = (now_ms()-t0)/1000.0;
    return calcResult(g_cfg.users, ok, fail, dur);
}

// 2. 长连接消息吞吐
BenchResult benchChat() {
    resetStats();
    atomic<int64_t> ok{0}, fail{0}, conn_ok{0}, conn_fail{0};
    int threads = min(g_cfg.concurrency, g_cfg.users);

    double t0 = now_ms();
    vector<thread> ths;
    for (int t = 0; t < threads; t++) {
        ths.emplace_back([&ok, &fail, &conn_ok, &conn_fail, t]() {
            int uid = 1 + t;
            int fd = connectSrv(); if (fd < 0) { conn_fail++; return; }
            conn_ok++;
            if (!sendStr(fd, loginReq(uid))) { fail += g_cfg.msgs; close(fd); return; }
            string r = recvStr(fd);
            if (r.empty() || r.find("\"errno\":0") == string::npos) { fail += g_cfg.msgs; close(fd); return; }
            ok++; // login counted

            for (int i = 0; i < g_cfg.msgs; i++) {
                int target = g_cfg.users + 1000 + i; // target offline user to avoid forwarding
                double ts = now_ms();
                if (!sendStr(fd, chatReq(uid, target, i))) { fail++; continue; }
                double te = now_ms();
                ok++; addLat(te-ts);
            }
            close(fd);
        });
    }
    for (auto& th : ths) th.join();
    double dur = (now_ms()-t0)/1000.0;
    int64_t total = threads * (1 + g_cfg.msgs);
    cout << "  连接: " << conn_ok << "/" << threads << " 成功" << endl;
    return calcResult(total, ok, fail, dur);
}

// 3. 注册压测
BenchResult benchRegister() {
    resetStats();
    atomic<int64_t> ok{0}, fail{0};
    int count = min(500, g_cfg.users/2);
    int threads = min(g_cfg.concurrency, count);
    int batch = count / threads;

    double t0 = now_ms();
    vector<thread> ths;
    for (int t = 0; t < threads; t++) {
        ths.emplace_back([&ok, &fail, t, batch]() {
            for (int i = 0; i < batch; i++) {
                int id = g_cfg.users + 20000 + t * batch + i;
                int fd = connectSrv(); if (fd < 0) { fail++; continue; }
                double ts = now_ms();
                if (!sendStr(fd, regReq("bn"+to_string(id)))) { fail++; close(fd); continue; }
                string r = recvStr(fd);
                double te = now_ms();
                if (!r.empty() && r.find("\"errno\":0") != string::npos) { ok++; addLat(te-ts); }
                else { fail++; }
                close(fd);
            }
        });
    }
    for (auto& th : ths) th.join();
    double dur = (now_ms()-t0)/1000.0;
    return calcResult(count, ok, fail, dur);
}

// 4. 持续压力
BenchResult benchSustain() {
    resetStats();
    atomic<bool> stop{false};
    atomic<int64_t> ok{0}, fail{0}, conn_ok{0}, conn_fail{0};
    int threads = min(g_cfg.concurrency, g_cfg.users);

    double t0 = now_ms();
    vector<thread> ths;
    for (int t = 0; t < threads; t++) {
        ths.emplace_back([&ok, &fail, &conn_ok, &conn_fail, &stop, t]() {
            int uid = 1 + t;
            int fd = connectSrv(5.0); if (fd < 0) { conn_fail++; return; }
            conn_ok++;
            if (!sendStr(fd, loginReq(uid))) { close(fd); return; }
            string r = recvStr(fd, 5.0);
            if (r.empty() || r.find("\"errno\":0") == string::npos) { close(fd); return; }

            int seq = 0;
            while (!stop) {
                int target = g_cfg.users + 10000 + seq;
                double ts = now_ms();
                if (!sendStr(fd, chatReq(uid, target, seq))) { fail++; break; }
                double te = now_ms();
                ok++; addLat(te-ts);
                seq++;
            }
            close(fd);
        });
    }

    this_thread::sleep_for(chrono::seconds(g_cfg.seconds));
    stop = true;
    for (auto& th : ths) th.join();
    double dur = (now_ms()-t0)/1000.0;
    cout << "  连接: " << conn_ok << "/" << threads << " 在线" << endl;
    return calcResult(ok+fail, ok, fail, dur);
}

// ===================== 主程序 =====================
int main(int argc, char* argv[]) {
    if (argc >= 3) { g_cfg.ip = argv[1]; g_cfg.port = atoi(argv[2]); }
    if (argc >= 4) g_cfg.users = atoi(argv[3]);
    if (argc >= 5) g_cfg.concurrency = atoi(argv[4]);
    if (argc >= 6) g_cfg.msgs = atoi(argv[5]);
    if (argc >= 7) g_cfg.seconds = atoi(argv[6]);

    cout << string(60,'=') << "\n  集群聊天服务器 - 压力测试 v3\n" << string(60,'=') << endl;
    cout << "  目标: " << g_cfg.ip << ":" << g_cfg.port << "  用户: " << g_cfg.users
         << "  并发: " << g_cfg.concurrency << "  消息: " << g_cfg.msgs << "  持续: " << g_cfg.seconds << "秒" << endl;

    // Test 1: Login
    section("测试1: 登录QPS");
    auto r1 = benchLogin();
    printResult(r1, "登录");

    // Test 2: Chat throughput  
    section("测试2: 消息吞吐量");
    auto r2 = benchChat();
    printResult(r2, "聊天消息");

    // Test 3: Register
    section("测试3: 注册压测");
    auto r3 = benchRegister();
    printResult(r3, "注册");

    // Test 4: Sustained load
    section("测试4: 持续" + to_string(g_cfg.seconds) + "秒压力");
    auto r4 = benchSustain();
    printResult(r4, "持续压力");

    section("测试完成");
    cout << "\n用法: ./stress_test [ip] [port] [users] [concurrency] [msgs] [seconds]\n" << endl;
    return 0;
}
