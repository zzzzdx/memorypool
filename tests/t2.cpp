#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_cache.h"

using namespace memorypool;

GTEST_TEST(memory_pool,pcache_getspan)
{
    PageCache& c=PageCache::GetInstance();
    Span* s1=c.GetSpan(1);
    Span* s2=c.GetSpan(2);
    Span* s3=c.GetSpan(3);
    Span* s4=c.GetSpan(4);
    EXPECT_EQ(s1->page_counts,1);
    EXPECT_EQ(s2->id,s1->id+s1->page_counts);
    EXPECT_EQ(s3->id,s2->id+s2->page_counts);
    EXPECT_EQ(s4->id,s3->id+s3->page_counts);
}