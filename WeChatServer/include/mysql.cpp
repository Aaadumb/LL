#include "mysql.h"
MySQL::MySQL(int mx)
{
    driver=sql::mysql::get_driver_instance();
    for(int i=0;i<mx;i++)
    {
        sql::Connection*con=
            driver->connect(PORT,USER,PASSWORD);
        con->setSchema(DATABASE);
        allcons.push_back(con);
        cons.push(con);
    }
}
MySQL::~MySQL()
{
    for(auto&con:allcons)
    {
        if(con)
        {
            con->close();
            delete con;
        }
    }
    allcons.clear();
}
MySQL& MySQL::get_MySQL_instance()
{
    static MySQL instance(20);
    return instance;
}

sql::Connection* MySQL::get_con_instance()
{
    std::unique_lock<std::mutex> lock(mtx);
    condition.wait(lock,[&](){
        return !cons.empty();
    });
    auto con=cons.front();
    cons.pop();
    return con;
}

void MySQL::return_con_instance(sql::Connection* con)
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        cons.push(con);
    }
    condition.notify_one();

}