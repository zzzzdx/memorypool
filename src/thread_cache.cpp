#include "thread_cache.h"
#include "page_heap.h"
namespace memorypool
{
ThreadCache::~ThreadCache()
{
    for(size_t i=0;i<FREELIST_COUNTS;++i)
    {
        FreeList& list=_free_lists[i];
        while(list.Size()>0)
        {
            void* batch[MAX_BLOCK_MOVE];
            size_t num=list.Pop(batch,MAX_BLOCK_MOVE);
            CentralCache::GetInstance().RelFreeList(batch,num,i);
        }
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
    {
        void* ret;
        if(free_list.Pop(&ret))
            return ret;
    }
    
    size_t len=NumForSize(size);
    void* batch[MAX_BLOCK_MOVE];
    len=CentralCache::GetInstance().GetFreeList(batch,SizeCalc::RoundUp(size),len);
    free_list.Push(batch+1,len-1);
    if(free_list.GetMax()<512)
        free_list.IncMax(len);
    return batch[0];
}

void ThreadCache::Deallocate(void *p,size_t size)
{
    size_t idx=SizeCalc::Index(size);
    FreeList& list=_free_lists[idx];
    list.Push(p);

    //ThreadCache过大须释放
    if(list.Size()>(list.GetMax()<<1))
    {
        void* batch[MAX_BLOCK_MOVE];
        int num=list.Size()-list.GetMax();
        num=list.Pop(batch,num);
        CentralCache::GetInstance().RelFreeList(batch,num,idx);
        list.IncMax(-num);
    }
}

size_t ThreadCache::NumForSize(size_t size)
{
    FreeList& free_list=_free_lists[SizeCalc::Index(size)];
    size_t max_size=free_list.GetMax();
    size_t num=BIG_OBJ_SIZE/size;
    if(num<2)
        num=2;
    else if(num>MAX_BLOCK_MOVE)
        num=MAX_BLOCK_MOVE;

    num=std::min(num,max_size);
    return num;
    /*
    FreeList& free_list=_free_lists[SizeCalc::Index(size)];
    size_t num=std::min(free_list.GetMax(),SizeCalc::NumToMove(SizeCalc::RoundUp(size)));
    return num;
}
    */
}
}