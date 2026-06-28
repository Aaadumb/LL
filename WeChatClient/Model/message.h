#pragma once
#include <QString>
enum MsgType {
	None = 0,
	Send = 1,
	Reveive = 2,
};
class Message {
public:
	Message() = default;

	Message(QString t, QString infor,MsgType tp,long long id_) :time(t), information(infor) ,type(tp),id(id_){};

	~Message() = default;

	QString getTime() { return time; }
	void setTime(QString t) { time = t; }

	QString getInfor() { return information; }
	void setInfor(QString infor) { information = infor; }

	void setType(MsgType type_) { type = type_; }
	MsgType getType() { return type; }

	void setId(long long id_) { id = id_; }
	long long getId() { return id; }
private:
	QString time;
	
	QString information;

	MsgType type;

	long long id;
	
};