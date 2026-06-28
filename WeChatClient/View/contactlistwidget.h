#pragma once

#include <QWidget>
#include "ui_contactlistwidget.h"
namespace Ui { class ContactListWidget; }

// 先分析需求 这个类主要的操作对象就是根据searchEdit的变化 改变stackwidget
// 当改变的同时 点击按钮下发的时候 向上传递 我要请求根据名字查找对象
// 介于这个情况 目前暂定 将friend的new与释放放到net层 net层目测还要绑定一个类的
// 估计不会太臃肿
class ContactListWidget  : public QWidget
{
	Q_OBJECT

public:
	ContactListWidget(QWidget *parent=nullptr);
	~ContactListWidget();

private:
	Ui::ContactListWidget* ui;
	void on_lineEdit_textChanged();
	void on_addBtn_clicked();
	void on_cancelBtn_clicked();
	void on_addFriendBtn_clicked();
signals:
	void searchFriendInfoRq(QString name);
};

