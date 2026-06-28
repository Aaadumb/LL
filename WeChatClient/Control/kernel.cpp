#include "kernel.h"

Kernel::Kernel(QObject *parent)
	: QObject(parent)
{
	communication = new Communication(this);
	login = new LoginDialog;
	connect(login, &LoginDialog::SIG_LoginCommit, this, &Kernel::dealSIG_LoginCommit);
	connect(login, &LoginDialog::SIG_RegisterCommit, this, &Kernel::dealSIG_RegisterCommit);
	connect(login, &LoginDialog::SIG_CloseLoginDialog, this, &Kernel::dealSIG_CloseLoginDialog);
	connect(communication, &Communication::readMsg, this, &Kernel::dealReadMsg);
	
	login->show();
}

Kernel::~Kernel()
{
	// 先把所有new出来的东西全部释放
	for (auto& [key, val] : allFriend)
	{
		delete val;
	}
	for (auto& [key, val] : newFriend)
	{
		delete val;
	}
	allFriend.clear();
	newFriend.clear();
	if (login)
	{
		delete login;
	}
	if (communication)
	{
		delete communication;
	}
	if (mainwindow)
	{
		delete mainwindow;
	}
}


void Kernel::init()
{
	login->hide();
	mainwindow = new MainWindow(avatar);
	
	// 需要绑定一些事件
	connect(mainwindow, &MainWindow::sendFriendMsg, this, &Kernel::dealSendFriendMsg);
	connect(mainwindow, &MainWindow::moreMsgRq, this, &Kernel::dealMoreMsgRq);
	connect(mainwindow, &MainWindow::searchFriendInfoRq, this, &Kernel::dealSearchFriendInfoRq);
	connect(mainwindow, &MainWindow::acceptFriendApply, this, &Kernel::dealAcceptFriendApply);
	connect(mainwindow, &MainWindow::requestAddFriend, this, &Kernel::dealRequestAddFriend);
	connect(mainwindow, &MainWindow::updateAvatar, this, &Kernel::dealUpdateAvatar);
	mainwindow->show();
}

void Kernel::dealSearchFriendInfoRq(QString name)
{
	json js;
	js["kind"] = EventType::SearchFriend;
	js["according"] = name.toStdString();
	communication->dealWrite(js.dump());
}

void Kernel::dealAcceptFriendApply(Friend* data)
{
	if (!data)
	{
		return;
	}
	json js;
	js["kind"] = EventType::AcceptFriend;
	js["id1"] = id;
	js["id2"] = data->getId();
	communication->dealWrite(js.dump());
}

void Kernel::dealRequestAddFriend(Friend* data)
{
	if (!data)
	{
		return;
	}
	json js;
	js["kind"] = EventType::RequestFriend;
	js["id1"] = id;
	js["id2"] = data->getId();
	communication->dealWrite(js.dump());
}

void Kernel::dealUpdateAvatar()
{
	// 从根目录中让用户选择头像
	QString filePath = QFileDialog::getOpenFileName(mainwindow, "选择头像", QDir::homePath(), QObject::tr("Images (*.png *.jpg);;All Files (*)"));
	if (filePath.isEmpty())return;//未选择头像
	if (!filePath.endsWith(".png") && !filePath.endsWith(".jpg"))
	{
		// 所选的内容不合法
		QMessageBox::about(mainwindow, "提示", "文件类型必须为jpg或png");
		return;
	}
	// 所选路径合法 打包告知服务器 让服务器确认
	json js;
	js["kind"] = EventType::UploadAvatar;
	js["id"] = id;
	js["image"] = base64_encode(encode(filePath.toStdString()));
	// 打包发给客户端
	communication->dealWrite(js.dump());
}

void Kernel::dealMoreMsgRq(int friend_id, long long msgId)
{
	if (!allFriend.contains(friend_id))
	{
		return;
	}
	json js;
	js["kind"] = EventType::MoreFriendMsg;
	js["id1"] = id;
	js["id2"] = friend_id;
	js["msgId"] = msgId;
	communication->dealWrite(js.dump());
}

void Kernel::dealSendFriendMsg(int friend_id, Message msg)
{
	if (!allFriend.contains(friend_id))
	{
		return;
	}
	json js;
	js["kind"] = EventType::SendMessage;
	js["id1"] = id;
	js["id2"] = friend_id;
	js["content"] = msg.getInfor().toStdString();
	js["time"] = msg.getTime().toStdString();
	communication->dealWrite(js.dump());
}

