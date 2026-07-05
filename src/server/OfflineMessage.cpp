#include "OfflineMessage.h"
#include "database.h"

void OfflineMessage::store(int id, std::string msg) {
    char sql[1024] = { 0 };
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", 
            id, msg.c_str());
    
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

void OfflineMessage::remove(int id) {
    char sql[100] = { 0 };
    sprintf(sql, "delete from OfflineMessage where userid = %d", id);

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

std::vector<std::string> OfflineMessage::query(int id) {
    char sql[100] = { 0 };
    sprintf(sql, "select message from OfflineMessage where userid = %d", id);

    std::vector<std::string> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(row[0]);
            }

            mysql_free_result(res);
            return vec;
        }
    }

    return vec;
}