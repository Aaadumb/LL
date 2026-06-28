#pragma once

#include <QWidget>
#include "ui_chatlistwidget.h"
#include "chatlistitem.h"
#include <unordered_map>
namespace Ui { class ChatListWidget; }
class ChatListWidget  : public QWidget
{
	Q_OBJECT

public:
	ChatListWidget(QWidget *parent=nullptr);
	~ChatListWidget();
	ChatListWidget() = default;

	void addItem(Friend* data,QString lastMsg = "", QString lastTime = "");
	void updateItem(int friendid);

	bool haveItemSelected();
private:
	Ui::ChatListWidget* ui;
	QListWidgetItem* selectedItem = nullptr;			// 记录当前被选中的friend
	std::unordered_map<int, QListWidgetItem*> items;	// 记录所有的item 方便后续更新item
	
	void dealToTopRq(int id);							// 将某一item设置到顶端

	void updateSelectedItem(int id);					// 当某一item被选中时 设置当前的selectedItem

	void dealMoreFriendMsgRq(int id);

signals:
	void onChatView(Friend* data);						// 当某一item被选中时 向上更新聊天框的内容
	void moreFriendMsgRq(int id,long long msgId);
};

