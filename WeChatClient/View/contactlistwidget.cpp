#include "contactlistwidget.h"

ContactListWidget::ContactListWidget(QWidget *parent):
	ui(new Ui::ContactListWidget),
	QWidget(parent)
{
	ui->setupUi(this);
	connect(ui->lineEdit, &QLineEdit::textChanged, this, &ContactListWidget::on_lineEdit_textChanged);
	connect(ui->addBtn, &QPushButton::clicked, this, &ContactListWidget::on_addBtn_clicked);
	connect(ui->cancelBtn, &QPushButton::clicked, this, &ContactListWidget::on_cancelBtn_clicked);
	connect(ui->addFriendBtn, &QPushButton::clicked, this, &ContactListWidget::on_addFriendBtn_clicked);

	// ui文件有点bug 手动设置一下图片
	ui->addBtn->setIcon(QIcon("images/addFriend.svg"));
	ui->label->setPixmap(QPixmap("images/search.png"));
	ui->searchBtn->setIcon(QIcon("images/search2.svg"));
	ui->pushButton_2->setIcon(QIcon("images/contact.svg"));
}

ContactListWidget::~ContactListWidget()
{
	delete ui;
}

void ContactListWidget::on_lineEdit_textChanged()
{
	QString name = ui->lineEdit->text();
	ui->label_confirm->setText(name);
}

void ContactListWidget::on_addBtn_clicked()
{
	ui->contactStackedWidget->setCurrentIndex(1);
}

void ContactListWidget::on_cancelBtn_clicked()
{
	ui->contactStackedWidget->setCurrentIndex(0);
}

void ContactListWidget::on_addFriendBtn_clicked()
{
	QString name = ui->lineEdit->text();
	if (name.isEmpty())
	{
		return;
	}
	emit searchFriendInfoRq(name);
}