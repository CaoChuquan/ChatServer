#pragma once

#include <string>

class User {
public:
    User(int id = -1, std::string n = "", 
        std::string pw = "", bool stat = false)
        : id_(id), name_(n), password_(pw), state_(stat)
    { }

    void setId(int id) { id_ = id; }
    void setName(std::string& name) { name_ = name; }
    void setPassword(std::string& pw) { password_ = pw; }
    
    void changeState() {  state_ = !state_; }

    int getId() { return id_; }
    std::string getName() { return name_; }
    std::string getPassword() { return password_; }
    bool getState() { return state_; }
private:
    int id_;
    std::string name_;
    std::string password_;
    bool state_;
};