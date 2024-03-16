#include "central_cache.h"
#include "page_cache.h"

namespace memorypool
{


void CentralCache::GetSpan(size_t block_size, size_t page_num)
{
    SpanList& list=_span_lists[SizeCalc::Index(block_size)];
    Span* span=PageCache::GetInstance().GetSpan(page_num);
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
    list.PushFront(span);
}

CentralCache &CentralCache::GetInstance()
{
    static CentralCache central_cache;
    return central_cache;
}

size_t CentralCache::GetFreeList(void *&start, void *&end, size_t size, size_t len)
{
    std::lock_guard<std::mutex> guard(_lock);
    SpanList& list=_span_lists[SizeCalc::Index(size)];

    //获取空span，并构建空闲链表
    if(list.Size()==0)
        GetSpan(size,SizeCalc::Align(size*len,12)>>12);

    //保证空span永远在前,一个span用完才会取新的
    start=nullptr;
    size_t num=0;
    while(start==nullptr)
    {
        Span* span=list.Begin();
        if(span->freelist)
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
        else
            GetSpan(size,SizeCalc::Align(size*len,12)>>12);
    }
    return num;
}

void CentralCache::RelFreeList(void *start)
{
    std::lock_guard<std::mutex> guard(_lock);    
    //返回的空闲链表可能由多个span分配
    while(start)
    {
        void* next=GetNextBlock(start);
        Span* span=PageCache::GetInstance().GetSpanFromBlock(start);
        if(span)
        {
            --span->use_counts;
            SetNextBlock(start,span->freelist);
            span->freelist=start;

            //处理span位置，同时判断是否释放
            if(span->use_counts==0)
            {
                SpanList& list=_span_lists[SizeCalc::Index(span->block_size)];
                list.Erase(span);
                list.PushFront(span);
            }
        }
        start=next;
    }
}
}