#include "contactwidget.h"

ContactWidget::ContactWidget(QWidget *parent):
	ui(new Ui::ContactWidget),
	QWidget(parent)
{
	ui->setupUi(this);
	ui->stackedWidget->setCurrentIndex(1);
}

ContactWidget::~ContactWidget()
{
	delete ui;
}

void ContactWidget::setPage(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
}

void ContactWidget::addItem(Friend* data, FriendApplyType type)
{
	if (!data)return;
	QListWidgetItem* item = new QListWidgetItem;
	allFriend[data->getId()] = item;
	ui->listWidget_FriendReq->addItem(item);
	if (type == FriendApplyType::Accept)
	{
		NewFriendItem* widget = new NewFriendItem;
		widget->setFriend(data);
		connect(widget, &NewFriendItem::acceptFriendApply, this, &ContactWidget::dealAcceptFriendApply);
		ui->listWidget_FriendReq->setItemWidget(item, widget);
	}
	else
	{
		UserInfoShowDialog* widget = new UserInfoShowDialog;
		widget->setFriend(data);
		connect(widget, &UserInfoShowDialog::requestAddFriend, this, &ContactWidget::dealRequestAddFriend);
		ui->listWidget_FriendReq->setItemWidget(item, widget);
	}
	item->setSizeHint(QSize(600, 200));
}

void ContactWidget::dealAcceptFriendApply(Friend* data)
{
	removeFriendItem(data);
	emit acceptFriendApply(data);
}

void ContactWidget::dealRequestAddFriend(Friend* data)
{
	removeFriendItem(data);
	emit requestAddFriend(data);
}

void ContactWidget::removeFriendItem(Friend* data)
{
	int id = data->getId();
	if (!allFriend.contains(id))return;
	auto item = allFriend[id];
	allFriend.erase(id);
	delete ui->listWidget_FriendReq->takeItem(ui->listWidget_FriendReq->row(item));
}