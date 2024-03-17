#include "thread_cache.h"
#include "page_heap.h"
namespace memorypool
{
ThreadCache::~ThreadCache()
{
    for(size_t i=0;i<FREELIST_COUNTS;++i)
    {
        FreeList& list=_free_lists[i];
        if(list.Start())
            CentralCache::GetInstance().RelFreeList(list.Start(),i);
    }
}

ThreadCache &ThreadCache::GetInstance()
{
    thread_local ThreadCache thread_cache;
    return thread_cache;
}

void *ThreadCache::Allocate(size_t size)
{
    FreeList& free_list=_free_lists[SizeCalc::Index(size)];
    if(free_list.Size())
        return free_list.Pop();
    
    size_t len=NumForSize(size);
    void* start,*end;
    len=CentralCache::GetInstance().GetFreeList(start,end,SizeCalc::RoundUp(size),len);
    free_list.Push(start,end,len);
    if(free_list.GetMax()<512)
        free_list.IncMax(len);
    return free_list.Pop();
}

void ThreadCache::Deallocate(void *p,size_t size)
{
    size_t idx=SizeCalc::Index(size);
    FreeList& list=_free_lists[idx];
    list.Push(p);

    //ThreadCache过大须释放
    if(list.Size()>(list.GetMax()<<1))
    {
        int rel_num=list.Size()-list.GetMax();
        CentralCache::GetInstance().RelFreeList(list.Pop(rel_num),idx);
        list.IncMax(-rel_num);
    }
}

size_t ThreadCache::NumForSize(size_t size)
{
    FreeList& free_list=_free_lists[SizeCalc::Index(size)];
    size_t max_size=free_list.GetMax();
    if(max_size==0)
        max_size=10;
    size_t num=BIG_OBJ_SIZE/size;
    if(num<2)
        num=2;
    else if(num>206)
        num=206;

    num=std::min(num,max_size);
    return num;
}
}