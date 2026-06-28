#include "receivewidget.h"

ReceiveWidget::ReceiveWidget(QWidget *parent):
	QWidget(parent),
	ui(new Ui::ReceiveWidget)
{
	ui->setupUi(this);
}

ReceiveWidget::~ReceiveWidget()
{
	delete ui;
}

void ReceiveWidget::setArratar(QString arratar)
{
	ui->headBtn->setIcon(QIcon(arratar));
}

void ReceiveWidget::setContent(QString text)
{
	ui->msgLbl->setText(text);
	//resize(ui->msgLbl->width(), ui->msgLbl->height());
	adjustSize();
}