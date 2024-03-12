#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_cache.h"

using namespace memorypool;

GTEST_TEST(memory_pool,ccahe_freelist)
{
    CentralCache& c=CentralCache::GetInstance();
    void* start;
    void* end;
    c.GetFreeList(start,end,1<<10,4);
    EXPECT_EQ(3,((char*)end-(char*)start)>>10);
    for(int i=0;i<3;++i)
        start=GetNextBlock(start);
    EXPECT_EQ(start,end);
}