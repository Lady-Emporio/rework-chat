#pragma once

#include <string>


	void log(std::string nameFunc,std::string message);
	void error(std::string nameFunc, std::string message);
	void fatal(std::string nameFunc, std::string message);

	

	// Служебные функции
	void __log(std::string nameFunc, std::string message);
	std::string rjust(std::string text, size_t width, char fillchar);
	std::string ljust(std::string text, size_t width, char fillchar);
