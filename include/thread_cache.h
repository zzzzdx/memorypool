#pragma once
#include "common.h"
#include "central_cache.h"

using namespace memorypool;
namespace memorypool
{
//线程的生命周期较小，注意析构时释放内存
//ThreadCache只负责缓存空闲块链表，申请和释放交给CentralCache
class ThreadCache
{
public:
    ~ThreadCache();
    static ThreadCache& GetInstance();
    void* Allocate(size_t size);
    void Deallocate(void* p);

private:
    ThreadCache(){}
    size_t NumForSize(size_t size);
    FreeList _free_lists[FREELIST_COUNTS];
};

}