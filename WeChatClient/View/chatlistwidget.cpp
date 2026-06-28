#include "chatlistwidget.h"

ChatListWidget::ChatListWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::ChatListWidget)
{
	ui->setupUi(this);
	// ui 文件的工作目录不匹配 重新设置一下图标
	ui->searchBtn->setIcon(QIcon("images/search2.svg"));
	ui->plusBtn->setIcon(QIcon("images/plus.svg"));
}

ChatListWidget::~ChatListWidget()
{
	delete ui;
}

void ChatListWidget::addItem(Friend* data, QString lastMsg, QString lastTime)
{
	QListWidgetItem* item = new QListWidgetItem;
	ChatListItem* widget = new ChatListItem(data);
	widget->updateMsgLabel(lastMsg);
	widget->updateTimeLable(lastTime);
	ui->listWidget->addItem(item);
	ui->listWidget->setItemWidget(item,widget);
	item->setSizeHint(QSize(200, 60));
	items[data->getId()] = item;
	connect(widget, &ChatListItem::toTopRq, this, &ChatListWidget::dealToTopRq);
	connect(widget, &ChatListItem::updateChatListItem, this, &ChatListWidget::updateSelectedItem);
	connect(widget, &ChatListItem::moreFriendMsgRq, this, &ChatListWidget::dealMoreFriendMsgRq);
}

void ChatListWidget::dealMoreFriendMsgRq(int id)
{
	emit moreFriendMsgRq(id,LLONG_MAX);
}
void ChatListWidget::updateItem(int friendid)
{
	if (!items.contains(friendid))return;
	auto item = items[friendid];
	ChatListItem* widget = static_cast<ChatListItem*>(ui->listWidget->itemWidget(item));
	widget->updateMsg();

}

void ChatListWidget::dealToTopRq(int id)
{
	if (!items.contains(id))return;
	auto item = items[id];
	int row = ui->listWidget->row(item);
	if (row == 0)return;
	auto widget=static_cast<ChatListItem*>(ui->listWidget->itemWidget(item));
	ui->listWidget->removeItemWidget(item);
	ui->listWidget->takeItem(row);
	ui->listWidget->insertItem(0, item);
	ui->listWidget->setItemWidget(item, widget);
}
void ChatListWidget::updateSelectedItem(int id)
{
	if (!items.contains(id))return;
	auto item = items[id];
	if (selectedItem)
	{
		ChatListItem* widget = static_cast<ChatListItem*>(ui->listWidget->itemWidget(selectedItem));
		widget->cancel();
	}
	selectedItem = item;
	ChatListItem* data = static_cast<ChatListItem*>(ui->listWidget->itemWidget(item));
	emit onChatView(data->getFriend());
}

bool ChatListWidget::haveItemSelected()
{
	return selectedItem != nullptr;
}
