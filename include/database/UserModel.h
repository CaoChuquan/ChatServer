#pragma once

#include "database.h"
#include <string>

int userRegister(std::string&, std::string&);
bool userUpdateState(int id, bool stat);