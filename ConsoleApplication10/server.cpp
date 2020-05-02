#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Winsock2.h> 
#include <ws2tcpip.h> 

#include "server.h"
#include "ManagerOnline.h"
#include <windows.h>
#include "sky.h"
#include <vector>
#include <sstream>
#include "json.hpp"
#include <thread>
#include <mutex>

inline std::mutex manager_mutex;

using json = nlohmann::json;

const std::string ANSWER_UNKNOW = "unknow";
const std::string ANSWER_NOT_AUTH = "not auth";
const std::string ANSWER_JSON_PARSE_ERROR = "ANSWER_JSON_PARSE_ERROR";
const std::string ANSWER_OK = "TWILIGHT";
const std::string ANSWER_NAME_NOT_FOUND = "ANSWER_NAME_NOT_FOUND";
const std::string ANSWER_VALUE_NOT_FOUND = "ANSWER_VALUE_NOT_FOUND";


void server_forever()
{
	initSockets();
	int server = createSocket();
	bind(server);
	listen(server);
	

	while (true) {
		///////////////////////////////////////////////////////////
		/////////////////// Start eternal while ///////////////////
		///////////////////////////////////////////////////////////
		{
			//std::lock_guard lock(manager_mutex);
			next_pass_while(server);
		}
		
		
		//////////////////////////////////////////////////////////
		/////////////////// End eternal while ////////////////////
		//////////////////////////////////////////////////////////
	}
}


void initSockets()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		fatal("initSocket", "WSAStartup falled");
	}
}

int createSocket()
{
	SOCKET _s;
	_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_s == INVALID_SOCKET)
	{
		fatal("createSocket","Error create socket.");
	}
	else {
		log("createSocket","Good create socket.");
	}
	return _s;
}

void bind(int s)
{
	int port = 5555;
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_port = htons(port);

	inet_pton(AF_INET, "127.0.0.1", &(service.sin_addr));

	int iResult = bind(s, (SOCKADDR *)&service, sizeof(service));
	if (iResult == SOCKET_ERROR) {
		fatal("bind","Error with bind server.");
	}
	else {
		log("bind","Good bind on: '" + std::to_string(port) + "'.");
	}
}

void listen(int s)
{
	if (listen(s, 60) == SOCKET_ERROR) {
		fatal("listen","listen error.");
	}
}


GetMessageStatus getMessage(int fd, std::string * message)
{
	if (!isCanRead(fd)) {
		error("getMessage", "call func with empty socket.");
		return GetMessageStatus::isError;
	}
	*message = "";
	while (isCanRead(fd)) {
		char buf[400 * 8 * 2];
		int byte_count;
		byte_count = recv(fd, buf, sizeof(buf), 0);
		if (0 == byte_count) {
			log("getMessage", "Get message about close: '" + std::to_string(fd) + "'.");
			return GetMessageStatus::isCloseMessage;
		}
		else if (SOCKET_ERROR == byte_count) {
			error("getMessage","recv error:" + std::to_string(WSAGetLastError()));
			return GetMessageStatus::isError;
		}
		buf[byte_count] = '\0';
		*message += buf;
	}
	log("getMessage", "get:"+(*message));
	return GetMessageStatus::isOK;
}

bool isCanRead(int s)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(s, &readfds);
	int result = select(
		NULL,
		&readfds,
		NULL,
		NULL,
		&tv
	);
	if (0 == result) {
		return false;
	}
	return true;
}

void workWithMessage(std::string message, UserPtr user, int fd)
{
	json json_message;
	try {
		json_message = json::parse(message);
	}
	catch (...) {
		answer(fd, ANSWER_JSON_PARSE_ERROR);
		return;
	}


	std::string jsonName = json_message.value("name", "-1");
	if (jsonName == "-1") {
		answer(fd, ANSWER_NAME_NOT_FOUND);
		return;
	}
	std::string jsonValue = json_message.value("value", "-1");
	if (jsonValue == "-1") {
		answer(fd, ANSWER_VALUE_NOT_FOUND);
		return;
	}

	if (!(*user).isAuth && jsonName != "auth") {
		answer(fd, ANSWER_NOT_AUTH);
		return;
	}

	if ("auth" == jsonName) {
		ManagerOnline::getManager()->authLate(user,  fd, jsonValue);
		return;
	}
	
	if ("message" == jsonName) {
		std::string who = jsonValue;
		std::string alertAboutNewData = "data";
		ManagerOnline::getManager()->sendUserByName(who, alertAboutNewData);
		return;
	}
	if ("ping" == jsonName) {
		answer(fd, "pong");
		return;
	}
	answer(fd, ANSWER_UNKNOW);
}
//Заголовок фиксированной длины
//Данные

