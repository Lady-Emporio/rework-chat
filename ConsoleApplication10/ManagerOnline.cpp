#include "ManagerOnline.h"
#include "sky.h"
#include "server.h"

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

UserPtr ManagerOnline::createOrUpdateUser(std::string name, int addingFd,bool isAuth, bool isComeFd)
{
	UserPtr u = nullptr;
	bool isFound = false;
	for (int i = 0; i != users.size(); ++i) {
		u = users[i];
		if ((*u).name == name) {
			if (isComeFd) {
				u->addComeFD(addingFd);
			}
			else {
				(*u).addFD(addingFd);
			}
			
			if (isAuth) {
				(*u).isAuth = true;
			}
			isFound = true;
		}
	}

	if (!isFound) {
		log("MO::createOrUpdateUser", "Create user: '"+ name +"', with fd: '"+std::to_string(addingFd) +"'.");
		u=User::create(name, addingFd, isComeFd);
		(*u).isAuth = isAuth;
		users.push_back(u);
	}

	return u;
	
}

void ManagerOnline::deleteUser(UserPtr user)
{
	for (int i = 0; i != users.size(); ++i) {
		UserPtr l_user = users[i];
		if (l_user == user) {//Если объект в одной и той же памяти
			log("MO::deleteUser", "found and del user: '"+ (*user).name+"'.");
			users.erase(users.begin() + i);
			return;
		}
	}
	error("MO::deleteUser", "call del user but not found him: '" + (*user).name + "'.");
}

void ManagerOnline::closeFd(UserPtr user, int fd, bool isNeedClose)
{
	user->deleteFd(fd, isNeedClose);
	if (user->isDisconnect()) {
		log("MO::closeFd", "User not have fd. Mark to del user:" + (*user).name);
		needDelUser.push_back(user);
	}
}

void ManagerOnline::deleteNow()
{
	int logMaxDel = needDelUser.size();
	int logdelCount = 0;
	while (!needDelUser.empty()) {
		UserPtr user = needDelUser[0];
		if ((*user).isDisconnect()) {
			deleteUser(user);
			++logdelCount;
		}
		needDelUser.erase(needDelUser.begin());
	}
	log("MO::deleteNow", "Check: '" + std::to_string(logMaxDel) + "'. Del: '" + std::to_string(logdelCount) + "' . Not del: '" + std::to_string(logMaxDel- logdelCount) + "'.");
}

void ManagerOnline::authLate(UserPtr olduser, int fd, std::string newName,bool isCome)
{
	authUserLate usLate;
	usLate.olduser = olduser;
	usLate.fd = fd;
	usLate.newName = newName;
	usLate.isCome = isCome;
	needAuthUser.push_back(usLate);
}

void ManagerOnline::authNow()
{
	while (!needAuthUser.empty()) {
		authUserLate u= needAuthUser[0];
		needAuthUser.erase(needAuthUser.begin());
		closeFd(u.olduser, u.fd, false);
		UserPtr new_user=createOrUpdateUser(u.newName,u.fd,true,u.isCome);
		sendLate(new_user,u.fd, "Wellcome"+ u.newName);
	}
}

void ManagerOnline::sendLate(UserPtr user,int fd, std::string message)
{
	sendLateMessage u;
	u.fd = fd;
	u.message = message;
	u.user = user;
	needSendLate.push_back(u);
}

void ManagerOnline::sendLateAllNow()
{

	while (!needSendLate.empty()) {
		sendLateMessage u = needSendLate[0];
		needSendLate.erase(needSendLate.begin());
		answer(u.user,u.fd, u.message);
	}
}

bool ManagerOnline::sendUserByName(std::string name, std::string message)
{
	bool isSend = false;
	for (int i = 0; i != users.size(); ++i) {
		UserPtr user = users[i];
		if ((*user).name == name) {
			for (int i = 0; i != (*user).fds.size(); ++i) {
				int fd = (*user).fds[i];
				answer(user,fd, message);
				isSend = true;
			}
		}
	}
	return isSend;
}

void ManagerOnline::callLateFunc()
{
	deleteNow();
	authNow();
	sendLateAllNow();// always must be in end;
}

void ManagerOnline::selectError()
{
	bool isSomethingFound;
	for (UserPtr user : users) {
		for (int fd : user->fds) {
			if (SOCKET_ERROR == fd) {
				isSomethingFound = true;
				log("MO::selectError", "found error socket in user: '"+user->name+"'. fd: "+std::to_string(fd)+"'.");
				closeFd(user, fd, true);
			}
		}
	}
	if (!isSomethingFound) {
		error("MO::selectError", "selectError nothing not found. All must work.");
	}
}
