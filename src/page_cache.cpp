#include "page_cache.h"

namespace memorypool
{
PageCache &PageCache::GetInstance()
{
    static PageCache page_cache;
    return page_cache;
}

}