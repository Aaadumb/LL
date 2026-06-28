#include "redis.h"

Redis::Redis(int mx)
{
    for(int i=0;i<mx;i++)
    {
        redisContext * con=redisConnect(IP,PORT);
        allcons.push_back(con);
        cons.push(con);
    }
}

Redis::~Redis()
{
    for(auto& con:allcons)
    {
        if(con)
        {
            redisFree(con);
        }
    }
    allcons.clear();
}

Redis& Redis::get_Redis_instance()
{
    static Redis instance(20);
    return instance;
}

void Redis::freeReply(redisReply* res)
{
    freeReplyObject(res);
}

redisReply* Redis::execute(redisContext* con,std::string com)
{
    return static_cast<redisReply*>(redisCommand(con,com.c_str()));
}

redisContext* Redis::get_con_instance()
{
    std::unique_lock<std::mutex> lock(mtx);
    condition.wait(lock,[&](){
        return !cons.empty();
    });
    auto res=cons.front();
    cons.pop();
    return res;
}

void Redis::return_con_instance(redisContext* con)
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        cons.push(con);
    }
    condition.notify_one();
}