#include "central_cache.h"
#include "page_heap.h"

namespace memorypool
{
    
CentralFreeList::~CentralFreeList()
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

void CentralFreeList::GetSpan(size_t block_size, size_t page_num)
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
    _list.PushFront(span);
}

size_t CentralFreeList::GetFreeList(void** batch,size_t len)
{
    std::lock_guard<std::mutex> guard(_lock);
    size_t size=SizeCalc::Size(_idx);
    size_t i=0;
    while(i==0)
    {
       Span* span=_list.Begin();
        if(span==_list.End() || span->freelist==nullptr)
            GetSpan(size,SizeCalc::Align(size*len,12)>>12);
        else
        {
            void* cur=span->freelist;
            for(;i<len && cur!=nullptr;++i)
            {
                ++span->use_counts;
                batch[i]=cur;
                cur=GetNextBlock(cur);
            } 

            span->freelist=cur;
            if(span->freelist==nullptr){
                _list.Erase(span);
                _list.PushBack(span);
            }
            break;
        } 
    }
    return i;
}

void CentralFreeList::RelFreeList(void** batch,size_t len)
{   
    std::lock_guard<std::mutex> guard(_lock);
    PageHeap& ph=PageHeap::GetInstance();
    //返回的空闲链表可能由多个span分配
    for(size_t i=0;i<len;++i)
    {
        void* cur=batch[i];
        Span* span=ph.GetSpanFromBlock(cur);
        if(span)
        {
            --span->use_counts;
            SetNextBlock(cur,span->freelist);
            span->freelist=cur;
            _list.Erase(span);
                
            //判断是否释放
            if(span->use_counts==0)
                ph.FreeSpan(span);
            else
                _list.PushFront(span);
        }
    }
}

void *TransferCacheManager::Alloc(size_t size)
{
    return _arena.Alloc(size);
}

TransferCache::TransferCache(size_t idx, TransferCacheManager *manger, size_t capacity):_capacity(capacity),
                    _slots(nullptr),_size(0),_freelist(idx),_manger(manger){
        _slots=(void**)_manger->Alloc(_capacity*sizeof(void*));
}

size_t TransferCache::GetFreeList(void **batch, size_t len)
{
    std::lock_guard<std::mutex> guard(_lock);
    size_t i=0;
    while(_size>0 && len>0)
    {
        batch[i++]=_slots[--_size];
        --len;
    }

    if(len)
        i+=_freelist.GetFreeList(batch+i,len);

    return i;
}

void TransferCache::RelFreeList(void **batch, size_t len)
{
    std::lock_guard<std::mutex> guard(_lock);
    size_t i=0;

    while(_size<_capacity && len>0)
    {
        _slots[_size++]=batch[i++];
        --len;
    }

    if(len)
        _freelist.RelFreeList(batch+i,len);
}
}