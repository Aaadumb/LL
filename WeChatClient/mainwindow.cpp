#include "mainwindow.h"
MainWindow::MainWindow(QString avatar_,QWidget *parent):
	ui(new Ui::MainWindow),
	avatar(avatar_),
	QMainWindow(parent)
{
	ui->setupUi(this);
	chatlistwidget = new ChatListWidget(this);
	chatwidget = new ChatWidget(avatar, this);
	contactlistwidget = new ContactListWidget(this);
	contactwidget = new ContactWidget(this);

	//设置一下更新头像有关事件
	connect(ui->moreBtn, &QPushButton::clicked, this, &MainWindow::dealUpdateAvatar);

	chatwidget->hide();
	contactlistwidget->hide();
	contactwidget->hide();

	ui->headBtn->setIcon(QIcon(avatar));
	ui->gridLayout_2->addWidget(chatlistwidget);
	
	//这里是测试时添加 成品需要删除这里 初始时不添加聊天窗口
	//ui->gridLayout_4->addWidget(chatwidget);
	
	
	m_btn = ui->allchatBtn;

	// 设置窗口无边界 需要自己重写伸缩 放大放小 以及移动逻辑 先把这部分写完好了
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
	// 设置鼠标不按下左键依然可以跟踪moveevent
	setMouseTracking(true);
	// 安装事件过滤器
	installEventFilter(this);

	// 绑定一下事件
	connect(ui->maxinBtn, &QPushButton::clicked, this, &MainWindow::on_maxinBtn_clicked);
	connect(ui->mininBtn, &QPushButton::clicked, this, &MainWindow::on_mininBtn_clicked);
	connect(ui->closeBtn, &QPushButton::clicked, this, &MainWindow::on_closeBtn_clicked);
	connect(chatlistwidget, &ChatListWidget::onChatView, this, &MainWindow::dealChatView);
	connect(chatlistwidget, &ChatListWidget::moreFriendMsgRq, this, &MainWindow::dealMoreMsgRq);
	connect(chatwidget, &ChatWidget::SendFriendMsg, this, &MainWindow::dealSendFriendMsg);
	connect(chatwidget, &ChatWidget::moreMsgRq, this, &MainWindow::dealMoreMsgRq);
	connect(ui->allchatBtn, &QPushButton::clicked, this, &MainWindow::on_mBtn_clicked);
	connect(ui->contactBtn, &QPushButton::clicked, this, &MainWindow::on_mBtn_clicked);
	connect(contactwidget, &ContactWidget::acceptFriendApply, this, &MainWindow::dealAcceptFriendApply);
	connect(contactwidget, &ContactWidget::requestAddFriend, this, &MainWindow::dealRequestAddFriend);
	connect(contactlistwidget, &ContactListWidget::searchFriendInfoRq, this, &MainWindow::dealSearchFriendInfoRq);

}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent* e)
{
	isMousePressed = true;
	// 记录鼠标在全局屏幕坐标
	QPoint globalPos = QCursor::pos();
	// 记录鼠标相对于窗口左上角的偏移（用于拖动窗口）
	QPoint topLeft = frameGeometry().topLeft();
	QPoint offset = globalPos - topLeft;
	posx = offset.x();
	posy = offset.y();

	// 仍然计算 area（使用本地坐标检测边界）
	QPoint localPos = mapFromGlobal(globalPos);
	area = getArea(localPos.x(), localPos.y());
	return QMainWindow::mousePressEvent(e);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* e)
{
	isMousePressed = false;
	QApplication::restoreOverrideCursor();//将光标设置为默认状态
	return QMainWindow::mouseReleaseEvent(e);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (state == WidgetState::Maximized)
	{
		setWindowState(Qt::WindowState::WindowNoState);
		state = WidgetState::Normal;
	}
	else if (state == WidgetState::Normal)
	{
		setWindowState(Qt::WindowState::WindowMaximized);
		state = WidgetState::Maximized;
	}
	return QMainWindow::mouseDoubleClickEvent(e);
}

