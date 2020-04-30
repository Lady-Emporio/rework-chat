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

const std::string ANSWER_UNKNOW = "unknow";
void server_forever()
{
	initSockets();
	int server = createSocket();
	bind(server);
	listen(server);
	ManagerOnline * manager = ManagerOnline::getManager();

	while (true) {
		///////////////////////////////////////////////////////////
		/////////////////// Start eternal while ///////////////////
		///////////////////////////////////////////////////////////

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
			continue;
		}
		else if (SOCKET_ERROR == result) {
			fatal("server_forever", "select error." + std::to_string(WSAGetLastError()));
			continue;
		}
		// NEW CONNECTION ######################################
		if (FD_ISSET(server, &readfds)) {
			struct sockaddr_storage their_addr;
			socklen_t addr_size;
			addr_size = sizeof their_addr;
			int new_fd = accept(server, (struct sockaddr *)&their_addr, &addr_size);
			log("server_forever", "new connection on:" + std::to_string(new_fd));

			manager->createOrUpdateUser("unknow guest: " + std::to_string(new_fd), new_fd);
		}
		//COME MESSAGE ######################################
		
		for (UserPtr user : manager->users) {
			for (int fd : user->fds) {
				if (FD_ISSET(fd, &readfds)) {
					std::string message;
					GetMessageStatus status= getMessage(fd, &message);
					switch (status)
					{
					case isError:
						log("server_forever", "!!!!!!!!! something wrong with socket. Close it.");
						manager->closeFd(user,fd);
						break;
					case isOK:
						workWithMessage(message, user,fd);
						break;
					case isCloseMessage:
						manager->closeFd(user, fd);
						break;
					default:
						break;
					}
				}
			}
		}
		manager->makeLateDeleteNow();
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

std::vector<std::string> parserMessage(std::string message) {
	std::vector<std::string> messagesList;
	if (message.size() < 3) {
		return messagesList;
	}
	int pos = 1;
	for (int i = 1; i != message.size(); ++i) {
		char now = message[i];
		if (now == '|') {
			int addToNewStringLenChar = i - pos;
			if (addToNewStringLenChar != 0) {
				std::string lastMessage(message, pos, addToNewStringLenChar);
				messagesList.push_back(lastMessage);
			}
			pos = i + 1;
		}
	}
	return messagesList;
}
void workWithMessage(std::string message, UserPtr user, int fd)
{
	
	//split separator |
	// text can not have |
	/*
	"|text|"   = text
	"|text1|text2|"   = [text1,text2]
	"text1|text2"   = unknow
	"text1|"   = unknow
	"|text2"   = unknow
	*/

	/*
	"|4444|"=[4444]
	"|4||5|"=[4,5]
	"|4|5|7|"=[4,5,7]
	"||333||"=333
	"||4||"=[4]
	"|1||"=[1]
	"||||"=[]
	"||"=[]
	"|||"=[]
	*/
	if (message.size() < 2) {
		error("workWithMessage", "ANSWER_UNKNOW on:"+ message);
		answer(fd, ANSWER_UNKNOW);
		return;
	}
	if ('|' != message[0]) {
		error("workWithMessage", "ANSWER_UNKNOW on:" + message);
		answer(fd, ANSWER_UNKNOW);
		return;
	}

	if ('|' != message[message.size() - 1]) {
		error("workWithMessage", "ANSWER_UNKNOW on:" + message);
		answer(fd, ANSWER_UNKNOW);
		return;
	}
	std::vector<std::string> messagesList = parserMessage(message);

	{//+Delail log
		bool isDetail = true;
		if (isDetail) {
			log("workWithMessage", "Come: '" +std::to_string(messagesList.size())+ "' messages:.");
			for (int i = 0; i != messagesList.size(); ++i) {
				log("workWithMessage", "Message: '" + std::to_string(i) + "' :"+ messagesList[i]);
			}
		}
	}// -Delail log



}

void answer(int fd, std::string message)
{
}
