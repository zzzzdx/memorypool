#pragma once
#include "common.h"
#include "thread_cache.h"
#include "page_heap.h"

//内存池的使用
//1.替代malloc，free族
//2.构建allocator用于容器
//3.重载operator new针对某类型

namespace memorypool
{

inline void* Allocate(size_t size)
{
    if(size<BIG_OBJ_SIZE)
        return ThreadCache::GetInstance().Allocate(size);
    else
        return PageHeap::GetInstance().GetBigObj(size);
}

inline void Deallocate(void* p)
{
    Span* span=PageHeap::GetInstance().GetSpanFromBlock(p);
    if(!span)
        return;
    
    if(span->block_size<BIG_OBJ_SIZE)
        ThreadCache::GetInstance().Deallocate(p,span->block_size);
    else
        PageHeap::GetInstance().FreeBigObj(p);
}

}