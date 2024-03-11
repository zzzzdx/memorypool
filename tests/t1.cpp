#include "gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"

using namespace memorypool;
void run()
{
    ThreadCache& c=ThreadCache::GetInstance();
    printf("%d %p\n",std::this_thread::get_id(),&c);
}

GTEST_TEST(memory_pool,DISABLED_t_local)
{
    int num=2;
    std::vector<std::thread> tv;
    for(int i=0;i<num;++i)
    {
        std::thread t(&run);
        tv.push_back(std::move(t));
    }
    for(int i=0;i<num;++i)
        tv[i].join();
}