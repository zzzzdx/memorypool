#include "thread_cache.h"
namespace memorypool
{
ThreadCache &ThreadCache::GetInstance()
{
    thread_local ThreadCache thread_cache;
    return thread_cache;
}
}