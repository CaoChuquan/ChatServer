#include "usermodel.hpp"
#include "db.h"
#include <iostream>
#include <mutex>
using namespace std;

MySQL g_mysql;
once_flag g_mysql_init_flag;
mutex g_db_mutex;

static void ensureMySQL() {
    call_once(g_mysql_init_flag, []() {
        g_mysql.connect();
    });
}

bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    lock_guard<mutex> lock(g_db_mutex);
    ensureMySQL();
    if (g_mysql.update(sql))
    {
        user.setId(mysql_insert_id(g_mysql.getConnection()));
        return true;
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d", id);

    lock_guard<mutex> lock(g_db_mutex);
    ensureMySQL();
    MYSQL_RES *res = g_mysql.query(sql);
    if (res != nullptr)
    {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setPwd(row[2]);
            user.setState(row[3]);
            mysql_free_result(res);
            return user;
        }
        mysql_free_result(res);
    }
    return User();
}

bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    lock_guard<mutex> lock(g_db_mutex);
    ensureMySQL();
    return g_mysql.update(sql);
}

void UserModel::resetState()
{
    char sql[1024] = "update User set state = 'offline' where state = 'online'";

    lock_guard<mutex> lock(g_db_mutex);
    ensureMySQL();
    g_mysql.update(sql);
}