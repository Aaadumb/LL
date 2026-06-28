#include <iostream>
#include <string>
#include <queue>
//主要是对主程序发送来的字符串进行处理 解决粘包等问题
//同时帮助主程序打包一下字符串
//整体就不采用单例模式了
//协议定义头的长度固定为20个字符 信息为具体的内容有多少个字节
#define HEADLEN 20
void packMsg(std::string &s);
class ProtocolParsing{
public:
    void solveMsg(std::string &s);
    std::string getMsg();
private:
    // msgs用于存储完整的消息
    std::queue<std::string> msgs;
    
    //以下都是处理半包的
    // headString用于处理头包
    std::string headString="";
    // len表示数据到底多长
    int len=-1;
    // bodyString 用于保存消息
    std::string bodyString="";
    // p 字符串指针 整体采用类似双指针的思想 提升效能
    int p=0;

    //拼接头部
    void spliceHead(std::string&s);
    //拼接内容
    void spliceBody(std::string&s);
};