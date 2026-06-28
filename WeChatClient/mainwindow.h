#pragma once

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "View/chatlistwidget.h"
#include "View/chatwidget.h"
#include "View/contactlistwidget.h"
#include "View/contactwidget.h"
#include <QMenu>
namespace Ui { class MainWindow; }

enum MouseArea
{
	External=0,
	Top,
	Bottom,
	Left,
	Right,
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	Internal,
};
enum WidgetState
{
	Normal=0,
	Maximized
};
enum PageType {
	AllChat = 0,
	AddFriend,
};
class MainWindow  : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QString avatar_,QWidget *parent=nullptr);
	~MainWindow();

	// 处理上层可能发送来的事件

	// 用于更新 chatlistwidget 主要用于初始化内容
	void addFriend(Friend* data,QString lastMsg="",QString lastTime="");

	void receiveFriendNewMsg(Friend* data, Message msg);	// 用于处理跟朋友的新消息

	void receiveFriendOldMsg(Friend* data, Message msg);	// 用于处理跟朋友的旧消息

	//还需要处理发送来的朋友申请和搜索到的朋友信息
	void solveSearchFriend(Friend* data);

	void solveRequestFriend(Friend* data);

	//同时如果更新了头像 还需要同时更新一下
	void solveUpdateAvatar(QString avatar_);
private:
	Ui::MainWindow* ui;

	ChatListWidget* chatlistwidget=nullptr;

	ChatWidget* chatwidget=nullptr;

	ContactListWidget* contactlistwidget = nullptr;

	ContactWidget* contactwidget = nullptr;

	QString avatar;

	QMenu* m_menu = nullptr;

	// 对于鼠标逻辑 首先我们需要重写的鼠标逻辑函数有 press move leave 以及悬停
	// 对于最后一个 写一个事件过滤器
	bool eventFilter(QObject* sender, QEvent* e)override;

	void mousePressEvent(QMouseEvent* e)override;

	void mouseMoveEvent(QMouseEvent* e)override;

	void mouseDoubleClickEvent(QMouseEvent *e)override;

	void mouseReleaseEvent(QMouseEvent* e)override;
	
	bool isMousePressed = false;

	//记录相对位置
	int posx;

	int posy;

	MouseArea area = MouseArea::External;

	WidgetState state = WidgetState::Normal;

	//得到点(x,y) 对于窗口的位置信息
	MouseArea getArea(int x,int y);

	void on_maxinBtn_clicked();

	void on_mininBtn_clicked();

	void on_closeBtn_clicked();



	// 记录当前的页面信息 方便处理不同的事件类型
	QPushButton* m_btn = nullptr;
	PageType pagetype = AllChat;

	void on_mBtn_clicked();			//用于处理当按钮点击时 内部布局的显示问题

	
	// 处理下层ui 发送的事件
	void dealChatView(Friend *data);

	void dealSendFriendMsg(int id, Message msg);

	void dealMoreMsgRq(int friendid,long long id=LLONG_MAX);

	void dealSearchFriendInfoRq(QString name);

	void dealAcceptFriendApply(Friend* data);

	void dealRequestAddFriend(Friend* data);

	void dealUpdateAvatar();
	
signals:
	// 第二版的时候把命名规则改一下 全部改成SIG_开头的
	void sendFriendMsg(int id, Message msg);
	void moreMsgRq(int friendid, long long id);
	void searchFriendInfoRq(QString name);
	void acceptFriendApply(Friend* data);
	void requestAddFriend(Friend* data);
	void updateAvatar();
};

// 首先作为中心的控制窗口 需要和外部联系 但是只管ui 不管数据