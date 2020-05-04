#pragma once

#include <windows.h>
#include "User.h"
#include <vector>
#include <string>

class authUserLate {
public:
	authUserLate():isCome(false){}
	UserPtr olduser;
	int fd;
	std::string newName;
	bool isCome;
};

class sendLateMessage {
public:
	sendLateMessage(){}
	int fd;
	std::string message;
	UserPtr user;
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
	UserPtr createOrUpdateUser(std::string name,int addingFd, bool isAuth,bool isComeFd);
	void deleteUser(UserPtr user);

	void closeFd(UserPtr user,int fd, bool isNeedClose);
	void deleteNow();
	void authLate(UserPtr olduser, int fd, std::string newName,bool isCome);
	void authNow();
	void sendLate(UserPtr user,int fd, std::string message);
	void sendLateAllNow();

	bool sendUserByName(std::string name,std::string message);
	void callLateFunc();

	void selectError();
};

