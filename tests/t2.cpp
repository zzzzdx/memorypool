#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_heap.h"
#include "memory_pool.h"

using namespace memorypool;

GTEST_TEST(memory_pool,DISABLED_deallocate)
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
            int size=8;
            if(i>=256)
                size=16;
            c.Deallocate(vec[i],size);
        }
        c.Deallocate(vec,4096);
        
    };
    
    CentralCache& cc=CentralCache::GetInstance();
    PageHeap& pc=PageHeap::GetInstance();
    std::thread t1(work);
    std::thread t2(work);
    std::thread t3(work);
    std::thread t4(work);
    std::thread t5(work);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    EXPECT_EQ(true,pc.FreeAllSpans());
}

void AllocateTest(int works,std::vector<int> sizes,int rounds)
{
    printf("allocate\n");
    clock_t start,end;
    start=clock();
    for(int r=0;r<rounds;++r)
    {
        std::vector<std::thread> threads;
        threads.reserve(works);
        auto work=[](std::vector<int> sizes){
            void** ps=(void**)Allocate(sizes.size()*206*8);
            int idx=0;
            for(auto s : sizes)
            {
                for(int i=0;i<206;++i)
                {
                    void* p=Allocate(s);
                    *(int*)p=15;
                    ps[idx++]=p;
                }
            }

            idx=0;
            for(auto s : sizes)
            {
                for(int i=0;i<206;++i)
                {
                    Deallocate(ps[idx++]);
                }
            }
            Deallocate(ps);
        };

        for(int i=0;i<works;++i)
            threads.push_back(std::thread(work,sizes));
        for(auto& t :threads)
            t.join();
    }
    end=clock();
    PageHeap& ph=PageHeap::GetInstance();
    printf("isfree %d\n",ph.FreeAllSpans());
    printf("use time %ld\n",end-start);
}

GTEST_TEST(memory_pool,m_deallocate)
{
    AllocateTest(5,{8,32,48},3);
}