void MainWindow::mouseMoveEvent(QMouseEvent* e)
{
	if (isMousePressed && area!=MouseArea::External)
	{
		// 使用全局坐标与 frameGeometry（全局）配合，避免 local/global 混淆
		QPoint globalPos = QCursor::pos();
		QRect geom = frameGeometry(); // 全局坐标系下的窗口 rect

		// 为了避免边缘抖动或跳跃，基于当前 frameGeometry 的 left/top 保持不变（按需修改）
		const int minW = qMax(minimumWidth(), 200); // 可根据需求调整默认最小宽
		const int minH = qMax(minimumHeight(), 120);

		switch (area)
		{
		case MouseArea::Top:
		{
			int newTop = globalPos.y();
			// 防止太高或超越底部
			if (geom.bottom() - newTop < minH) newTop = geom.bottom() - minH;
			geom.setTop(newTop);
			break;
		}
		case MouseArea::Bottom:
		{
			int newBottom = globalPos.y();
			if (newBottom - geom.top() < minH) newBottom = geom.top() + minH;
			geom.setBottom(newBottom);
			break;
		}
		case MouseArea::TopLeft:
		{
			int newLeft = globalPos.x();
			int newTop = globalPos.y();
			// 限制最小宽高
			if (geom.right() - newLeft < minW) newLeft = geom.right() - minW;
			if (geom.bottom() - newTop < minH) newTop = geom.bottom() - minH;
			geom.setTopLeft(QPoint(newLeft, newTop));
			break;
		}
		case MouseArea::TopRight:
		{
			int left = geom.left();
			int newTop = globalPos.y();
			int newRight = globalPos.x();
			// 计算新宽度并限制
			int newWidth = newRight - left;
			if (newWidth < minW) newWidth = minW;
			// 限制 top 使高度不小于最小高度
			if (geom.bottom() - newTop < minH) newTop = geom.bottom() - minH;
			// 应用 new geometry（使用 left 保持不变）
			geom.setTop(newTop);
			geom.setRight(left + newWidth);
			break;
		}
		case MouseArea::BottomLeft:
		{
			int newLeft = globalPos.x();
			int top = geom.top();
			int newBottom = globalPos.y();
			// 限制最小宽高
			int newWidth = geom.right() - newLeft;
			if (newWidth < minW) newLeft = geom.right() - minW;
			int newHeight = newBottom - top;
			if (newHeight < minH) newBottom = top + minH;
			geom.setBottom(newBottom);
			geom.setLeft(newLeft);
			break;
		}
		case MouseArea::BottomRight:
		{
			int left = geom.left();
			int top = geom.top();
			int newRight = globalPos.x();
			int newBottom = globalPos.y();
			int newWidth = newRight - left;
			int newHeight = newBottom - top;
			if (newWidth < minW) newWidth = minW;
			if (newHeight < minH) newHeight = minH;
			geom.setRight(left + newWidth);
			geom.setBottom(top + newHeight);
			break;
		}
		case MouseArea::Left:
		{
			int newLeft = globalPos.x();
			// 限制最小宽
			if (geom.right() - newLeft < minW) newLeft = geom.right() - minW;
			geom.setLeft(newLeft);
			break;
		}
		case MouseArea::Right:
		{
			// 右侧伸缩：基于 left 计算新的宽度，避免使用 setRight 直接导致的跳动
			int left = geom.left();
			int newWidth = globalPos.x() - left;
			if (newWidth < minW) newWidth = minW;
			geom.setRight(left + newWidth);
			break;
		}
		case MouseArea::Internal:
			// 拖动窗口：newTopLeft = globalPos - offset
			{
				QPoint newTopLeft(globalPos.x() - posx, globalPos.y() - posy);
				// 使用 move 而不是直接 setGeometry，可以保留大小
				move(newTopLeft);
				return QMainWindow::mouseMoveEvent(e);
			}
			break;
		default:
			break;
		}

		// 防止尺寸变得过小（二次保险）
		if (geom.width() < minW) geom.setWidth(minW);
		if (geom.height() < minH) geom.setHeight(minH);

		// 将新的全局矩形应用到窗口（对顶级窗口 setGeometry 接受全局坐标）
		setGeometry(geom);
	}
	return QMainWindow::mouseMoveEvent(e);
}

bool MainWindow::eventFilter(QObject* sender, QEvent* event)
{
	if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
	{
		auto pos = QCursor::pos();
		// 转成本地坐标用于检测区域
		pos = mapFromGlobal(pos);
		auto area_ = getArea(pos.rx(), pos.ry());
		switch (area_)
		{
		case MouseArea::Top:
		case MouseArea::Bottom:
			setCursor(Qt::SizeVerCursor);
			break;
		case MouseArea::Left:
		case MouseArea::Right:
			setCursor(Qt::SizeHorCursor);
			break;
		case MouseArea::TopLeft:
		case MouseArea::BottomRight:
			setCursor(Qt::SizeFDiagCursor);
			break;
		case MouseArea::TopRight:
		case MouseArea::BottomLeft:
			setCursor(Qt::SizeBDiagCursor);
			break;
		default:
			setCursor(Qt::ArrowCursor);
		}
	}
	return QMainWindow::eventFilter(sender, event);
}

