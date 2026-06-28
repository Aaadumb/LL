#pragma once
// 主要用于处理粘包和半包
#include <string>
#include <queue>
#define HEADLEN 20

void packMsg(std::string& msg);

class ProtocolParsing
{
public:
	void solveMsg(std::string& s);
	std::string getMsg();
private:
	std::queue<std::string> msgs;
	
	std::string headString = "";
	
	int len = -1;

	int p = 0;

	std::string bodyString = "";

	void spliceHead(std::string& s);

	void spliceBody(std::string& s);
};

