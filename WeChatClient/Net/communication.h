#pragma once

#include <QObject>
#include <QTcpSocket>
#include "protocolparsing.h"
// 处理和服务器的连接 同时处理上层发送来的消息 交给下层去处理 这里为了分工明确 
// 连接类只处理接发消息 解决粘包 传给下方
#define SERVEIP "127.0.0.1"
#define SERVEPORT 9527
class Communication  : public QObject
{
	Q_OBJECT

public:
	Communication(QObject *parent);
	~Communication();

	bool getState();

	void dealWrite(std::string content);//处理写事件

private:
	ProtocolParsing protocol;			// 粘包处理器

	QTcpSocket* socket = nullptr;

	bool success = false;				//判断是否连接成功

	void dealRead();					//处理读事件

signals:
	void readMsg(std::string content);
};

