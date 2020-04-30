#include "User.h"
#include "sky.h"


User::User()
{
	isAuth = false;
	updateTime();
}


User::~User()
{
}

void User::addFD(int fd)
{
	fds.push_back(fd);
}

void User::deleteFd(int fd)
{
	for (int i = 0; i != fds.size(); ++i) {
		int local_fd = fds[i];
		if (local_fd == fd) {
			log("User::deleteFd", "close and del fd:'" + std::to_string(fd) + "'.");
			fds.erase(fds.begin() + i);
			closesocket(fd);
			return;
		}
	}
	error("User::deleteFd", "try error but not found fd");
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
