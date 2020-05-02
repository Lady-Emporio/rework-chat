#include "User.h"
#include "sky.h"
#include <string>

User::User()
{
	isAuth = false;
	updateTime();
}


User::~User()
{
	log("~User", "user: '"+name+"' is final delite.");
}

void User::addFD(int fd)
{
	log("User::addFD", "user: '"+name+"' add self fd: '"+std::to_string(fd)+"'.");
	fds.push_back(fd);
}

void User::deleteFd(int fd,bool isNeedClose)
{
	for (int i = 0; i != fds.size(); ++i) {
		int local_fd = fds[i];
		if (local_fd == fd) {
			
			if (isNeedClose) {
				closesocket(fd);
				log("User::deleteFd", "close and del fd: '" + std::to_string(fd) + "'. in user: '"+name+"'.");
			}
			else {
				log("User::deleteFd", "only del fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
			}
			fds.erase(fds.begin() + i);
			return;
		}
	}
	error("User::deleteFd", "try error but not found fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
}

void User::updateTime()
{
	_create = std::chrono::system_clock::now();
}

void User::fillFdset(fd_set * x)
{
	for (int i = 0; i != fds.size(); ++i) {
		FD_SET(fds[i], x);
	}
}

UserPtr User::create(std::string name, int fd)
{
	UserPtr u=std::make_shared<User>();
	(*u).name = name;
	(*u).addFD(fd);
	return u;
}

bool User::isDisconnect()
{
	if (fds.size() == 0) {
		return true;
	}
	return false;
}
