#include "friendmodel.hpp"
#include "db.h"
#include <mutex>
using namespace std;

extern mutex g_db_mutex;
extern MySQL g_mysql;

void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d, %d)", userid, friendid);

    lock_guard<mutex> lock(g_db_mutex);
    g_mysql.update(sql);
}

vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid=%d", userid);

    vector<User> vec;
    lock_guard<mutex> lock(g_db_mutex);
    MYSQL_RES *res = g_mysql.query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res)) != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            vec.push_back(user);
        }
        mysql_free_result(res);
    }
    return vec;
}
