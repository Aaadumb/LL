#include "chatlistitem.h"
ChatListItem::ChatListItem(Friend *data_,QWidget *parent)
	:m_data(data_),
	QWidget(parent),
	ui(new Ui::ChatListItem)
{
	ui->setupUi(this);
	this->installEventFilter(this);
	initItem();
	ui->widget->setStyleSheet("QWidget#widget{background: white;}");
}
ChatListItem::~ChatListItem()
{
	delete ui;
}

void ChatListItem::initItem()
{
	if (m_data)
	{
		avatar.load(m_data->getAvatar());
		ui->headLbl->setPixmap(avatar);
		if (m_data->getCount() > 0)
		{
			ui->widget_5->setVisible(true);
			ui->countLbl->setText(QString::number(m_data->getCount()));
			emit toTopRq(m_data->getId());
		}
		else
		{
			ui->widget_5->setVisible(false);
		}
		ui->nameLbl->setText(m_data->getName());
		if (m_data->haveMsg())
		{
			auto msg = m_data->getLastestMeg();
			ui->messLbl->setText(solveContent(msg.getInfor()));
			ui->timeLbl->setText(msg.getTime());
		}
	}
}

void ChatListItem::setFriend(Friend*data)
{
	m_data = data;
	initItem();
}

Friend* ChatListItem::getFriend()
{
	return m_data;
}

void ChatListItem::mousePressEvent(QMouseEvent* e)
{
	if (!isSelected)
	{
		ui->widget->setStyleSheet("QWidget#widget{background: green;}");
		//setStyleSheet("background-color: green;");
		isSelected = true;
		m_data->setCount(0);
		ui->widget_5->setVisible(false);
		//emit onChatView(m_data);
		emit updateChatListItem(m_data->getId());
		if (isFirstPress)
		{
			isFirstPress = false;
			emit moreFriendMsgRq(m_data->getId());
		}
	}
	QWidget::mousePressEvent(e);
}

void ChatListItem::enterEvent(QEvent* e)
{
	//如果当前已经是选中状态 不做操作
	if (!isSelected)
	{
		ui->widget->setStyleSheet("QWidget#widget{background: #f0f0f0;}");
	}
	QWidget::enterEvent(e);
}

void ChatListItem::leaveEvent(QEvent* e)
{
	if (!isSelected)
	{
		ui->widget->setStyleSheet("QWidget#widget{background: white;}");
	}
	QWidget::leaveEvent(e);
}

void ChatListItem::cancel()
{
	if (isSelected)
	{
		isSelected = false;
		ui->widget->setStyleSheet("QWidget#widget{background: white;}");
	}
}
void ChatListItem::updateMsg()
{
	if (!m_data)
	{
		return;
	}
	Message msg = m_data->getLastestMeg();
	ui->messLbl->setText(solveContent(msg.getInfor()));
	ui->timeLbl->setText(msg.getTime());
	if (!isSelected && msg.getType() == MsgType::Reveive)
	{
		ui->countLbl->setText(QString::number(m_data->getCount()));
		ui->widget_5->setVisible(true);
		emit toTopRq(m_data->getId());
	}
	else
	{
		m_data->setCount(0);
		ui->widget_5->setVisible(false);
	}
}

QString ChatListItem::solveContent(QString content)
{
	QFontMetrics metrics(font()); // 字体度量对象，用于测量文本的宽度。
	// 根据给定的最大宽度(150像素)对文本进行截断，并添加省略号（...）以表示被截断了。截断的位置是在文本的右侧
	auto new_content = metrics.elidedText(content, Qt::ElideRight, 150);
	return new_content;
}

void ChatListItem::updateMsgLabel(QString msg)
{
	ui->messLbl->setText(solveContent(msg));
}

void ChatListItem::updateTimeLable(QString time)
{
	ui->timeLbl->setText(time);
}