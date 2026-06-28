#pragma once

#include <QWidget>
#include "View/receivewidget.h"
#include "View/senderwidget.h"
#include "ui_chatwidget.h"
#include "../Model/friend.h"
#include <qtimezone.h>
#include <QScrollBar>
namespace Ui { class ChatWidget; }
class ChatWidget  : public QWidget
{
	Q_OBJECT

public:
	ChatWidget(QString avartar,QWidget*parent=nullptr);
	~ChatWidget();
	void setChatContent(Friend* data);
	
	void updateAvatar(QString avatar);

	//用于中途朋友消息来临 或者请求更早消息时的微调操作
	void headAddText(Message s);
	void tailAddText(Message s);
	
	Friend* getFriend();

private:
	Ui::ChatWidget* ui;
	
	void on_sendBtn_clicked();
	
	void onScrollBarChange();

	void dealText(Message &s, int opt);

	Friend* m_data = nullptr;

	QString m_avatar;

signals:
	void SendFriendMsg(int id,Message content);							//向外部说明 要更新朋友的消息列表了
	
	void moreMsgRq(int friendid,long long id=LLONG_MAX);							//向外部申请更多的历史消息 xxx没有最新的时间

};

