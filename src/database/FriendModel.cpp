#include "FriendModel.h"
#include "database.h"

void friendInsert(int user_id, int friend_id) {
    char sql[100] = { 0 };
    sprintf(sql, "insert into Friend values(%d, %d)", user_id, friend_id);


    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }

}

std::vector<User> friendQuery(int user_id) {
    char sql[100] = { 0 };
    sprintf(sql, 
        "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid=%d",
        user_id);

    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user {
                    atoi(row[0]),
                    row[1],
                    "",
                    row[2]
                };
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    
    return vec;
}