#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_cache.h"

using namespace memorypool;

GTEST_TEST(memory_pool,DISABLED_allocate)
{
    auto work=[](void** vec,int s)
    {
        ThreadCache& c=ThreadCache::GetInstance();
        for(int i=0;i<512;++i)
        {
            vec[i+s]=c.Allocate(8);
            *(int*)vec[i+s]=i+s;
        }
        for(int i=0;i<512;++i)
            c.Deallocate(vec[i+s]);
    };
    
    CentralCache& cc=CentralCache::GetInstance();
    void* vec[1024];
    std::thread t1(work,vec,0);
    std::thread t2(work,vec,512);
    t1.join();
    t2.join();
}

GTEST_TEST(memory_pool,deallocate)
{
    ThreadCache& c=ThreadCache::GetInstance();
    CentralCache& cc=CentralCache::GetInstance();
    void* vec[1536];
    for(int i=0;i<1536;++i)
    {
        vec[i]=c.Allocate(8);
    }
    for(int i=0;i<1536;++i)
        c.Deallocate(vec[i]);
}
