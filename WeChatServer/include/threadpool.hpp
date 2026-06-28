// 先把线程池写出来
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <utility>
class ThreadPool{
public:
    static ThreadPool& get_ThreadPool_instance()
    {
        static ThreadPool threadpool(15);
        return threadpool;
    }
    template<class T,class... Args>
    void enqueue_task(T&& f,Args&& ... args)
    {
        auto task = std::bind(std::forward<T>(f), std::forward<Args>(args)...);

        mtx.lock();
        tasks.emplace([task = std::move(task)]() mutable {
            task();
        });
        mtx.unlock();
        condition.notify_one();
    }
private:
    ThreadPool(int mx=15)
    {
        for(int i=0;i<mx;i++)
        {
            threads.emplace_back([&](){
                while(true)
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock,[&](){
                        return !(!stop&&tasks.empty());
                    });
                    if(stop && tasks.empty())
                    {
                        return;
                    }
                    auto task=std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    };
    ~ThreadPool()
    {
        mtx.lock();
        stop=true;
        mtx.unlock();
        condition.notify_all();
        for(auto& t:threads)
        {
            t.join();
        }
    }
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::condition_variable condition;
    std::mutex mtx;
    bool stop=false;
};