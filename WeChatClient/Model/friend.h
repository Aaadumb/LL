#pragma once
#include "message.h"
#include <deque>
class Friend {
public:
	Friend() = default;

	~Friend() = default;

	Friend(int id_, QString name_, QString avatar_, int count_=0,std::deque<Message> msg_ = std::deque<Message>()) :
		id(id_),
		name(name_),
		avatar(avatar_),
		msg(msg_),
		count(count_){};

	void setId(int id_) { id = id_; }
	int getId() { return id; }

	void setName(QString name_) { name = name_; }
	QString getName() { return name; }

	void setAvatar(QString avatar_) { avatar = avatar_; }
	QString getAvatar() { return avatar; }

	void setMsgVec(std::deque<Message>msg_) { msg = msg_; }
	
	bool haveMsg() { return !msg.empty(); }
	std::deque<Message> getAllMsg() { return msg; }
	Message getLastestMeg() { return msg.back(); }
	Message getOldestMsg() { return msg.front(); }

	void tailAddMsg(Message message) { msg.push_back(message); }
	void headAddMsg(Message message) { msg.push_front(message); }
	
	void setCount(int count_) { count = count_; }
	int getCount() { return count; }
	void addCount(int cnt) { count += cnt; }

	bool getIsAll() { return isAll; }
	void setIsAll(bool isAll_) { isAll = isAll_; }
private:
	std::deque<Message> msg;
	
	int id;
	
	QString name;

	QString avatar;

	int count=0;	//记录没有读的消息 也就是最新的消息

	bool isAll = false;//判断是否拿到了全部的消息
	
};