void Kernel::dealSIG_LoginCommit(QString username, QString password)
{
	if (!communication->getState())
	{
		login->showErrorTips("未能与服务器正常连接");
	}
	else
	{
		json msg;
		msg["kind"] = EventType::Login;
		msg["name"] = username.toStdString();
		msg["pwd"] = password.toStdString();
		communication->dealWrite(msg.dump());
	}
}

void Kernel::dealSIG_RegisterCommit(QString username, QString tel, QString password)
{
	if (!communication->getState())
	{
		login->showErrorTips("未能与服务器正常连接");
	}
	else
	{
		json msg;
		msg["kind"] = EventType::Register;
		msg["name"] = username.toStdString();
		msg["phone"] = tel.toStdString();
		msg["pwd"] = password.toStdString();
		communication->dealWrite(msg.dump());
	}
}


void Kernel::dealSIG_CloseLoginDialog()
{
	QTimer::singleShot(0, QCoreApplication::instance(), &QCoreApplication::quit); // 退出事件循环
}

void Kernel::dealReadMsg(std::string msg)
{
	json js;
	//std::ofstream ofs("log.txt", std::ios::out);
	//ofs << "写入数据:";
	//ofs << msg << '\n';
	try
	{
		js = json::parse(msg);
	}
	catch (const json::exception& e)
	{
		qDebug() << e.what() << '\n';
		return;
	}
	if (!js.contains("kind"))
	{
		return;
	}
	int kind = js["kind"].get<int>();
	switch (kind)
	{
	case EventType::Login:
		solveLogin(js);
		break;
	case EventType::Register:
		solveRegister(js);
		break;
	case EventType::AcceptFriend:
		solveAcceptFriend(js);
		break;
	case EventType::FriendInit:
		solveFriendInit(js);
		break;
	case EventType::FriendRequestInit:
		solveFriendRequestInit(js);
		break;
	case EventType::MoreFriendMsg:
		solveMoreFriendMsg(js);
		break;
	case EventType::RequestFriend:
		solveRequestFriend(js);
		break;
	case EventType::SearchFriend:
		solveSearchFriend(js);
		break;
	case EventType::SendMessage:
		solveSendMessage(js);
		break;
	case EventType::UploadAvatar:
		solveUploadAvatar(js);
		break;
	default:
		break;
	}
}

void Kernel::solveLogin(json& js)
{
	if (!js.contains("res"))
	{
		return;
	}
	int res = js["res"].get<int>();
	if (res == -1)
	{
		if (!js.contains("reason"))
		{
			login->showErrorTips("发生未知错误");
		}
		else
		{
			std::string reason = js["reason"].get<std::string>();
			// 交给登录处理
			login->showErrorTips(reason.data());
		}
	}
	else
	{
		if (!js.contains("id") || !js.contains("avatar"))
		{
			login->showErrorTips("发生未知错误");
		}
		else
		{
			id = js["id"].get<int>();
			avatar=updateAvatar(id,js["avatar"].get<std::string>()).data();
			init();
			qDebug() << "登录成功!\n";
		}
	}
}

void Kernel::solveRegister(json& js)
{
	if (!js.contains("res"))
	{
		return;
	}
	int res = js["res"].get<int>();
	if (res == -1)
	{
		if (!js.contains("reason"))
		{
			login->showErrorTips("发生未知错误");
		}
		else
		{
			std::string reason = js["reason"].get<std::string>();
			login->showErrorTips(reason.data());
		}
	}
	else
	{
		if (!js.contains("id") || !js.contains("avatar"))
		{
			login->showErrorTips("发生未知错误");
		}
		else
		{
			id = js["id"].get<int>();
			//处理登录之后的逻辑
			avatar = updateAvatar(id, js["avatar"].get<std::string>()).data();
			init();
			qDebug() << "注册成功!\n";
		}
	}
}

