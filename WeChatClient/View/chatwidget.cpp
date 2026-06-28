#include "chatwidget.h"

ChatWidget::ChatWidget(QString avatar,QWidget*parent):
	m_avatar(avatar),
	QWidget(parent),
	ui(new Ui::ChatWidget)
{
	ui->setupUi(this);
	ui->listWidget->scrollToBottom();

	QScrollBar* bar = ui->listWidget->verticalScrollBar();
	connect(bar, &QScrollBar::valueChanged, this, &ChatWidget::onScrollBarChange);
	connect(ui->sendBtn, &QPushButton::clicked, this, &ChatWidget::on_sendBtn_clicked);


	ui->listWidget->setStyleSheet(R"(
        QListWidget::item {
            background: transparent;
        }

        QListWidget::item:hover {
            background: transparent;
        }

        QListWidget::item:selected {
            background: transparent;
            color: black;
        }

        QListWidget::item:selected:active {
            background: transparent;
        }

        QListWidget::item:selected:!active {
            background: transparent;
        }
        )");
}

ChatWidget::~ChatWidget()
{
	delete ui;
}

void ChatWidget::setChatContent(Friend* data)
{
	// 调用这个函数时 默认更换了朋友对象
	m_data = data;
	ui->listWidget->clear();
	if (!data)return;
	auto msgs = data->getAllMsg();
	ui->nameLbl->setText(data->getName());
	for (auto& msg : msgs)
	{
		tailAddText(msg);
	}
	ui->listWidget->scrollToBottom();
}

void ChatWidget::on_sendBtn_clicked()
{
	QString content = ui->sendEdit->toPlainText();
	if (content.isEmpty())return;
	ui->sendEdit->clear();
	Message msg;
	msg.setInfor(content);
	msg.setType(MsgType::Send);
	msg.setTime(QTime::currentTime().toString("hh:mm:ss"));
	// 向上层发个消息 告诉它 我这边发消息了
	//tailAddText(msg);
	ui->listWidget->scrollToBottom();
	if (m_data)
	{
		emit SendFriendMsg(m_data->getId(), msg);
	}
}

void ChatWidget::headAddText(Message msg)
{
	// 头部添加消息要防抖
	// 记录滚动条的位置 方便后续防抖
	QScrollBar* bar = ui->listWidget->verticalScrollBar();
	int oldValue = bar->value();
	int oldMax = bar->maximum();
	dealText(msg, 0);
	int newMax = bar->maximum();
	bar->setValue(oldValue + (newMax - oldMax));
}

void ChatWidget::tailAddText(Message msg)
{
	dealText(msg, 1);
	ui->listWidget->scrollToBottom();
}


void ChatWidget::onScrollBarChange()
{
	// 标记一下 这里是  bug
	//return;
	if (!m_data)
	{
		return;
	}
	QScrollBar* bar = ui->listWidget->verticalScrollBar();
	int curMin = bar->minimum();
	int curValue = bar->value();


	if (curValue != curMin || m_data->getIsAll())return;
	// 先在这里写 后面看看需不需要重新补逻辑
	m_data->setIsAll(true);
	
	
	// 触发信号 等待服务端把消息发过来 等发过来再调用头插法慢慢显示
	// 得到最新的时间
	if (!m_data->haveMsg())
	{
		emit moreMsgRq(m_data->getId(),LLONG_MAX);
	}
	else
	{
		long long id= m_data->getOldestMsg().getId();
		emit moreMsgRq(m_data->getId(),id);
	}
}

void ChatWidget::dealText(Message &msg, int opt)
{
	QListWidgetItem* item = new QListWidgetItem;
	if (opt == 1)ui->listWidget->addItem(item);
	else ui->listWidget->insertItem(0, item);
	if (msg.getType() == MsgType::Send)
	{
		ReceiveWidget* widget = new ReceiveWidget;
		widget->setArratar(m_avatar);
		ui->listWidget->setItemWidget(item, widget);
		widget->setContent(msg.getInfor());
		item->setSizeHint(widget->sizeHint());
	}
	else if (msg.getType() == MsgType::Reveive)
	{
		SenderWidget* widget = new SenderWidget;
		widget->setArratar(m_data->getAvatar());
		ui->listWidget->setItemWidget(item, widget);
		widget->setContent(msg.getInfor());
		item->setSizeHint(widget->sizeHint() );
	}
}

Friend* ChatWidget::getFriend()
{
	return m_data;
}

void ChatWidget::updateAvatar(QString avatar)
{
	m_avatar = avatar;
}