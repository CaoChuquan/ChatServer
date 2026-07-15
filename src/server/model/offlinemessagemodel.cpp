#include "offlinemessagemodel.hpp"
#include "db.h"
#include <mutex>
using namespace std;

extern mutex g_db_mutex;
extern MySQL g_mysql;

void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());

    lock_guard<mutex> lock(g_db_mutex);
    g_mysql.update(sql);
}

void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    lock_guard<mutex> lock(g_db_mutex);
    g_mysql.update(sql);
}

vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);

    vector<string> vec;
    lock_guard<mutex> lock(g_db_mutex);
    MYSQL_RES *res = g_mysql.query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr)
        {
            vec.push_back(row[0]);
        }
        mysql_free_result(res);
    }
    return vec;
}
