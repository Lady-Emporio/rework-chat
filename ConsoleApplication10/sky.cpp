#include "sky.h"
#include <sstream>
#include <windows.h>
#include <iostream>

std::string rjust(std::string text, size_t width, char fillchar = '0') {
	if (text.size() >= width) {
		return text;
	}
	std::string returnString(width - text.size(), fillchar);
	returnString += text;
	return returnString;
}


std::string ljust(std::string text, size_t width, char fillchar = '0') {
	if (text.size() >= width) {
		return text;
	}
	std::string addString(width - text.size(), fillchar);
	text+= addString;
	return text;
}

void log(std::string nameFunc, std::string message)
{
	__log(nameFunc,"     " + message);
}

void error(std::string nameFunc, std::string message)
{
	__log(nameFunc,"ERROR: "+ message);
}

void fatal(std::string nameFunc, std::string message)
{
	__log(nameFunc, "!!!!!!!!!! FATAL !!!!!!!!!!" + message);
}



void __log(std::string nameFunc, std::string message)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	std::stringstream buf;
	buf << st.wYear << "." <<
		rjust(std::to_string(st.wMonth), 2) << "." <<
		rjust(std::to_string(st.wDay), 2) << " " <<
		rjust(std::to_string(st.wHour), 2) << ":" <<
		rjust(std::to_string(st.wMinute), 2) << ":" <<
		rjust(std::to_string(st.wSecond), 2) << "_" <<
		rjust(std::to_string(st.wMilliseconds), 3);
	auto now = buf.str();

	std::cout << now << " | in: '" << ljust(nameFunc,20,' ') <<"' : '"<< message << "'. "<<std::endl;
}