void Kernel::solveUploadAvatar(json& js)
{
	if (!js.contains("res") || !js.contains("id") || !js.contains("image"))
	{
		return;
	}
	// 处理逻辑
	if (js["res"].get<int>() == -1 || id!=js["id"].get<int>())
	{
		qDebug() << "更新头像失败!\n";
		return;
	}
	avatar = updateAvatar(id, js["image"].get<std::string>()).data();
	// 向下层通知
	if (mainwindow)
	{
		mainwindow->solveUpdateAvatar(avatar);
	}
}
void Kernel::solveSearchFriend(json& js)
{
	if (!js.contains("friend"))
	{
		return;
	}
	if (!js["friend"].is_array())
	{
		return;
	}
	for (json& people : js["friend"])
	{
		if (!people.contains("id") || !people.contains("name") || !people.contains("avatar"))
		{
			continue;
		}
		//处理逻辑 逻逻辑也比较简单 创建一个朋友 然后放到newfriend里面
		Friend* p = new Friend;
		int id_ = people["id"].get<int>();
		if (!newFriend.contains(id_))
		{
			p->setId(id_);
			p->setName(people["name"].get<std::string>().data());
			p->setAvatar(updateAvatar(id_, people["avatar"].get<std::string>()).data());
			newFriend[id_] = p;
		}
		// 向下处理
		if (mainwindow)
		{
			mainwindow->solveSearchFriend(p);
		}
	}
}
void Kernel::solveRequestFriend(json& js)
{
	// 出现这个的时候表示有人在你在线的时候想你发送了好友申请
	// 并且这个属于在线消息 如果当前newfriend记录了 就不再继续响应
	if (!js.contains("id") || !js.contains("name") || !js.contains("avatar"))
	{
		return;
	}
	//处理逻辑
	int id_ = js["id"].get<int>();
	if (newFriend.contains(id_))
	{
		return;
	}
	Friend* people = new Friend;
	people->setId(id_);
	people->setName(js["name"].get<std::string>().data());
	people->setAvatar(updateAvatar(id_, js["avatar"].get<std::string>()).data());
	newFriend[id_] = people;
	// 向下传递消息
	if (mainwindow)
	{
		mainwindow->solveRequestFriend(people);
	}
}
void Kernel::solveAcceptFriend(json& js)
{
	// 因为如果在线 两边都是有对方的信息的 所以可以只看id
	if (!js.contains("res") || !js.contains("id"))
	{
		return;
	}
	// 处理逻辑
	int id_ = js["id"].get<int>();
	if (!newFriend.contains(id_) || allFriend.contains(id_))
	{
		return;
	}
	allFriend[id_] = newFriend[id_];
	newFriend.erase(id_);
	// 向下处理
	if (mainwindow)
	{
		mainwindow->addFriend(allFriend[id_]);
	}
}
void Kernel::solveSendMessage(json& js)
{
	if (!js.contains("id1") || !js.contains("id2") ||
		!js.contains("content") || !js.contains("time") || 
		!js.contains("msgId"))
	{
		return;
	}
	//处理逻辑
	// 说明成功发送了消息
	int id1 = js["id1"].get<int>(), id2 = js["id2"].get<int>();
	if (id1 != id && id2 != id)
	{
		return;
	}
	else
	{
		// flag 用于表示这条消息是否是我发的
		bool flag = (id1 == id);
		if (id2 == id)std::swap(id1, id2);
		if (!allFriend.contains(id2))
		{
			return;
		}
		Message msg;
		msg.setId(js["msgId"].get<long long>());
		msg.setInfor(js["content"].get<std::string>().data());
		msg.setType((flag ? MsgType::Send : MsgType::Reveive));
		msg.setTime(js["time"].get<std::string>().data());
		// 将这条消息添加到friend里面
		allFriend[id2]->tailAddMsg(msg);
		// 向下层传递消息
		if (mainwindow)
		{
			mainwindow->receiveFriendNewMsg(allFriend[id2], msg);
		}
	}
}
void Kernel::solveMoreFriendMsg(json& js)
{
	if (!js.contains("id") ||  !js.contains("isAll"))
	{
		return;
	}
	if (!js.contains("msgs")||!js["msgs"].is_array())
	{
		//处理逻辑
		return;
	}
	int friendid = js["id"].get<int>();
	// 外层做判断
	if (!allFriend.contains(friendid))
	{
		return;
	}
	// 先把isall处理了
	allFriend[friendid]->setIsAll(js["isAll"].get<bool>());
	for (auto& msg : js["msgs"])
	{
		if (!msg.contains("msgId") || !msg.contains("content") ||
			!msg.contains("time") || !msg.contains("msgType"))
		{
			continue;
		}
		//处理逻辑
		Message m(msg["time"].get<std::string>().data(), msg["content"].get<std::string>().data(),
			msg["msgType"].get<int>() ? MsgType::Send : MsgType::Reveive, msg["msgId"].get<long long>());
		allFriend[friendid]->headAddMsg(m);
		// 向下传递消息
		if (mainwindow)
		{
			mainwindow->receiveFriendOldMsg(allFriend[friendid], m);
		}
	}
}
void Kernel::solveFriendRequestInit(json& js)
{
	if (!js.contains("request") || !js["request"].is_array())
	{
		return;
	}
	for (auto& people : js["request"])
	{
		if (!people.contains("id") || !people.contains("name") || !people.contains("avatar"))
		{
			continue;
		}
		int id_ = people["id"].get<int>();
		//处理逻辑 需要去掉不需要的请求
		if (allFriend.contains(id_) || newFriend.contains(id_))
		{
			continue;
		}
		Friend* p = new Friend(id_,people["name"].get<std::string>().data(),
			updateAvatar(id_,people["avatar"].get<std::string>()).data());
		newFriend[id_] = p;
		//向下传递逻辑 这里可能需要处理数据的延时性 逻辑可能需要重新思量一下
		if (mainwindow)
		{
			mainwindow->solveRequestFriend(p);
		}
	}
}
void Kernel::solveFriendInit(json& js)
{
	if (!js.contains("friend")||!js["friend"].is_array())
	{
		return;
	}
	for (auto& people : js["friend"])
	{
		if (!people.contains("id") || !people.contains("avatar") ||
			!people.contains("noReadMsg") || !people.contains("name")||
			!people.contains("lastMsg")||!people.contains("lastTime"))
		{
			continue;
		}
		//处理逻辑
		int id_ = people["id"].get<int>();
		if (allFriend.contains(id_))
		{
			continue;
		}
		Friend* p = new Friend(id_, people["name"].get<std::string>().data(),
			updateAvatar(id_, people["avatar"].get<std::string>()).data(),people["noReadMsg"].get<int>());
		allFriend[id_] = p;
		if (mainwindow)
		{
			// 同时需要将最后一条消息的内容和事件告诉下层
			if (people["lastMsg"].get<std::string>() == "x")
			{
				mainwindow->addFriend(p);
			}
			else
			{
				mainwindow->addFriend(p, people["lastMsg"].get<std::string>().data(), people["lastTime"].get<std::string>().data());
			}
		}
	}
}

