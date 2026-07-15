#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <mutex>
using namespace std;

class MySQL
{
public:
    MySQL();
    ~MySQL();
    bool connect();
    bool update(string sql);
    MYSQL_RES *query(string sql);
    MYSQL* getConnection();
    void lock();
    void unlock();
private:
    MYSQL *_conn;
    static mutex _mutex;
};

#endif