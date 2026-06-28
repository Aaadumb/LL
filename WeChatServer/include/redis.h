#include <hiredis/hiredis.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#define IP "127.0.0.1"
#define PORT 6379
class Redis{
public:
    static Redis& get_Redis_instance();
    
    redisReply* execute(redisContext* con,std::string com);
    
    void freeReply(redisReply* res);
    
    void return_con_instance(redisContext* con);

    redisContext* get_con_instance();
private:
    Redis(int mx=20);
    ~Redis();


    std::vector<redisContext*> allcons;
    std::queue<redisContext*> cons;
    std::mutex mtx;
    std::condition_variable condition;
};