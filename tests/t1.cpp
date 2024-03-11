#include "gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"

using namespace memorypool;

GTEST_TEST(memory_pool,DISABLED_t_local)
{
    auto run=[]()
    {
        ThreadCache& c=ThreadCache::GetInstance();
        printf("%d %p\n",std::this_thread::get_id(),&c);
    };
    int num=2;
    std::vector<std::thread> tv;
    for(int i=0;i<num;++i)
    {
        std::thread t(run);
        tv.push_back(std::move(t));
    }
    for(int i=0;i<num;++i)
        tv[i].join();
}

GTEST_TEST(memory_pool,cent_static)
{
    auto run=[](){
        CentralCache& c=CentralCache::GetInstance();
        printf("%p\n",&c);
    };
    int num=2;
    std::vector<std::thread> tv;
    for(int i=0;i<num;++i)
    {
        std::thread t(run);
        tv.push_back(std::move(t));
    }
    for(int i=0;i<num;++i)
        tv[i].join();
}