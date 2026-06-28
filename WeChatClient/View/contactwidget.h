#pragma once

#include <QWidget>
#include "ui_contactwidget.h"
#include "newfrienditem.h"
#include "userinfoshowdialog.h"
#include <unordered_map>
namespace Ui { class ContactWidget; }
enum FriendApplyType {
	Request,
	Accept,
};
class ContactWidget  : public QWidget
{
	Q_OBJECT

public:
	ContactWidget(QWidget *parent=nullptr);
	
	~ContactWidget();
	
	void setPage(int index);
	
	void addItem(Friend* data, FriendApplyType type);

signals:
	void acceptFriendApply(Friend* data);

	void requestAddFriend(Friend* data);
private:
	Ui::ContactWidget* ui;
	
	std::unordered_map<int, QListWidgetItem*> allFriend;

	void dealAcceptFriendApply(Friend* data);
	
	void dealRequestAddFriend(Friend* data);
	
	void removeFriendItem(Friend* data);
};

