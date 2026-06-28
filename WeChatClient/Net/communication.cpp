#include "communication.h"

Communication::Communication(QObject *parent)
	: QObject(parent)
{
	socket = new QTcpSocket(this);
	//先默认只连接一次

	connect(socket, &QTcpSocket::connected, this, [=]()
	{
			qDebug() << "连接成功!\n";
			success = true;
	});
	connect(socket, &QTcpSocket::errorOccurred, this, [=]()
	{
			qDebug() << "连接失败\n";
			success = false;
	});
	connect(socket, &QTcpSocket::readyRead, this, &Communication::dealRead);

	socket->connectToHost(SERVEIP, SERVEPORT);

}

Communication::~Communication()
{
	delete socket;
}

void Communication::dealRead()
{
	std::string content = static_cast<std::string>(socket->readAll());
	//std::ofstream ofs("log.txt",std::ios::out);
	//ofs << "写入数据:";
	//ofs << content.size() << '\n';
	protocol.solveMsg(content);
	std::string msg;
	while (true)
	{
		msg = protocol.getMsg();
		if (msg == "")
		{
			break;
		}
		//std::ofstream ofs("log.txt",std::ios::out);
		//ofs << "写入数据:";
		//ofs << msg.size() << '\n';
		//传递消息
		emit readMsg(msg);
	}
}

void Communication::dealWrite(std::string content)
{
	if (!success)
	{
		return;
	}
	packMsg(content);
	socket->write(content.data(),content.size());
}

bool Communication::getState()
{
	return success;
}