#include "UserModel.h"

using std::string;

int userRegister(string& name, string& password) {
    // 组装 SQL 语句
    char sql[1024] = { 0 };
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', 'offline')",
        name.c_str(), password.c_str());

    // 执行 SQL 语句
    MySQL mysql;
    if (mysql.connect() && mysql.update(sql)) {
        return mysql_insert_id(mysql.getConnection());
    }
    else {
        return -1;
    }
}

bool userUpdateState(int id, bool s) {
    char sql[100] = { 0 };
    if (s) {
        sprintf(sql, "update User set state = 'online' where id = %d", id);
    }
    else {
        sprintf(sql, "update User set state = 'offline' where id = %d", id);
    }

    MySQL mysql;
    if (mysql.connect() && mysql.update(sql)) {
        return true;
    }
    return false;
}

void userResetState() {
    char sql[] = "update User set state = 'offline' where state = 'online'";
 
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}