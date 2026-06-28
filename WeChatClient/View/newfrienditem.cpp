#include "newfrienditem.h"

NewFriendItem::NewFriendItem(QWidget *parent):
	ui(new Ui::NewFriendItem),
	QWidget(parent)
{
	ui->setupUi(this);
	connect(ui->acceptBtn,&QPushButton::clicked,this,&NewFriendItem::on_acceptBtn_clicked);
}

NewFriendItem::~NewFriendItem()
{
	delete ui;
}

void NewFriendItem::setFriend(Friend* data)
{
	m_data = data;
	QString name = data->getName();
	QString avatar = data->getAvatar();
	ui->label_name->setText(name);
	ui->label_Introduction->setText("我是"+name);
	ui->label_icon->setPixmap(QPixmap(avatar));
}

Friend* NewFriendItem::getFriend()
{
	return m_data;
}

void NewFriendItem::on_acceptBtn_clicked()
{
	if (!m_data)return;
	emit acceptFriendApply(m_data);
}

