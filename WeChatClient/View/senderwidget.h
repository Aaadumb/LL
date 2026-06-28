#pragma once

#include <QWidget>
#include "../Model/label.h"
#include "ui_senderwidget.h"
namespace Ui { class SenderWidget; }
class SenderWidget  : public QWidget
{
	Q_OBJECT

public:
	explicit SenderWidget(QWidget *parent=nullptr);
	~SenderWidget();
	void setContent(QString s);
	void setArratar(QString s);
private:
	Ui::SenderWidget* ui;
};

