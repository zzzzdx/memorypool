#include "common.h"
#include "page_cache.h"
#include <assert.h>


using namespace memorypool;

namespace memorypool
{


void *PageCache::GetPages(int page_num){
    void* p=mmap(nullptr,4096*page_num,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(p==nullptr)
        throw "mmap fail";
    return p;
}

void PageCache::FreePages(void *origin_page_ptr,size_t size){
    munmap(origin_page_ptr,size);
}

Span *PageCache::GetSpanNoLock(size_t page_num)
{
    if(page_num>MAX_SPAN_SIZE)
        page_num=MAX_SPAN_SIZE;
    for(int i=page_num-1;i<MAX_SPAN_SIZE;++i)
    {
        SpanList& list=_list_container[i];
        if(list.Size())
        {
            Span* ret=list.Begin();
            list.Erase(ret);
            if(page_num<ret->page_counts)
            {
                Span* left=ret;

                ret=new Span;
                ret->id=left->id;
                ret->page_counts=page_num;
                for(PageId i=ret->id;i<ret->page_counts+ret->id;++i)
                    _id_span_map[i]=ret;

                left->id+=page_num;
                left->page_counts-=page_num;
                _list_container[left->page_counts-1].PushBack(left);
            }
            return ret;
        }
    }

    void* pages=GetPages(MAX_SPAN_SIZE);
    Span* span=new Span;
    span->id=(PageId)pages>>12;
    span->page_counts=MAX_SPAN_SIZE;
    _list_container[MAX_SPAN_SIZE-1].PushBack(span);
    for(PageId i=span->id;i<span->page_counts+span->id;++i)
        _id_span_map[i]=span;
    return GetSpanNoLock(page_num);
}

PageCache &PageCache::GetInstance()
{
    static PageCache page_cache;
    return page_cache;
}

Span *PageCache::GetBigObj(int size)
{
    assert(size>=BIG_OBJ_SIZE);
    int page_num=SizeCalc::RoundUp(size,12)>>12;
    void* p=GetPages(page_num);
    Span* span = new Span;
    span->id=(PageId)p>>12;
    span->page_counts=page_num;

    std::lock_guard<std::mutex> guard(_lock);
    _id_span_map.insert(std::pair<PageId,Span*>(span->id,span));
    return nullptr;
}
void PageCache::FreeBigObj(void *ptr)
{
    PageId id=(PageId)ptr>>12;
    size_t size;

    {
        std::lock_guard<std::mutex> guard(_lock);
        auto itr=_id_span_map.find(id);
        if(itr==_id_span_map.end())
            return;
        size=itr->second->page_counts<<12;
        _id_span_map.erase(itr);
    }
    
    FreePages(ptr,size);
}
Span *PageCache::GetSpan(size_t page_num)
{
    std::lock_guard<std::mutex> guard(_lock);
    return GetSpanNoLock(page_num);
}
}