#pragma once

#include <QWidget>
#include "ui_chatlistitem.h"
#include "Model/friend.h"
#include <QPixmap>
namespace Ui { class ChatListItem; }
class ChatListItem  : public QWidget
{
	Q_OBJECT

public:
	ChatListItem(Friend*data_,QWidget *parent=nullptr);
	~ChatListItem();
	ChatListItem() = default;

	void setFriend(Friend* data);
	Friend* getFriend();

	void updateMsgLabel(QString msg);

	void updateTimeLable(QString time);

	void updateMsg();
	void cancel();				//取消选中状态
private:
	Ui::ChatListItem* ui;
	
	Friend* m_data=nullptr;
	
	QPixmap avatar;

	bool isSelected = false;	//设置是否是选中状态

	void enterEvent(QEvent* e)override;

	void leaveEvent(QEvent* e)override;

	void mousePressEvent(QMouseEvent* e)override;

	void initItem();

	QString solveContent(QString s);

	bool isFirstPress = true;
signals:
	//void onChatView(Friend* data);			// 鼠标下压时 更新聊天窗口的聊天信息
	void updateChatListItem(int id);		// 鼠标下压时 让上层被选中的item设置为未被选中
	void toTopRq(int id);					// 消息来临时 让上层将自己移动到顶端
	void moreFriendMsgRq(int id);
};

