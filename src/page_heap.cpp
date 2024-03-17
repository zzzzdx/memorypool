#include "common.h"
#include "page_heap.h"
#include <assert.h>
#include <mutex>
#include <set>

using namespace memorypool;

namespace memorypool
{


void *PageHeap::GetPages(int page_num){
    void* p=mmap(nullptr,4096*page_num,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(p==nullptr)
        throw "mmap fail";
    ++_total_get;
    return p;
}

void PageHeap::FreePages(void *origin_page_ptr,size_t size){
    munmap(origin_page_ptr,size);
}

Span *PageHeap::GetSpanNoLock(size_t page_num)
{
    if(page_num>MAX_SPAN_SIZE)
        page_num=MAX_SPAN_SIZE;
    for(int i=page_num-1;i<MAX_SPAN_SIZE;++i)
    {
        SpanList& list=_span_lists[i];
        if(list.Size()>0)
        {
            Span* ret=list.Begin();
            list.Erase(ret);
            if(page_num<ret->page_counts)
            {
                Span* left=ret;

                ret=new Span;
                ret->id=left->id;
                ret->start=left->start;
                ret->page_counts=page_num;
                for(PageId i=ret->id;i<ret->page_counts+ret->id;++i)
                    _id_span_map[i]=ret;

                left->id+=page_num;
                left->start=false;
                left->page_counts-=page_num;
                _span_lists[left->page_counts-1].PushBack(left);
            }
            //Logger::GetInstance()->debug("GetSpanNoLock give {} pages",ret->page_counts);
            ret->used=true;
            return ret;
        }
    }

    void* pages=GetPages(MAX_SPAN_SIZE);
    //Logger::GetInstance()->debug("GetSpanNoLock get {} pages",MAX_SPAN_SIZE);
    Span* span=new Span;
    span->start=true;
    span->id=(PageId)pages>>12;
    span->page_counts=MAX_SPAN_SIZE;
    _span_lists[MAX_SPAN_SIZE-1].PushBack(span);
    for(PageId i=span->id;i<span->page_counts+span->id;++i)
        _id_span_map[i]=span;
    return GetSpanNoLock(page_num);
}

PageHeap &PageHeap::GetInstance()
{
    static PageHeap page_cache;
    return page_cache;
}

Span *PageHeap::GetBigObj(int size)
{
    assert(size>=BIG_OBJ_SIZE);
    int page_num=SizeCalc::Align(size,12)>>12;
    void* p=GetPages(page_num);
    Span* span = new Span;
    span->id=(PageId)p>>12;
    span->page_counts=page_num;

    std::lock_guard<std::shared_mutex> guard(_lock);
    _id_span_map.insert(std::pair<PageId,Span*>(span->id,span));
    return nullptr;
}
void PageHeap::FreeBigObj(void *ptr)
{
    PageId id=(PageId)ptr>>12;
    size_t size;

    {
        std::lock_guard<std::shared_mutex> guard(_lock);
        auto itr=_id_span_map.find(id);
        if(itr==_id_span_map.end())
            return;
        size=itr->second->page_counts<<12;
        _id_span_map.erase(itr);
    }
    
    FreePages(ptr,size);
}

Span *PageHeap::GetSpan(size_t page_num)
{
    std::lock_guard<std::shared_mutex> guard(_lock);
    return GetSpanNoLock(page_num);
}

void PageHeap::FreeSpan(Span *span)
{
    //Logger::GetInstance()->debug("FreeSpan free {} pages",span->page_counts);
    
    //每个空span都会尝试合并，只需前后合并一次即可
    //向前合并
    std::lock_guard<std::shared_mutex> guard(_lock);
    span->used=false;
    if(!span->start)
    {
        PageId prev_id=span->id-1;
        auto itr=_id_span_map.find(prev_id);
        if(itr!=_id_span_map.end() && itr->second->used==false)
        {
            Span* prev_span=itr->second;
            SpanList& list=_span_lists[prev_span->page_counts-1];
            list.Erase(prev_span);

            span->id=prev_span->id;
            span->page_counts+=prev_span->page_counts;
            span->start=prev_span->start;
            for(PageId i=prev_span->id;i<prev_span->page_counts+prev_span->id;++i)
                _id_span_map[i]=span;
            delete prev_span;
        }
    }

    //向后合并
    if(!(span->start && span->page_counts==MAX_SPAN_SIZE))
    {
        PageId next_id=span->id+span->page_counts;
        auto itr=_id_span_map.find(next_id);
        if(itr!=_id_span_map.end() && itr->second->used==false && itr->second->start==false)
        {
            Span* next_span=itr->second;
            SpanList& list=_span_lists[next_span->page_counts-1];
            list.Erase(next_span);

            span->page_counts+=next_span->page_counts;
            for(PageId i=next_span->id;i<next_span->id+next_span->page_counts;++i)
                _id_span_map[i]=span;
            delete next_span;
        }
    }
    
    SpanList& list=_span_lists[span->page_counts-1];
    list.PushBack(span);
    
}

Span *PageHeap::GetSpanFromBlock(void *block)
{
    std::shared_lock<std::shared_mutex> guard(_lock);
    return _id_span_map[(PageId)block>>12];
}
bool PageHeap::FreeAllSpans()
{
    for(int i=0;i<MAX_SPAN_SIZE-1;++i)
    {
        if(_span_lists[i].Size()!=0)
            return false;
    }
    if(_span_lists[MAX_SPAN_SIZE-1].Size()==_total_get)
        return true;
    else
        return false;
}
}