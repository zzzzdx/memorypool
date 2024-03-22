#include "sys_alloc.h"

namespace memorypool
{
void *Arena::Alloc(size_t size)
{
    if((_end-_start)<size)
    {
        size_t align_pages=SizeCalc::Align(size,12)>>12;
        size_t ask_pages=_incr_pages>=align_pages?_incr_pages:align_pages;
        _start=(char*)Mmap(ask_pages);
        _end=_start+(ask_pages<<12);
    }
    _end-=size;
    return _end;
}

}
