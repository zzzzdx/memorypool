#include "common.h"
#include "page_heap.h"
#include <assert.h>
#include <mutex>
#include <set>

using namespace memorypool;

namespace memorypool
{



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

                ret=_span_alloc.Allocate();
                ret->id=left->id;
                ret->start=left->start;
                ret->page_counts=page_num;
                for(PageId i=ret->id;i<ret->page_counts+ret->id;++i)
                    _id_span_map.insert(i,ret);

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

    void* pages=Mmap(MAX_SPAN_SIZE);
    ++_total_get;
    //Logger::GetInstance()->debug("GetSpanNoLock get {} pages",MAX_SPAN_SIZE);
    Span* span=_span_alloc.Allocate();
    span->start=true;
    span->id=(PageId)pages>>12;
    span->page_counts=MAX_SPAN_SIZE;
    _span_lists[MAX_SPAN_SIZE-1].PushBack(span);
    for(PageId i=span->id;i<span->page_counts+span->id;++i)
        _id_span_map.insert(i,span);
    return GetSpanNoLock(page_num);
}

PageHeap &PageHeap::GetInstance()
{
    static PageHeap page_cache;
    return page_cache;
}

void *PageHeap::GetBigObj(int size)
{
    assert(size>=BIG_OBJ_SIZE);
    int page_num=SizeCalc::Align(size,12)>>12;
    void* p=Mmap(page_num);

    std::lock_guard<std::shared_mutex> guard(_lock);
    Span* span = _span_alloc.Allocate();
    span->id=(PageId)p>>12;
    span->page_counts=page_num;
    _id_span_map.insert(span->id,span);
    return p;
}
void PageHeap::FreeBigObj(void *ptr)
{
    PageId id=(PageId)ptr>>12;
    size_t size;

    {
        std::lock_guard<std::shared_mutex> guard(_lock);
        Span* span=_id_span_map.find(id);
        if(span==nullptr)
            return;
        size=span->page_counts<<12;
        _span_alloc.Deallocate(span);
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
        Span* prev_span=_id_span_map.find(prev_id);
        if(prev_span && prev_span->used==false)
        {
            SpanList& list=_span_lists[prev_span->page_counts-1];
            list.Erase(prev_span);

            span->id=prev_span->id;
            span->page_counts+=prev_span->page_counts;
            span->start=prev_span->start;
            for(PageId i=prev_span->id;i<prev_span->page_counts+prev_span->id;++i)
                _id_span_map.insert(i,span);
            
            _span_alloc.Deallocate(prev_span);
        }
    }

    //向后合并
    if(!(span->start && span->page_counts==MAX_SPAN_SIZE))
    {
        PageId next_id=span->id+span->page_counts;
        Span* next_span=_id_span_map.find(next_id);
        if(next_span && next_span->used==false && next_span->start==false)
        {
            SpanList& list=_span_lists[next_span->page_counts-1];
            list.Erase(next_span);

            span->page_counts+=next_span->page_counts;
            for(PageId i=next_span->id;i<next_span->id+next_span->page_counts;++i)
                _id_span_map.insert(i,span);
            
            _span_alloc.Deallocate(next_span);
        }
    }
    
    SpanList& list=_span_lists[span->page_counts-1];
    list.PushBack(span);
    
}

Span *PageHeap::GetSpanFromBlock(void *block)
{
    std::shared_lock<std::shared_mutex> guard(_lock);
    return _id_span_map.find((PageId)block>>12);
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