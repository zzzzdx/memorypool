#include "central_cache.h"
#include "page_cache.h"

namespace memorypool
{


void CentralCache::GetSpan(size_t block_size, size_t page_num)
{
    SpanList& list=_list_container[SizeCalc::Index(block_size)];
    Span* span=PageCache::GetInstance().GetSpan(page_num);
    char* cur=(char*)(span->id<<12);
    char* end=(char*)((span->id+span->page_counts)<<12);
    end-=2*block_size;
    span->freelist=cur;

    while(cur<=end)
    {
        SetNextBlock(cur,cur+block_size);
        cur+=block_size;
    }
    SetNextBlock(cur,nullptr);
    list.PushFront(span);
}

CentralCache &CentralCache::GetInstance()
{
    static CentralCache central_cache;
    return central_cache;
}

void CentralCache::GetFreeList(void *&start, void *&end, size_t size, size_t len)
{
    SpanList& list=_list_container[SizeCalc::Index(size)];

    //获取空span，并构建空闲链表
    if(list.Size()==0)
        GetSpan(size,SizeCalc::RoundUp(size*len,12)>>12);

    start=nullptr;
    while(start==nullptr)
    {
        Span* span=list.Begin();
        if(span->freelist)
        {
            end=start=span->freelist;
            ++span->use_counts;

            while(GetNextBlock(end) && --len>0)
            {
                ++span->use_counts;
                end=GetNextBlock(end);
            }   

            span->freelist=GetNextBlock(end);
            SetNextBlock(end,nullptr);
            //判断是否用完
            break;
        }
        else
            GetSpan(size,SizeCalc::RoundUp(size*len,12)>>12);
    }
}
}