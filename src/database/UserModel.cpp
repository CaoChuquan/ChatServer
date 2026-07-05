#include "UserModel.h"

bool UserUpdateState(int id, bool s) {
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