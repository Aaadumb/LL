#include "userinfoshowdialog.h"

UserInfoShowDialog::UserInfoShowDialog(QDialog*parent):
	ui(new Ui::UserInfoShowDialog),
	QDialog(parent)
{
	ui->setupUi(this);
	connect(ui->addFriendBtn, &QPushButton::clicked, this, &UserInfoShowDialog::on_addFriendBtn_clicked);
}

UserInfoShowDialog::~UserInfoShowDialog()
{
	delete ui;
}

void UserInfoShowDialog::setFriend(Friend* data)
{
	m_data = data;
	QString name = data->getName();
	QString avatar = data->getAvatar();
	ui->label_username->setText(name);
	ui->iconBtn->setIcon(QIcon(avatar));
}

Friend* UserInfoShowDialog::getFriend()
{
	return m_data;
}

void UserInfoShowDialog::on_addFriendBtn_clicked()
{
	emit requestAddFriend(m_data);
}