MouseArea MainWindow::getArea(int x, int y)
{
	// 使用 widget 的本地 rect（相对于自己左上角的坐标：[0,0..width,height]）来判断是否在窗口内
	QRect rect = this->rect(); // local coordinates
	if (!rect.contains(x, y))return MouseArea::External;
	int top = 0, bottom = rect.height(), left = 0, right = rect.width();
	if (y < top + 10)
	{
		//说明在上边界
		if (x < left + 10) return MouseArea::TopLeft;
		if (x > right - 10)return MouseArea::TopRight;
		return MouseArea::Top;
	}
	if (y > bottom - 10)
	{
		// 说明在下边界
		if (x < left + 10)return MouseArea::BottomLeft;
		if (x > right - 10)return MouseArea::BottomRight;
		return MouseArea::Bottom;
	}
	// 说明在左右边界 或者是内部
	if (x < left + 10)return MouseArea::Left;
	if (x > right - 10)return MouseArea::Right;
	return MouseArea::Internal;
}

void MainWindow::on_closeBtn_clicked()
{
	close();
	// 标记一下 后续估计这里需要触发一下事件
}

void MainWindow::on_maxinBtn_clicked()
{
	if (state == WidgetState::Maximized)
	{
		state = WidgetState::Normal;
		setWindowState(Qt::WindowState::WindowNoState);
	}
	else
	{
		state = WidgetState::Maximized;
		setWindowState(Qt::WindowState::WindowMaximized);
	}
}
void MainWindow::on_mininBtn_clicked()
{
	setWindowState(Qt::WindowState::WindowMinimized);
}

void MainWindow::dealChatView(Friend* data)
{
	if (pagetype != PageType::AllChat)return;
	chatwidget->setChatContent(data);
	if (ui->gridLayout_4->indexOf(chatwidget)==-1)
	{
		ui->gridLayout_4->addWidget(chatwidget);
	}
	chatwidget->show();
}
void MainWindow::dealSendFriendMsg(int id, Message msg)
{
	//这里我估计也需要等上层返回
	//if (pagetype == PageType::AllChat)
	//{
	//	chatlistwidget->updateItem(id, msg);
	//}
	emit sendFriendMsg(id, msg);
}
void MainWindow::dealMoreMsgRq(int friendid,long long id_)
{
	emit moreMsgRq(friendid, id_);
}

void MainWindow::addFriend(Friend* data, QString lastMsg, QString lastTime)
{
	chatlistwidget->addItem(data,lastMsg,lastTime);
}

void MainWindow::receiveFriendNewMsg(Friend* data, Message msg)
{
	chatlistwidget->updateItem(data->getId());
	if (data==chatwidget->getFriend())
	{
		chatwidget->tailAddText(msg);
	}
}
void MainWindow::receiveFriendOldMsg(Friend* data, Message msg)
{
	chatlistwidget->updateItem(data->getId());
	if (data == chatwidget->getFriend())
	{
		chatwidget->headAddText(msg);
	}
}

void MainWindow::on_mBtn_clicked()
{
	QPushButton* btn = (QPushButton*)QObject::sender();
	if (btn == m_btn)
	{
		m_btn->setChecked(true);
		return;
	}
	if (pagetype == PageType::AllChat)
	{
		ui->gridLayout_2->removeWidget(chatlistwidget);
		chatlistwidget->hide();
		if (ui->gridLayout_4->indexOf(chatwidget) != -1)
		{
			ui->gridLayout_4->removeWidget(chatwidget);
			chatwidget->hide();
		}
		pagetype = PageType::AddFriend;
		ui->gridLayout_2->addWidget(contactlistwidget);
		ui->gridLayout_4->addWidget(contactwidget);
		contactlistwidget->show();
		contactwidget->show();
	}
	else
	{
		ui->gridLayout_2->removeWidget(contactlistwidget);
		ui->gridLayout_4->removeWidget(contactwidget);
		contactlistwidget->hide();
		contactwidget->hide();
		pagetype = PageType::AllChat;
		ui->gridLayout_2->addWidget(chatlistwidget);
		chatlistwidget->show();
		//测试代码 正式上线需要注释
		if (chatlistwidget->haveItemSelected())
		{
			ui->gridLayout_4->addWidget(chatwidget);
			chatwidget->show();
		}
	}
	m_btn->setChecked(false);
	m_btn = btn;
	m_btn->setChecked(true);
}

void MainWindow::dealAcceptFriendApply(Friend* data)
{
	emit acceptFriendApply(data);
}

void MainWindow::dealSearchFriendInfoRq(QString name)
{
	emit searchFriendInfoRq(name);
}

void MainWindow::dealRequestAddFriend(Friend* data)
{
	emit requestAddFriend(data);
}


void MainWindow::solveSearchFriend(Friend* data)
{
	contactwidget->addItem(data, FriendApplyType::Request);
}

void MainWindow::solveRequestFriend(Friend* data)
{
	contactwidget->addItem(data, FriendApplyType::Accept);
}

void MainWindow::solveUpdateAvatar(QString avatar_)
{
	avatar = avatar_;
	ui->headBtn->setIcon(QIcon(avatar));
	if (chatwidget)
	{
		chatwidget->updateAvatar(avatar);
	}
}

void MainWindow::dealUpdateAvatar()
{
		emit updateAvatar();
}