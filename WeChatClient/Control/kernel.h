#pragma once

#include <QObject>
#include <vector>
#include "../mainwindow.h"
#include "../logindialog.h"
#include "../Net/communication.h"
#include "json.hpp"
#include <fstream>
#include <QFileDialog>
#include <QTimer>
using json = nlohmann::json;
// 由这个类专门处理信息的整合 和 拆析 并向网络层传递信息 因为网络层由信号传递机制 所以让kernel也接受网络层的消息
enum EventType {
	Login = 0,
	Register,
	UploadAvatar,
	SearchFriend,
	RequestFriend,
	AcceptFriend,
	SendMessage,
	MoreFriendMsg,
	FriendRequestInit,
	FriendInit
};
class Kernel  : public QObject
{
	Q_OBJECT

public:
	Kernel(QObject *parent=nullptr);
	~Kernel();
private:
	// 第一个哈希表用于存储 用户所有朋友的信息
	std::unordered_map<int, Friend*> allFriend;

	// 第二个哈希表存储 所有可能是新朋友的人
	std::unordered_map<int, Friend*> newFriend;
	int id;
	QString avatar;
	MainWindow* mainwindow = nullptr;
	LoginDialog* login = nullptr;
	Communication* communication = nullptr;

	// 正常的用户流程函数
	void init();

	void dealMoreMsgRq(int friend_id, long long msgId);
	void dealSendFriendMsg(int friend_id, Message msg);
	void dealSIG_LoginCommit(QString username, QString password);
	void dealSIG_RegisterCommit(QString username, QString tel, QString password);
	void dealSIG_CloseLoginDialog();
	void dealReadMsg(std::string msg);
	void dealSearchFriendInfoRq(QString name);
	void dealUpdateAvatar();
	void dealAcceptFriendApply(Friend* data);
	void dealRequestAddFriend(Friend* data);

	void solveLogin(json& js);
	void solveRegister(json& js);
	void solveUploadAvatar(json& js);
	void solveSearchFriend(json& js);
	void solveRequestFriend(json& js);
	void solveAcceptFriend(json& js);
	void solveSendMessage(json& js);
	void solveMoreFriendMsg(json& js);
	void solveFriendRequestInit(json& js);
	void solveFriendInit(json& js);

	// 处理头像文件的函数
	std::string encode(std::string path);
	std::string base64_encode(const std::string& s);
	std::string base64_decode(const std::string& s);
	void storageAvatar(std::string& path,std::string& image);
	std::string updateAvatar(int id_,const std::string& s);

};

