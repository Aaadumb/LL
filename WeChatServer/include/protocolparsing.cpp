#include "protocolparsing.h"
#include <string>
void ProtocolParsing::solveMsg(std::string &s)
{
    p=0;
    while(p<s.size()){
        if(len==-1)
        {
            spliceHead(s);
            if((int)headString.size()==HEADLEN)
            {
                len=stoi(headString);
                headString="";
            }
        }
        if(len!=-1)
        {
            spliceBody(s);
            if((int)bodyString.size()==len)
            {
                msgs.emplace(std::move(bodyString));
                bodyString="";
                len=-1;
            }
        }
    }
}
std::string ProtocolParsing::getMsg()
{
    if(msgs.empty())
    {
        return "";
    }
    std::string msg=std::move(msgs.front());
    msgs.pop();
    return msg;
}
void packMsg(std::string &s)
{
    std::string head=std::to_string((int)s.size());
    int need=HEADLEN-head.size();
    if(need>0)
    {
        head.insert(0,std::string(need,'0'));
    }
    s.insert(0,head);
}

void ProtocolParsing::spliceHead(std::string&s)
{
    int need=HEADLEN-headString.size();
    if(need<=0)return;
    int flag;
    if(s.size()-p>=need)
    {
        flag=p+need;
    }
    else
    {
        flag=s.size();
    }
    for(p;p<flag;++p)
    {
        headString+=s[p];
    }
}

void ProtocolParsing::spliceBody(std::string&s)
{
    int need=len-bodyString.size();
    if(need<=0)return;
    int flag;
    if(s.size()-p>=need)
    {
        flag=p+need;
    }
    else
    {
        flag=s.size();
    }
    for(p;p<flag;++p)
    {
        bodyString+=s[p];
    }
}