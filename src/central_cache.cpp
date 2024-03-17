#include "central_cache.h"
#include "page_heap.h"

namespace memorypool
{
    
CentralCache::~CentralCache()
{
    /*
    for(int i=0;i<FREELIST_COUNTS;++i)
    {
        if(_span_lists[i].Size())
        {
            for(Span* span=_span_lists[i].Begin();span!=_span_lists[i].End();)
            {
                Span* next=span->next;
                PageHeap::GetInstance().FreeSpan(span);
            }
                
        }
    }  
    */
}

void CentralCache::GetSpan(size_t block_size, size_t page_num)
{
    Span* span=PageHeap::GetInstance().GetSpan(page_num);
    span->block_size=block_size;
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

    SpanList& list=_span_lists[SizeCalc::Index(block_size)];
    list.PushFront(span);
}

CentralCache &CentralCache::GetInstance()
{
    static CentralCache central_cache;
    return central_cache;
}

size_t CentralCache::GetFreeList(void *&start, void *&end, size_t size, size_t len)
{
    
    SpanList& list=_span_lists[SizeCalc::Index(size)];
    //注意锁的粒度，不需要全局锁
    std::lock_guard<std::mutex> guard(list._lock);

    //保证空span永远在前,一个span用完才会取新的
    start=nullptr;
    size_t num=0;
    while(start==nullptr)
    {
        Span* span=list.Begin();
        if(span==list.End() || span->freelist==nullptr)
            GetSpan(size,SizeCalc::Align(size*len,12)>>12);
        else
        {
            end=start=span->freelist;
            ++span->use_counts;
            ++num;

            while(GetNextBlock(end) && --len>0)
            {
                ++span->use_counts;
                ++num;
                end=GetNextBlock(end);
            }   

            span->freelist=GetNextBlock(end);
            SetNextBlock(end,nullptr);

            if(span->freelist==nullptr)
            {
                list.Erase(span);
                list.PushBack(span);
            }
            break;
        }
    }
    return num;
}

void CentralCache::RelFreeList(void *start,size_t idx)
{   
    SpanList& list=_span_lists[idx];
    std::lock_guard<std::mutex> guard(list._lock);
    //返回的空闲链表可能由多个span分配
    while(start)
    {
        void* next=GetNextBlock(start);
        Span* span=PageHeap::GetInstance().GetSpanFromBlock(start);
        if(span)
        {
            --span->use_counts;
            SetNextBlock(start,span->freelist);
            span->freelist=start;
                
            //判断是否释放
            if(span->use_counts==0)
            {
                list.Erase(span);
                PageHeap::GetInstance().FreeSpan(span);
            }
        }
        start=next;
    }
}
}