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
            TransferCacheManager::GetInstance().RelFreeList(batch,num,i);
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
    size_t idx=SizeCalc::Index(size);
    FreeList& free_list=_free_lists[idx];
    if(free_list.Size())
    {
        void* ret;
        if(free_list.Pop(&ret))
            return ret;
    }
    
    size_t batch_size=SizeCalc::NumToMove(size);
    size_t len=std::min<int>(batch_size,free_list.GetMax());

    //偶发栈溢出问题排查：
    //综合考虑该函数和所有被调用函数
    //首先考虑多线程，ThreadCache是thread_local概率不大
    //其次考虑哪些是变量，排查哪些语句使用过函数形参，最后发现GetFreeList中len使用错误，导致batch溢出，覆盖调用栈底返回地址
    void* batch[MAX_BLOCK_MOVE];
    len=TransferCacheManager::GetInstance().GetFreeList(batch,len,idx);
    free_list.Push(batch+1,len-1);
    
    if(free_list.GetMax()<batch_size)
        free_list.IncMax(1);
    return batch[0];
}

void ThreadCache::Deallocate(void *p,size_t size)
{
    size_t idx=SizeCalc::Index(size);
    FreeList& list=_free_lists[idx];
    list.Push(p);

    if(list.Size()>list.GetMax())
        ListTooLong(list,size);
}

void ThreadCache::ListTooLong(FreeList &list,size_t size)
{
    size_t batch_size=SizeCalc::NumToMove(size);
    if(list.GetMax()>batch_size)
    {
        size_t idx=SizeCalc::Index(size);
        if(list._overage<3){
            ++list._overage;
            return;
        }
        list._overage=0;
        void* batch[MAX_BLOCK_MOVE];
        list.Pop(batch,batch_size);
        TransferCacheManager::GetInstance().RelFreeList(batch,batch_size,idx);
        list.IncMax(-batch_size);
    }
    else
        list.IncMax(1);
}
}