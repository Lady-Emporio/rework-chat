#pragma once

#include <windows.h>
#include "User.h"
#include <vector>
#include <string>
class ManagerOnline
{
	ManagerOnline();
	~ManagerOnline();
	std::vector<UserPtr>needDelUser;
public:
	std::vector< UserPtr>users;
	static ManagerOnline * getManager();
	void fill_fdset(fd_set *x);
	void createOrUpdateUser(std::string name,int addingFd);
	void deleteUser(UserPtr user);

	void closeFd(UserPtr user,int fd);
	void makeLateDeleteNow();
};

