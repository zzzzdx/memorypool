#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_cache.h"

using namespace memorypool;

GTEST_TEST(memory_pool,allocate)
{
    auto work=[](void** vec,int s)
    {
        ThreadCache& c=ThreadCache::GetInstance();
        for(int i=0;i<512;++i)
        {
            vec[i+s]=c.Allocate(8);
            *(int*)vec[i+s]=i+s;
        }
    };
    
    CentralCache& cc=CentralCache::GetInstance();
    void* vec[1024];
    std::thread t1(work,vec,0);
    std::thread t2(work,vec,512);
    t1.join();
    t2.join();

    for(int i=0;i<1024;++i)
    {
        EXPECT_EQ(*(int*)vec[i],i);
    }
}