std::string Kernel::encode(std::string path)
{
	std::ifstream ifs(path, std::ios::in | std::ios::binary);
	if (!ifs.is_open())
	{
		return "";
	}
	std::string res{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
	return res;
}

std::string Kernel::base64_encode(const std::string& input)
{
	static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string output;
	output.reserve(((input.size() + 2) / 3) * 4);

	size_t i = 0;
	while (i < input.size())
	{
		uint32_t chunk = 0;
		int bytes = 0;
		for (; bytes < 3 && i + bytes < input.size(); ++bytes)
		{
			chunk = (chunk << 8) | static_cast<unsigned char>(input[i + bytes]);
		}
		chunk <<= (3 - bytes) * 8;

		output.push_back(alphabet[(chunk >> 18) & 0x3f]);
		output.push_back(alphabet[(chunk >> 12) & 0x3f]);
		output.push_back(bytes > 1 ? alphabet[(chunk >> 6) & 0x3f] : '=');
		output.push_back(bytes > 2 ? alphabet[chunk & 0x3f] : '=');
		i += bytes;
	}
	return output;
}

std::string Kernel::base64_decode(const std::string& input)
{
	static const std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::vector<int> values;
	values.reserve(input.size());
	for (char ch : input)
	{
		if (ch == '=')
		{
			break;
		}
		auto pos = alphabet.find(ch);
		if (pos != std::string::npos)
		{
			values.push_back(static_cast<int>(pos));
		}
	}

	std::string output;
	output.reserve((values.size() / 4) * 3);
	for (size_t i = 0; i + 3 < values.size(); i += 4)
	{
		int a = values[i];
		int b = values[i + 1];
		int c = values[i + 2];
		int d = values[i + 3];
		output.push_back(static_cast<char>((a << 2) | (b >> 4)));
		if (c < 64)
		{
			output.push_back(static_cast<char>(((b & 0x0f) << 4) | (c >> 2)));
		}
		if (d < 64)
		{
			output.push_back(static_cast<char>(((c & 0x03) << 6) | d));
		}
	}
	return output;
}

void Kernel::storageAvatar(std::string& path, std::string& image)
{
	std::ofstream ofs(path, std::ios::out | std::ios::binary);
	ofs.write(image.data(), image.size());
	// 写完关一下
	ofs.close();
}

std::string Kernel::updateAvatar(int id_,const std::string& s)
{
	//处理头像 需要将base64的解码使用
	std::string img = base64_decode(s);
	//先保存头像
	std::string path = "avatar/" + std::to_string(id_) + ".jpg";
	storageAvatar(path, img);
	return path;
}