#include "senderwidget.h"

SenderWidget::SenderWidget(QWidget *parent):
	QWidget(parent),
	ui(new Ui::SenderWidget)
{
	ui->setupUi(this);
}

SenderWidget::~SenderWidget()
{
	delete ui;
}

void SenderWidget::setContent(QString text)
{
	ui->msgLbl->setText(text);
	//resize(ui->msgLbl->width(),ui->msgLbl->height());
	adjustSize();
}

void SenderWidget::setArratar(QString arratar)
{
	ui->headBtn->setIcon(QIcon(arratar));
}