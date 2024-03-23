#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_heap.h"
#include "memory_pool.h"
#include <array>
#include "page_map.h"

using namespace memorypool;

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
    EXPECT_EQ(ph.FreeAllSpans(),true);
    EXPECT_EQ(true,ph.FreeAllSample());
    printf("isfree %d\n",ph.FreeAllSpans());
    printf("use time %ld\n",end-start);
}

GTEST_TEST(memory_pool,m_deallocate)
{
    AllocateTest(5,{8,32,48},3);
}



