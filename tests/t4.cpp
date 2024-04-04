#include<stdio.h>
#include<vector>
#include <mutex>
#include <functional>
#include <future>
#include <pthread.h>

class RWLockR
{
private:
    int _readers;
    pthread_mutex_t _wl;
    std::mutex _rl;

    
public:
    RWLockR():_readers(0),_wl(PTHREAD_MUTEX_INITIALIZER){}
    ~RWLockR(){pthread_mutex_destroy(&_wl);}
    void RLock() 
    {
        std::lock_guard<std::mutex> g(_rl);
        if (++_readers == 1)
            pthread_mutex_lock(&_wl);
    }
    void URLock()
    {
        std::lock_guard<std::mutex> g(_rl);
        if (--_readers==0)
            pthread_mutex_unlock(&_wl);
    }
    void WLock() { pthread_mutex_lock(&_wl); }
    void UWLock() { pthread_mutex_unlock(&_wl); }
};

int main()
{
    RWLockR lock;
    auto reader = [&lock]() {
        lock.RLock();
        printf("read\n");
        lock.URLock();
    };
    auto writer = [&lock]() {
        lock.WLock();
        printf("write\n");
        lock.UWLock();
    };

    std::vector<std::thread> tv;
    for (int i = 0; i < 4; ++i)
        tv.push_back(std::thread(writer));
    for (int i = 0; i < 4; ++i)
        tv.push_back(std::thread(reader));
    for (auto& i : tv)
        i.join();
 	return 0;
}