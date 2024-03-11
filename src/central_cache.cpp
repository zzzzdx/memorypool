#include "central_cache.h"

namespace memorypool
{
CentralCache &CentralCache::GetInstance()
{
    static CentralCache central_cache;
    return central_cache;
}

}