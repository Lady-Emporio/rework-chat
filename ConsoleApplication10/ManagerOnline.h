#pragma once

#include <windows.h>
#include "User.h"
#include <vector>
#include <string>

class authUserLate {
public:
	authUserLate() {}
	UserPtr olduser;
	int fd;
	std::string newName;
};

class sendLateMessage {
public:
	sendLateMessage(){}
	int fd;
	std::string message;
};

class ManagerOnline
{
	ManagerOnline();
	~ManagerOnline();
	std::vector<UserPtr>needDelUser;
	std::vector<authUserLate>needAuthUser;
	std::vector<sendLateMessage>needSendLate;

public:
	std::vector< UserPtr>users;
	static ManagerOnline * getManager();
	void fill_fdset(fd_set *x);
	void createOrUpdateUser(std::string name,int addingFd, bool isAuth);
	void deleteUser(UserPtr user);

	void closeFd(UserPtr user,int fd, bool isNeedClose);
	void deleteNow();
	void authLate(UserPtr olduser, int fd, std::string newName);
	void authNow();
	void sendLate(int fd, std::string message);
	void sendLateAllNow();

	bool sendUserByName(std::string name,std::string message);
	void callLateFunc();
};

