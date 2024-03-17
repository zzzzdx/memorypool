#pragma once
#include "common.h"
#include "thread_cache.h"
#include "page_heap.h"

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