//Оповещение фиксированной длины
void answer(int fd, std::string message)
{
	sendall(fd, message);
}

bool sendall(int fd, std::string message)
{
	const char * buf = message.data();
	int len = message.size();
	int total = 0;           // сколько байт мы послали 
	int bytesleft = len;	//// сколько байт осталось послать 
	int n=-1;
	
	while (total < len) {
		n = send(fd, buf + total, bytesleft, 0);
		if (n == -1) { 
			//+ handling error. /////////////////////////////
			if (0 != total) {
				error("sendall", "Can not send message. !!!!!! ALREADY SEND something!!!!!!. SEND:'"+std::to_string(total)+"' bytes.");
			}
			else {
				error("sendall", "Can not send message. Nothing not send. Zero send.");
			}
			break; 
			//- handling error. /////////////////////////////
		}
		total += n;
		bytesleft -= n;
	}

	return (0 == n);
}

bool sendall(int fd, char * buf, int len)
{
	int total = 0;           // сколько байт мы послали 
	int bytesleft = len;	//// сколько байт осталось послать 
	int n = -1;

	while (total < len) {
		n = send(fd, buf + total, bytesleft, 0);
		if (n == -1) {
			//+ handling error. /////////////////////////////
			if (0 != total) {
				error("sendall", "Can not send message. !!!!!! ALREADY SEND something!!!!!!. SEND:'" + std::to_string(total) + "' bytes.");
			}
			else {
				error("sendall", "Can not send message. Nothing not send. Zero send.");
			}
			break;
			//- handling error. /////////////////////////////
		}
		total += n;
		bytesleft -= n;
	}

	return (0 == n);
}

void run_daemon_server()
{
	std::thread thr(server_forever);
	thr.detach();
}

void next_pass_while(int server )
{
	ManagerOnline * manager = ManagerOnline::getManager();

	fd_set readfds;
	FD_ZERO(&readfds);
	manager->fill_fdset(&readfds);
	FD_SET(server, &readfds);


	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 500000;

	int result = select(NULL, &readfds, NULL, NULL, &tv);
	if (0 == result) {
		log("server_forever", "timeout select.");
		manager->callLateFunc();
		return;
	}
	else if (SOCKET_ERROR == result) {
		fatal("server_forever", "select error." + std::to_string(WSAGetLastError()));
		return;
	}
	// NEW CONNECTION ######################################
	if (FD_ISSET(server, &readfds)) {
		struct sockaddr_storage their_addr;
		socklen_t addr_size;
		addr_size = sizeof their_addr;
		int new_fd = accept(server, (struct sockaddr *)&their_addr, &addr_size);
		log("server_forever", "new connection on:" + std::to_string(new_fd));

		manager->createOrUpdateUser("unknow guest: " + std::to_string(new_fd), new_fd, false);
	}
	//COME MESSAGE ######################################

	for (UserPtr user : manager->users) {
		for (int fd : user->fds) {
			if (FD_ISSET(fd, &readfds)) {
				std::string message;
				GetMessageStatus status = getMessage(fd, &message);
				switch (status)
				{
				case isError:
					log("server_forever", "!!!!!!!!! something wrong with socket. Close it.");
					manager->closeFd(user, fd, true);
					break;
				case isOK:
					workWithMessage(message, user, fd);
					break;
				case isCloseMessage:
					manager->closeFd(user, fd, true);
					break;
				default:
					break;
				}
			}
		}
	}

	manager->callLateFunc();
}
