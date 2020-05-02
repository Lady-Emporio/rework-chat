#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include <windows.h>
#include <string>
#include "sky.h"

class User;
using UserPtr = std::shared_ptr<User>;

class FD {
public:
	FD(int fd);
	int fd;
	std::chrono::time_point<std::chrono::system_clock> _create;
	operator int() const
	{
		return fd;
	}
	std::string getLastTimeStr();
};

class User
{
public:
	std::vector<FD> fds;
	User();
	~User();
	bool isAuth;
	std::string name;
	std::string key;


	void addFD(int fd);
	void deleteFd(int fd, bool isNeedClose);
	void updateTime(int fd);
	void fillFdset(fd_set *x);
	static UserPtr create(std::string name, int fd);
	bool isDisconnect();
};

