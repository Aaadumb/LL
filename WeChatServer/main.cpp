// 执行服务器的主函数 同时也是主线程
// 主要接收连接客户端 接收客户端发来的消息并处理
// 先不考虑粘包问题 先解决业务逻辑ProtocolParsing
#include <iostream>
#include "include/threadpool.hpp"
#include "include/mysql.h"
// redis 对于目前的项目规模来说不必要
//#include "include/redis.h"
// 引入粘包 半包处理函数
#include "include/protocolparsing.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <signal.h>
#include <string>
#include <utility>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using json=nlohmann::json;
using PreparedStatement=sql::PreparedStatement;
using ResultSet=sql::ResultSet;
#define SERVER_PORT 9527
#define DEFAULTAVATAR "./avatar/x.jpg"
struct Message{
    std::string content;
    bool allSend=true;
};
enum EventType{
    Login=0,
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


std::mutex mtx,onlineMtx;

ThreadPool& threadpool=ThreadPool::get_ThreadPool_instance();

MySQL& mysql=MySQL::get_MySQL_instance();

// Redis& redis=Redis::get_Redis_instance();

// 存储在线用户对应的fd
std::unordered_map<int,int> online;

// 存储fd对应的在线用户
std::unordered_map<int,int> ronline;

// 存储每一个fd对应的读缓冲区 buffer
std::unordered_map<int,ProtocolParsing> readbuffer;

// 存储每一个fd对应的写缓冲区
std::unordered_map<int,Message> sendbuffer;

// 用于子线程的生产队列
std::queue< std::pair<int,std::string> > msgs;

inline int getFd(int id);

inline int getIdByFd(int fd);

inline void clearFd(int client_fd);

inline void recordFd(int id,int client_fd);

inline std::string encode(std::string path);

inline void releaseStmt(sql::PreparedStatement* pstmt,sql::ResultSet* res);

inline void enqueueMsg(int client_fd,json &js);

inline std::string getAvatar(sql::Connection* con,int id);

inline void getLastMsg(sql::Connection*con,int id1,int id2,json&js);

inline int getNoSendMsg(sql::Connection* con,int id1,int id2);

inline void dealGiveFriendInitMsg(int client_fd,sql::Connection* con,int id);

inline void dealLogin(int client_fd,json js);

inline bool insertUser(sql::Connection* con,json& js);

inline int getIdByName(sql::Connection* con,const std::string &name);

inline std::string getNameById(sql::Connection*con,int id);

inline void dealRegister(int client_fd,json  js);

inline void dealSearchFriend(int client_fd,json js);

inline void dealMessage(int client_fd,std::string & content);

inline void setnonblock(int fd);

inline void dealRequestFriend(json js);

inline void dealAcceptFriend(int client_fd,json js);

inline bool checkIsFriend(sql::Connection* con,int id1,int id2);

inline void dealGiveFriendRequest(int client_fd,sql::Connection* con,int id);

inline void dealMoreFriendMsg(int client_fd,json js);

inline void storageAvatar(std::string& path,std::string& image);

inline void dealUploadAvatar(int client_fd,json js);

inline void dealSendMessage(int client_fd,json js);

inline void solveAccept(int epd,int fd);

inline void solveSendBuffer(int epd);

inline void solveReadBuffer(int epd,int client_fd);

inline void solveWrite(int epd,int client_fd);
int main()
{
    signal(SIGPIPE,SIG_IGN);
    // 配置服务端的套接字
    int fd=socket(AF_INET,SOCK_STREAM,0);
    setnonblock(fd);
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(SERVER_PORT);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    // 配置端口复用
    int opt=1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    bind(fd,(sockaddr*)&addr,sizeof(addr));
    listen(fd,128);

    // 创建并配置epoll
    epoll_event events[128],event;
    int epd=epoll_create(128);
    event.events=EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    event.data.fd=fd;
    epoll_ctl(epd,EPOLL_CTL_ADD,fd,&event);


    std::cout<<"服务器创建成功"<<std::endl;
    while(true)
    {
        // 每次监听两百毫秒 监听不到就跳过 顺便处理发送事件
        int n=epoll_wait(epd,&events[0],128,200);
 
        // 把子线程的消息全部加入到写缓冲区当中
        solveSendBuffer(epd);
        if(n<0)
        {
            //处理异常逻辑
            std::cout<<"服务端异常 即将中止进程"<<std::endl;
            break;
        }
        for(int i=0;i<n;i++)
        {
            int curfd=events[i].data.fd;
            if(curfd==fd)
            {
                // 有客户端接入
                solveAccept(epd,fd);
            }
            else
            {
                int client_fd=events[i].data.fd;
                if(events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                {
                    //std::cout<<"客户端异常"<<std::endl;
                    event.data.fd=client_fd;
                    epoll_ctl(epd,EPOLL_CTL_DEL,client_fd,&event);
                    close(client_fd);
                    clearFd(client_fd);
                    readbuffer.erase(client_fd);
                    sendbuffer.erase(client_fd);
                    continue;
                }
                if(events[i].events & EPOLLIN)
                {
                    // 有读事件
                    solveReadBuffer(epd,client_fd);
                }
                if(events[i].events & EPOLLOUT)
                {
                    // 可写
                    solveWrite(epd,client_fd);
                }
            }
        }
    }
    //处理程序结束逻辑
    /*
        close(fd);
    */
    return 0;
}

inline void clearFd(int client_fd)
{
    onlineMtx.lock();
    if(ronline.find(client_fd)==ronline.end())
    {
        onlineMtx.unlock();
        return;
    }
    online.erase(ronline[client_fd]);
    ronline.erase(client_fd);
    onlineMtx.unlock();
}

inline int getFd(int id)
{
    int ans=-1;
    onlineMtx.lock();
    if(online.find(id)!=online.end())
    {
        ans=online[id];
    }
    onlineMtx.unlock();
    return ans;
}

inline int getIdByFd(int fd)
{
    int ans=-1;
    onlineMtx.lock();
    if(ronline.find(fd)!=ronline.end())
    {
        ans=ronline[fd];
    }
    onlineMtx.unlock();
    return ans;
}

inline void recordFd(int id,int client_fd)
{
    onlineMtx.lock();
    online[id]=client_fd;
    ronline[client_fd]=id;
    onlineMtx.unlock();
}

inline std::string base64_encode(const std::string& input)
{
    static const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

inline std::string base64_decode(const std::string& input)
{
    static const std::string alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

inline std::string encode(std::string path)
{
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open())
    {
        return "";
    }
    std::string res{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    return res;
}
inline void releaseStmt(sql::PreparedStatement* pstmt,sql::ResultSet* res)
{
    delete pstmt;
    delete res;
}
inline void enqueueMsg(int client_fd,json& js)
{
    if(getIdByFd(client_fd)==-1)
    {
        //std::cout<<"未能正常发送"<<std::endl;
        return;
    }
    //std::cout<<"正常发送消息"<<std::endl;
    mtx.lock();
    std::string msg=js.dump();
    packMsg(msg);
    msgs.emplace(client_fd,msg);
    mtx.unlock();
}
inline std::string getAvatar(sql::Connection* con,int id)
{
    sql::PreparedStatement* pstmt=con->prepareStatement("select avatar from user where id=?;");
    pstmt->setInt(1,id);
    ResultSet* res=pstmt->executeQuery();
    std::string avatar="";
    if(res->next())
    {
        avatar=base64_encode(encode(std::string(res->getString("avatar"))));
    }
    releaseStmt(pstmt,res);
    return avatar;
}
inline void getLastMsg(sql::Connection*con,int id1,int id2,json&js)
{
    PreparedStatement* pstmt=
        con->prepareStatement("select content,time,id from message where (id1=? && id2=?) or (id1=? && id2=?) order by id desc limit 1;");
    pstmt->setInt(1,id1);
    pstmt->setInt(4,id1);
    pstmt->setInt(2,id2);
    pstmt->setInt(3,id2);
    ResultSet* res=pstmt->executeQuery();
    if(res->next())
    {
        js["lastMsg"]=res->getString("content");
        js["lastTime"]=res->getString("time");
    }
    else
    {
        std::string tmp="x";
        js["lastMsg"]=tmp;
        js["lastTime"]=tmp;
    }
    releaseStmt(pstmt,res);
}
inline int getNoSendMsg(sql::Connection* con,int id1,int id2)
{
    int ans=0;
    PreparedStatement* pstmt=
        con->prepareStatement("select count(*) from message where id1=? and id2=? && is_send=?;");
    pstmt->setInt(1,id1);
    pstmt->setInt(2,id2);
    pstmt->setBoolean(3,false);
    ResultSet* res=pstmt->executeQuery();
    if(res->next())
    {
        ans=res->getInt("count(*)");
    }
    releaseStmt(pstmt,res);
    //std::cout<<"ans:"<<ans<<std::endl;
    return ans;
}

inline std::string getNameById(sql::Connection*con,int id)
{
    sql::PreparedStatement* pstmt=con->prepareStatement("select name from user where id=?");
    pstmt->setInt(1,id);
    ResultSet* res=pstmt->executeQuery();
    std::string name="";
    if(res->next())
    {
        name=res->getString("name");
    }
    releaseStmt(pstmt,res);
    return name;
}

inline void dealGiveFriendRequest(int client_fd,sql::Connection* con,int id)
{
    /*
        //需要查询 请求人的id name 和avatar
        select u.id,u.name,u.avatar from user u join request_friend r on r.id1=u.id and r.id2=?; 
    */
    PreparedStatement* pstmt=
        con->prepareStatement("select u.id,u.name,u.avatar from user u join request_friend r on r.id1=u.id and r.id2=?;");
    pstmt->setInt(1,id);
    ResultSet* res=pstmt->executeQuery();
    if(res->next()){
        json back;
        back["kind"]=EventType::FriendRequestInit;
        do
        {
            json people;
            people["id"]=res->getInt("id");
            people["name"]=res->getString("name");
            people["avatar"]=base64_encode(encode(res->getString("avatar")));
            back["request"].push_back(people);
        }while(res->next());
        // 把消息发送出去
        enqueueMsg(client_fd,back);
    }
    releaseStmt(pstmt,res);
    // 由于表的协议 发送完之后就不存了
    pstmt=con->prepareStatement("delete from request_friend where id2=?");
    pstmt->setInt(1,id);
    pstmt->executeUpdate();
    releaseStmt(pstmt,nullptr);
}
inline void dealGiveFriendInitMsg(int client_fd,sql::Connection* con,int id)
{  
    // 首先把有关此人的朋友全部找出来
    json back;
    back["kind"]=EventType::FriendInit;
    PreparedStatement* pstmt=con->prepareStatement("select id1,id2 from friend_relation where id1=? or id2=?;");
    pstmt->setInt(1,id);
    pstmt->setInt(2,id);
    ResultSet* res=pstmt->executeQuery();
    if(!res->next())
    {
        releaseStmt(pstmt,res);
        return;
    }
    do
    {
        int id1=res->getInt("id1"),id2=res->getInt("id2");
        // 然后朋友的列表需要的信息有 朋友的唯一id 朋友的头像 朋友的未读消息数 跟朋友的最后一条消息
        json people;
        if(id2==id)std::swap(id1,id2);
        people["id"]=id2;
        people["avatar"]=getAvatar(con,id2);
        people["noReadMsg"]=getNoSendMsg(con,id2,id1);
        people["name"]=getNameById(con,id2);
        getLastMsg(con,id1,id2,people);
        // people["lastMsg"]=std::move(tmp.first);
        // people["lastTime"]=std::move(tmp.second);
        back["friend"].push_back(people);
    }while(res->next());
    releaseStmt(pstmt,res);
    // 找到之后 放到消息队列里面
    enqueueMsg(client_fd,back);
}
inline void dealLogin(int client_fd,json js)
{
    // 登录需要账户和密码 先做一层数据校验
    if(!js.contains("name") || !js.contains("pwd"))
    {
        return;
    }
    std::string name=js["name"].get<std::string>();
    std::string pwd=js["pwd"].get<std::string>();
    json back;
    back["kind"]=EventType::Login;
    sql::Connection* con=mysql.get_con_instance();
    PreparedStatement* pstmt=con->prepareStatement("select pwd,id,avatar from user where name=?;");
    pstmt->setString(1,name);
    sql::ResultSet *res=pstmt->executeQuery();
    bool flag=false;    //用于标记是否成功登录

    if(!res->next())
    {
        // 说明查无此人
        back["res"]=-1;
        back["reason"]="查无此人";
    }
    else
    {
        std::string pwd2=std::move(res->getString("pwd"));
        if(pwd2!=pwd)
        {
            // 密码校验失败
            back["res"]=-1;
            back["reason"]="密码校验失败";
        }
        else
        {
            // 成功登录 返回唯一id和头像信息
            flag=true;
            back["res"]=1;  //1表示成功 -1 表示失败
            back["id"]=res->getInt("id");
            // 因为头像处理的是地址 这里需要二进制转格式
            back["avatar"]=base64_encode(encode(std::string(res->getString("avatar"))));
            recordFd(back["id"].get<int>(),client_fd);
            // 先把这一条消息发给客户端
            enqueueMsg(client_fd,back);
            // 这里需要把朋友信息一并发给客户
            dealGiveFriendInitMsg(client_fd,con,res->getInt("id"));
            // 并且需要把离线的好友请求给客户
            dealGiveFriendRequest(client_fd,con,res->getInt("id"));
            
        }
    }
    if(!flag)
    {
        enqueueMsg(client_fd,back);
    }
    releaseStmt(pstmt,res);
    mysql.return_con_instance(con);
}
inline bool insertUser(sql::Connection* con,json& js)
{
    bool ans=true;
    PreparedStatement* pstmt=con->prepareStatement("insert into user(name,phone,pwd) value (?,?,?);");
    pstmt->setString(1,js["name"].get<std::string>());
    pstmt->setString(2,js["phone"].get<std::string>());
    pstmt->setString(3,js["pwd"].get<std::string>());
    try{
        pstmt->executeUpdate();
    }
    catch(sql::SQLException&e)
    {
        std::cout<<e.what()<<std::endl;
        ans=false;
    }
    delete pstmt;
    return ans;
}
inline int getIdByName(sql::Connection* con,const std::string &name)
{
    sql::PreparedStatement* pstmt=con->prepareStatement("select id from user where name=?;");
    pstmt->setString(1,name);
    ResultSet* res=pstmt->executeQuery();
    int ans=-1;
    if(res->next())
    {
        ans=res->getInt("id");
    }
    releaseStmt(pstmt,res);
    return ans;
}
inline void dealRegister(int client_fd,json js)
{
    // 注册需要name phone pwd缺一不可
    if(!js.contains("name") || !js.contains("phone") || !js.contains("pwd"))
    {
        return;
    }
    if(getIdByFd(client_fd)!=-1)
    {
        json back;
        back["kind"]=EventType::Register;
        back["res"]=-1;
        back["reason"]="已登录账号不允许再次注册";
        enqueueMsg(client_fd,back);
        return;
    }
    json back;
    sql::Connection* con=mysql.get_con_instance();
    back["kind"]=EventType::Register;
    // 这里只需要判断是否有相同的name 或者电话号码 即可 如果有 返回名称已存在
    // 往用户表里填入数据
    bool ans=insertUser(con,js);
    if(!ans)
    {
        //当前名称已经被占用
        back["res"]=-1;
        back["reason"]="当前名称或号码已被占用";
    }
    else
    {
        back["res"]=1;
        // 这里传给用户默认头像
        back["avatar"]=base64_encode(encode(DEFAULTAVATAR));
        back["id"]=getIdByName(con,js["name"].get<std::string>());
        // 注册成功记录一下状态
        recordFd(back["id"].get<int>(),client_fd);
    }
    enqueueMsg(client_fd,back);
    mysql.return_con_instance(con);
}
inline void dealSearchFriend(int client_fd,json js)
{
    //优先做数据校验 需要对方的名字或者电话号码
    if(!js.contains("according"))
    { 
        // 这里这个according表示客户端输入的内容 可能是name 也可能是phone 反正必须要有
        return;
    }
    json back;
    sql::Connection* con=mysql.get_con_instance();
    back["kind"]=EventType::SearchFriend;
    // 查询需要得到id 名称 头像 信息
    sql::PreparedStatement* pstmt=con->prepareStatement("select id,name,avatar from user where name=? or phone=?;");
    pstmt->setString(1,js["according"].get<std::string>());
    pstmt->setString(2,js["according"].get<std::string>());
    ResultSet* res=pstmt->executeQuery();
    // 可能有傻逼名字跟电话号码一样 保险起见 全部都传给用户
    while(res->next())
    {
        json people;
        people["id"]=res->getInt("id");
        people["name"]=res->getString("name");
        people["avatar"]=base64_encode(encode(std::string(res->getString("avatar"))));
        back["friend"].push_back(people);
    }
    releaseStmt(pstmt,res);
    enqueueMsg(client_fd,back);
    mysql.return_con_instance(con);
}

inline void dealRequestFriend(json js)
{
    // 首先需要得到对方的id 然后如果当前朋友在线 跟对面发送一下信息 朋友请求表默认是发送完之后就删除的
    if(!js.contains("id1") || !js.contains("id2"))
    {
        return;
    }
    // 先做一层数据校验 如果两个人已经是好友 就不继续往下了
    sql::Connection*con=mysql.get_con_instance();
    int id1=js["id1"].get<int>(),id2=js["id2"].get<int>();
    if(id1==id2 || checkIsFriend(con,id1,id2))
    {
        mysql.return_con_instance(con);
        return;
    }
    int fd=getFd(id2);
    if(fd==-1)
    {
        //说明当前不在线 记录到数据库中
        PreparedStatement* pstmt=con->prepareStatement("insert into request_friend(id1,id2) value(?,?)");
        pstmt->setInt(1,id1);
        pstmt->setInt(2,id2);
        try
        {
            pstmt->executeUpdate();
        }
        catch(sql::SQLException&e)
        {
            std::cout<<e.what()<<std::endl;
        }
        releaseStmt(pstmt,nullptr);
    }
    else
    {
        //在线 发送即可
        json back;
        back["kind"]=EventType::RequestFriend;
        back["id"]=id1;
        back["name"]=getNameById(con,id1);
        back["avatar"]=getAvatar(con,id1);
        enqueueMsg(fd,back);
    }
    mysql.return_con_instance(con);
}

inline bool checkIsFriend(sql::Connection* con,int id1,int id2)
{
    // 表中默认前小后大
    if(id2<id1)std::swap(id1,id2);
    sql::PreparedStatement* pstmt=con->prepareStatement("select count(*) from friend_relation where (id1=? and id2=?)");
    pstmt->setInt(1,id1);
    pstmt->setInt(2,id2);
    ResultSet* res=pstmt->executeQuery();
    bool ans=false;
    if(res->next())
    {
        ans=(res->getInt(1)>0);
    };
    releaseStmt(pstmt,res);
    return ans;
}

inline void dealAcceptFriend(int client_fd,json js)
{
    // 做一下数据校验 需要接受方 和 申请方的id 成功的话 告诉客户端跟哪个用户成功建立了朋友关系即可
    if(!js.contains("id1") || !js.contains("id2") )
    {
        return;
    }
    sql::Connection* con=mysql.get_con_instance();
    // 先做一层数据校验 如果两个人已经是好友 就不继续往下了
    int id1=js["id1"].get<int>(),id2=js["id2"].get<int>();
    if(id1==id2 || checkIsFriend(con,id1,id2))
    {
        mysql.return_con_instance(con);
        return;
    }
    json back;
    back["kind"]=EventType::AcceptFriend;
    sql::PreparedStatement *pstmt=con->prepareStatement("insert into friend_relation(id1,id2) value(?,?)");
    pstmt->setInt(1,std::min(id1,id2));
    pstmt->setInt(2,std::max(id1,id2));
    try{
        pstmt->executeUpdate();
    }
    catch(sql::SQLException&e)
    {
        std::cout<<e.what()<<std::endl;
        delete pstmt;
        mysql.return_con_instance(con);
        return;
    }
    back["res"]=1;
    back["id"]=js["id2"].get<int>();
    enqueueMsg(client_fd,back);
    back["id"]=js["id1"].get<int>();
    int fd=getFd(id2);
    if(fd!=-1)
    {
        enqueueMsg(fd,back);
    }
    delete pstmt;
    mysql.return_con_instance(con);
}

inline void dealSendMessage(int client_fd,json js)
{
    // 发送的话 需要知道自己和对方的id以及发送的内容 发送的时间 缺一不可 同时需要判断当前用户是否在线
    if(!js.contains("id1") || !js.contains("id2") || !js.contains("content") ||!js.contains("time"))
    {
        return;
    }
    sql::Connection* con=mysql.get_con_instance();
    int id1=js["id1"].get<int>();
    int id2=js["id2"].get<int>();
    std::string content=std::move(js["content"].get<std::string>());
    std::string time=std::move(js["time"].get<std::string>());
    int fd=getFd(id2);
    bool is_send=(fd!=-1);
    // 将消息记录到数据库中
    sql::PreparedStatement* pstmt=con->prepareStatement("insert into message(id1,id2,content,time,is_send) value(?,?,?,?,?)");
    pstmt->setInt(1,id1);
    pstmt->setInt(2,id2);
    pstmt->setString(3,content);
    pstmt->setString(4,time);
    pstmt->setBoolean(5,is_send);
    pstmt->executeUpdate();
    delete pstmt;
    pstmt=con->prepareStatement("select last_insert_id()");
    sql::ResultSet* res=pstmt->executeQuery();
    long long msgId=-1;
    if(res->next())
    {
        msgId=res->getInt64("last_insert_id()");
    }
    json back;
    back["kind"]=EventType::SendMessage;
    back["id1"]=id1;
    back["id2"]=id2;
    back["content"]=content;
    back["time"]=time;
    back["msgId"]=msgId;
    if(fd!=-1)
    {
        enqueueMsg(fd,back);
    }
    enqueueMsg(client_fd,back);
    releaseStmt(pstmt,res);
    mysql.return_con_instance(con);
}

inline void dealMoreFriendMsg(int client_fd,json js)
{
    // 首先需要知道 找谁和谁的聊天记录 找多久之前的
    if(!js.contains("id1") || !js.contains("id2") || !js.contains("msgId"))
    {
        return;
    }
    sql::Connection* con=mysql.get_con_instance();
    int id1=js["id1"].get<int>(),id2=js["id2"].get<int>();
    long long msgId=js["msgId"].get<long long>();
    PreparedStatement* pstmt=
        con->prepareStatement("select id,id1,content,time from message where id<? and((id1=? && id2=?)or(id1=? && id2=?)) order by id desc limit 20");
    pstmt->setInt64(1,msgId);
    pstmt->setInt(2,id1);
    pstmt->setInt(3,id2);
    pstmt->setInt(4,id2);
    pstmt->setInt(5,id1);
    ResultSet* res=pstmt->executeQuery();
    json back;
    int cnt=0;
    long long mnMsgId=-1;
    back["kind"]=EventType::MoreFriendMsg;
    back["id"]=id2;
    while(res->next())
    {
        cnt++;
        mnMsgId=res->getInt64("id");
        json msg;
        msg["msgId"]=res->getInt64("id");
        msg["content"]=res->getString("content");
        msg["time"]=res->getString("time");
        msg["msgType"]=(res->getInt("id1")==id1)?1:0;
        back["msgs"].push_back(msg);
    }
    back["isAll"]=(cnt<20);
    enqueueMsg(client_fd,back);
    releaseStmt(pstmt,res);
    // 同时把is_send设置为true
    if(cnt>0)
    {
        pstmt=
            con->prepareStatement("update message set is_send=true where (id>=? and id<?) and((id1=? && id2=?)or(id1=? && id2=?))");
        pstmt->setInt64(1,mnMsgId);
        pstmt->setInt64(2,msgId);
        pstmt->setInt(3,id1);
        pstmt->setInt(4,id2);
        pstmt->setInt(5,id2);
        pstmt->setInt(6,id1);
        pstmt->executeUpdate();
        delete pstmt;
    }
    mysql.return_con_instance(con);
}

inline void storageAvatar(std::string& path,std::string& image)
{
    std::ofstream ofs(path,std::ios::out | std::ios::binary);
    ofs.write(image.data(),image.size());
    // 写完关一下
    ofs.close();
}

inline void dealUploadAvatar(int client_fd,json js)
{
    // 需要知道你更新的是谁的头像 也就是id 然后是头像的base64_encode 这里需要解码
    if(!js.contains("id") || !js.contains("image"))
    {
        return;
    }
    sql::Connection* con=mysql.get_con_instance();
    // 将图片检查并存储到存储目录下方
    std::string path="./avatar/"+std::to_string(js["id"].get<int>())+".jpg";
    std::string image=base64_decode(js["image"].get<std::string>());
    storageAvatar(path,image);
    // 更新数据库
    sql::PreparedStatement* pstmt=con->prepareStatement("update user set avatar=? where id=?");
    pstmt->setString(1,path);
    pstmt->setInt(2,js["id"].get<int>());
    pstmt->executeUpdate();
    // 更新之后返回给客户端
    js["res"]=1;
    enqueueMsg(client_fd,js);
    delete pstmt;
    mysql.return_con_instance(con);
}

inline void dealMessage(int client_fd,std::string& content)
{
    json js;
    try
    {
        js=json::parse(content);
    }
    catch(const json::exception&e)
    { 
        // catch(...) 表示捕获任何异常
        std::cout<<"json error: "<<e.what()<<std::endl;
        return;
    }
    if(!js.contains("kind"))
    {
        return;
    }
    int kind=js["kind"].get<int>();
    switch(kind)
    {
        case EventType::Login:
            threadpool.enqueue_task(dealLogin,client_fd,js);
            break;
        case EventType::Register:
            threadpool.enqueue_task(dealRegister,client_fd,js);
            break;
        case EventType::UploadAvatar:
            threadpool.enqueue_task(dealUploadAvatar,client_fd,js);
            break;
        case EventType::SearchFriend:
            threadpool.enqueue_task(dealSearchFriend,client_fd,js);
            break;
        case EventType::RequestFriend:
            threadpool.enqueue_task(dealRequestFriend,js);
            break;
        case EventType::AcceptFriend:
            threadpool.enqueue_task(dealAcceptFriend,client_fd,js);
            break;
        case EventType::SendMessage:
            threadpool.enqueue_task(dealSendMessage,client_fd,js);
            break;
        case EventType::MoreFriendMsg:
            threadpool.enqueue_task(dealMoreFriendMsg,client_fd,js);
            break;
        default:
            break;
    }
}
inline void setnonblock(int fd)
{
    int flag=fcntl(fd,F_GETFL);
    flag|=O_NONBLOCK;
    fcntl(fd,F_SETFL,flag);
}

inline void solveAccept(int epd,int fd)
{   
    sockaddr_in client_addr;
    socklen_t client_addr_len;
    epoll_event event;
    while(true)
    {
        client_addr_len=sizeof(client_addr);
        int client_fd=accept(fd,(sockaddr*)&client_addr,&client_addr_len);
        if(client_fd<=0)
        {
            break;
        }
        std::cout<<"有客户端接入"<<std::endl;
        std::cout<<"IP:"<<inet_ntoa(client_addr.sin_addr)<<std::endl;
        // 处理接入之后的逻辑
        setnonblock(client_fd);
        event.events=EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
        event.data.fd=client_fd;
        epoll_ctl(epd,EPOLL_CTL_ADD,client_fd,&event);
    }
}

inline void solveSendBuffer(int epd)
{
    epoll_event event;
    while(true)
    {

        mtx.lock();
        if(msgs.empty())
        {
            mtx.unlock();
            break;
        }
        auto msg=std::move(msgs.front());
        msgs.pop();
        mtx.unlock();

        //将消息加入到对应的缓冲区当中
        int client_fd=msg.first;
        if(getIdByFd(client_fd)==-1)
        {
            continue;
        }
        sendbuffer[client_fd].content += msg.second;
        if(sendbuffer[client_fd].allSend)
        {
            sendbuffer[client_fd].allSend=false;
            event.events=EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
            event.data.fd=client_fd;
            epoll_ctl(epd,EPOLL_CTL_MOD,client_fd,&event);
        }
    }
}

inline void solveReadBuffer(int epd,int client_fd)
{
    // if(getIdByFd(client_fd)==-1)
    // {
    //     std::cout<<"读异常"<<std::endl;
    //     return;
    // }
    // 服务端有读请求
    std::string s(BUFSIZ,' ');
    epoll_event event;
    bool error=false;
    while(true)
    {
        s.resize(BUFSIZ,' ');
        int len=read(client_fd,&s[0],s.size());
        if(len<=0)
        {
            if(len<0)
            {
                if(errno==EAGAIN || errno==EWOULDBLOCK)
                {
                    break;
                }
            }
            //处理异常逻辑 这里客户端直接断掉
            event.data.fd=client_fd;
            epoll_ctl(epd,EPOLL_CTL_DEL,client_fd,&event);
            close(client_fd);
            clearFd(client_fd);
            sendbuffer.erase(client_fd);
            error=true;
            break;
        }   
        else
        {
            s.resize(len);
            // 把内容交给粘包处理类 让它去处理粘包
            readbuffer[client_fd].solveMsg(s);
        }
    }
    // 然后从粘包处理类中拿信息
    while(true)
    {
        std::string msg=readbuffer[client_fd].getMsg();
        if(msg=="")
        {
            break;
        }
        // 交给线程池去处理事件
        dealMessage(client_fd,msg);
    }
    if(error)
    {
        readbuffer.erase(client_fd);
    }
}

inline void solveWrite(int epd,int client_fd)
{
    // 因为在项目中 只有有消息的才设置发送监听 保证了一定有数据可发送
    // 把消息发送给客户端
    if(getIdByFd(client_fd)==-1)
    {
        //std::cout<<"写 未发现对象"<<std::endl;
        return;
    }
    auto &buffer=sendbuffer[client_fd];
    // std::cout<<"向客户端发送数据"<<std::endl;
    // std::cout<<buffer.content<<std::endl;
    int p=0,len=buffer.content.size();
    epoll_event event;
    bool error=false;
    while(p<len)
    {
        int res=write(client_fd,&buffer.content[p],len-p);
        if(res==0)
        {
            //说明已经读完了
            p=len;
            break;
        }
        if(res<0)
        {
            if(errno!=EAGAIN && errno!=EWOULDBLOCK )
            {
                // 对方连接关闭了
                event.data.fd=client_fd;
                epoll_ctl(epd,EPOLL_CTL_DEL,client_fd,&event);
                close(client_fd);
                clearFd(client_fd);
                readbuffer.erase(client_fd);
                sendbuffer.erase(client_fd);
                error=true;
                //std::cout<<"写异常"<<std::endl;
            }
            break;
        }
        p+=res;
    }
    if(!error)
    {
        if(p>=len)
        {
            //消息发完了
            buffer.content="";
            buffer.allSend=true;
            event.data.fd=client_fd;
            event.events=EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP;
            epoll_ctl(epd,EPOLL_CTL_MOD,client_fd,&event);
            //std::cout<<"成功发送"<<std::endl;
        }
        else
        {
            //消息没发完 等待对方缓冲区可读
            buffer.content=buffer.content.substr(p,len-p);
            buffer.allSend=false;
        }
    }
}
