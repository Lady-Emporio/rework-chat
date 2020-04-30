#include "ManagerOnline.h"
#include "sky.h"


ManagerOnline::ManagerOnline()
{
}


ManagerOnline::~ManagerOnline()
{
}

ManagerOnline * ManagerOnline::getManager()
{
	static ManagerOnline * w = new ManagerOnline();
	return w;
}

void ManagerOnline::fill_fdset(fd_set * x)
{
	for (int i = 0; i != users.size(); ++i) {
		(*users[i]).fillFdset(x);
	}
}

void ManagerOnline::createOrUpdateUser(std::string name, int addingFd)
{
	UserPtr u = nullptr;
	bool isFound = false;
	for (int i = 0; i != users.size(); ++i) {
		u = users[i];
		if ((*u).name == name) {
			(*u).addFD(addingFd);
			isFound = true;
		}
	}

	if (!isFound) {
		u=User::create(name, addingFd);
	}

	users.push_back(u);
	
}

void ManagerOnline::deleteUser(UserPtr user)
{
	for (int i = 0; i != users.size(); ++i) {
		UserPtr l_user = users[i];
		if (l_user == user) {//Если объект в одной и той же памяти
			log("ManagerOnline::deleteUser", "found and del user:"+ (*user).name);
			users.erase(users.begin() + i);
			return;
		}
	}
	error("ManagerOnline::deleteUser", "call del user but not found him:" + (*user).name);
}

void ManagerOnline::closeFd(UserPtr user, int fd)
{
	user->deleteFd(fd);
	if (user->isDisconnect()) {
		needDelUser.push_back(user);
	}
}

void ManagerOnline::makeLateDeleteNow()
{
	while (!needDelUser.empty()) {
		UserPtr user = needDelUser[0];
		deleteUser(user);
		needDelUser.erase(needDelUser.begin());
	}
}
