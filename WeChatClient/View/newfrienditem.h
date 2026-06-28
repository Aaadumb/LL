#pragma once

#include <QWidget>
#include "ui_newfrienditem.h"
#include "../Model/friend.h"
namespace Ui { class NewFriendItem; }
class NewFriendItem  : public QWidget
{
	Q_OBJECT

public:
	NewFriendItem(QWidget *parent=nullptr);
	~NewFriendItem();
	void setFriend(Friend*);
	Friend* getFriend();
signals:
	void acceptFriendApply(Friend* data);
private:
	Ui::NewFriendItem* ui;
	Friend* m_data=nullptr;
	void on_acceptBtn_clicked();
};

