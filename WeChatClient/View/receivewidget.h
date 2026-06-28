#pragma once

#include <QWidget>
#include "../Model/label.h"
#include "ui_receivewidget.h"
namespace Ui { class ReceiveWidget; }
class ReceiveWidget  : public QWidget
{
	Q_OBJECT

public:
	explicit ReceiveWidget(QWidget *parent=nullptr);
	~ReceiveWidget();
	void setArratar(QString s);
	void setContent( QString s);
private:
	Ui::ReceiveWidget *ui;
};

