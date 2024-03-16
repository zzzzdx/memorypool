#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_heap.h"

using namespace memorypool;

GTEST_TEST(memory_pool,deallocate)
{
    auto work=[]()
    {
        ThreadCache& c=ThreadCache::GetInstance();
        void** vec=(void**)c.Allocate(4096);
        for(int i=0;i<256;++i)
        {
            vec[i]=c.Allocate(8);
            *(int*)vec[i]=i;
        }
        for(int i=256;i<512;++i)
        {
            vec[i]=c.Allocate(16);
            *(int*)vec[i]=i;
        }
        
        for(int i=0;i<512;++i){
            EXPECT_EQ(i,*(int*)vec[i]);
            c.Deallocate(vec[i]);
        }
        c.Deallocate(vec);
        
    };
    
    CentralCache& cc=CentralCache::GetInstance();
    PageHeap& pc=PageHeap::GetInstance();
    std::thread t1(work);
    std::thread t2(work);
    std::thread t3(work);
    t1.join();
    t2.join();
    t3.join();
    cc.Debug();
    int a=1;
}

