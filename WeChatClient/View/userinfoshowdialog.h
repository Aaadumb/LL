#pragma once

#include <QDialog>
#include "ui_userinfoshowdialog.h"
#include "../Model/friend.h"
namespace Ui { class UserInfoShowDialog; }
class UserInfoShowDialog  : public QDialog
{
	Q_OBJECT

public:
	UserInfoShowDialog(QDialog*parent=nullptr);
	~UserInfoShowDialog();
	
	void setFriend(Friend* data);
	Friend* getFriend();

signals:
	void requestAddFriend(Friend* data);
private:
	Ui::UserInfoShowDialog* ui;
	Friend* m_data=nullptr;
	void on_addFriendBtn_clicked();

};

