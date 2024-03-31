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

size_t CentralCache::GetFreeList(void** batch,size_t size,size_t len)
{
    
    SpanList& list=_span_lists[SizeCalc::Index(size)];
    //注意锁的粒度，不需要全局锁
    std::lock_guard<std::mutex> guard(list._lock);

    size_t i=0;
    while(i==0)
    {
       Span* span=list.Begin();
        if(span==list.End() || span->freelist==nullptr)
            GetSpan(size,SizeCalc::Align(size*len,12)>>12);
        else
        {
            void* cur=span->freelist;
            for(;i<size && cur!=nullptr;++i)
            {
                ++span->use_counts;
                batch[i]=cur;
                cur=GetNextBlock(cur);
            } 

            span->freelist=cur;
            if(span->freelist==nullptr){
                list.Erase(span);
                list.PushBack(span);
            }
            break;
        } 
    }
    return i;
}

void CentralCache::RelFreeList(void** batch,size_t len, size_t idx)
{   
    SpanList& list=_span_lists[idx];
    std::lock_guard<std::mutex> guard(list._lock);
    //返回的空闲链表可能由多个span分配
    for(size_t i=0;i<len;++i)
    {
        void* cur=batch[i];
        Span* span=PageHeap::GetInstance().GetSpanFromBlock(cur);
        if(span)
        {
            --span->use_counts;
            SetNextBlock(cur,span->freelist);
            span->freelist=cur;
                
            //判断是否释放
            if(span->use_counts==0)
            {
                list.Erase(span);
                PageHeap::GetInstance().FreeSpan(span);
            }
        }
    }
}
}