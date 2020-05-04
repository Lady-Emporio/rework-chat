#include "User.h"
#include "sky.h"
#include <string>
#include  <time.h>
#include <chrono>
#include <ctime>
#include <iomanip>

User::User()
{
	isAuth = false;
}


User::~User()
{
	log("~User", "user: '" + name + "' is final delite.");
}

void User::addFD(int fd)
{
	log("User::addFD", "user: '" + name + "' add self INCOME fd: '" + std::to_string(fd) + "'.");
	fds.push_back(fd);
}

void User::deleteFd(int fd, bool isNeedClose)
{
	for (int i = 0; i != fds.size(); ++i) {
		int local_fd = fds[i];
		if (local_fd == fd) {

			if (isNeedClose) {
				closesocket(fd);
				log("User::deleteFd", "close and del INCOME fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
			}
			else {
				log("User::deleteFd", "only del INCOME fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
			}
			fds.erase(fds.begin() + i);
			return;
		}
	}

	for (int i = 0; i != ComeFds.size(); ++i) {
		int local_fd = ComeFds[i];
		if (local_fd == fd) {

			if (isNeedClose) {
				closesocket(fd);
				log("User::deleteFd", "close and del COME fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
			}
			else {
				log("User::deleteFd", "only del COME fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
			}
			ComeFds.erase(ComeFds.begin() + i);
			return;
		}
	}
	error("User::deleteFd", "try error but not found fd: '" + std::to_string(fd) + "'. in user: '" + name + "'.");
}

void User::updateTime(int fd)
{
	for(int i=0;i!=fds.size();++i){
		if (fd == fds[i].fd) {
			fds[i]._create = std::chrono::system_clock::now();
			return;
		}
	}

	for (int i = 0; i != ComeFds.size(); ++i) {
		if (fd == ComeFds[i].fd) {
			ComeFds[i]._create = std::chrono::system_clock::now();
			return;
		}
	}
}

void User::fillFdset(fd_set * x)
{
	for (int i = 0; i != fds.size(); ++i) {
		FD_SET(fds[i], x);
	}
	for (int i = 0; i != ComeFds.size(); ++i) {
		FD_SET(ComeFds[i], x);
	}
}

UserPtr User::create(std::string name, int fd, bool isCome)
{
	UserPtr u = std::make_shared<User>();
	(*u).name = name;
	if (isCome) {
		u->addComeFD(fd);
	}
	else {
		(*u).addFD(fd);
	}
	
	return u;
}

bool User::isDisconnect()
{
	if (fds.size() == 0) {
		return true;
	}
	return false;
}

void User::addComeFD(int fd)
{
	log("User::addFD", "user: '" + name + "' add self COME fd: '" + std::to_string(fd) + "'.");
	ComeFds.push_back(fd);
}

FD::FD(int fd) :fd(fd)
{
	_create = std::chrono::system_clock::now();
}

std::string FD::getLastTimeStr()
{

	//std::time_t t = std::chrono::system_clock::to_time_t(_create);
	//std::string text = std::ctime_s(&t);
	//std::string text = std::put_time(std::localtime(&t));

	/*std::time_t t = std::chrono::system_clock::to_time_t(_create);
	std::string ts = std::ctime(&t);*/


	//Member	Type	Meaning	Range
	//tm_sec	int	seconds after the minute	0 - 60 *
	//tm_min	int	minutes after the hour	0 - 59
	//tm_hour	int	hours since midnight	0 - 23
	//tm_mday	int	day of the month	1 - 31
	//tm_mon	int	months since January	0 - 11
	//tm_year	int	years since 1900
	//tm_wday	int	days since Sunday	0 - 6
	//tm_yday	int	days since January 1	0 - 365

	struct tm tim;
	//time_t tt = time(NULL);
	time_t tt = std::chrono::system_clock::to_time_t(_create);
	localtime_s(&tim, &tt);

	int theDay = tim.tm_mday;
	int theMonth = tim.tm_mon;
	int theYear = tim.tm_year + 1900;

	int theHour = tim.tm_hour;
	int theMin = tim.tm_min;
	int theSec = tim.tm_sec;
	std::string text = std::to_string(theYear) + "." + std::to_string(theMonth) + "." + std::to_string(theDay) + " "+
		std::to_string(theHour) + ":" + std::to_string(theMin) + ":" + std::to_string(theSec);
	return text;
}
