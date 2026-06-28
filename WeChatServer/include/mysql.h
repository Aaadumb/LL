#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <queue>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#define USER "root"
#define PASSWORD "105128Chen#"
#define DATABASE "mydatabase"
#define PORT "tcp://127.0.0.1:3306"
class MySQL{
public:
    static MySQL& get_MySQL_instance();
    sql::Connection* get_con_instance();
    void return_con_instance(sql::Connection* con);
private:
    MySQL(int mx);
    ~MySQL();
    std::vector<sql::Connection*> allcons;
    std::queue<sql::Connection*> cons;
    sql::mysql::MySQL_Driver*driver=nullptr;
    std::mutex mtx;
    std::condition_variable condition;
};