#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include <windows.h>
class User;
using UserPtr = std::shared_ptr<User>;

class User
{
	
	std::chrono::time_point<std::chrono::system_clock> _create;
public:
	std::vector<int> fds;
	User();
	~User();
	bool isAuth;
	std::string name;
	std::string key;


	void addFD(int fd);
	void deleteFd(int fd, bool isNeedClose);
	void updateTime();
	void fillFdset(fd_set *x);
	static UserPtr create(std::string name, int fd);
	bool isDisconnect();
};

