#pragma once
#include <string>
#include "User.h"
void server_forever();

void initSockets();
int createSocket();
void bind(int s);
void listen(int s);


enum GetMessageStatus { isError, isOK, isCloseMessage};
GetMessageStatus getMessage(int fd, std::string *message);
bool isCanRead(int s);
void workWithMessage(std::string message, UserPtr user, int fd);
void answer(int fd, std::string message);
std::vector<std::string> parserMessage(std::string message);