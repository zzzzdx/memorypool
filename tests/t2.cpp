#include <stdio.h>
#include <thread>
#include <vector>
#include "thread_cache.h"

using namespace memorypool;
void run()
{
    ThreadCache& c=ThreadCache::GetInstance();
    printf("%d %p\n",std::this_thread::get_id(),&c);
}

int main()
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
    return 0;
}