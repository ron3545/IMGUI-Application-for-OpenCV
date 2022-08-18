#pragma once
#include <string>

#define EMPTY_NOW true //empty the strings on a struct named UserData

//used only for loginform function
struct UserData
{
	std::string username, password;
	
	//defaults
	UserData() {}
	UserData(std::string server_, std::string username_, std::string password_)
		: username(username_), password(password_) {}

	void operator<<(bool flag) {
		if (flag == true) {
			username.clear();
			password.clear();
		}
	}

	bool is_empty() {
		if (username.length() == 0 || password.length() == 0)
			return true;
		return false;
	}
	
};
