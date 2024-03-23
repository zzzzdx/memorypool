#pragma once
#include "common.h"
#include <mutex>
#include <sys/mman.h>

using namespace memorypool;
namespace memorypool
{

inline void* Mmap(size_t pages){
void* p=mmap(nullptr,pages<<12,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
if(p==nullptr)
    throw "mmap fail";
return p;
}

//线性管理虚拟内存，用于元数据，只申请不释放
class Arena
{
public:
    void* Alloc(size_t size);
private:
    char* _start=nullptr;
    char* _end=nullptr;
    const size_t _incr_pages=16;
};

template<class Tp>
class SampleAlloc
{
private:
    std::mutex _lock;
    Arena _arena;
    void* _freelist=nullptr;
    size_t _used=0;
public:
    Tp* Allocate();
    void Deallocate(Tp* p);
    size_t Used(){return _used;}
};

template <class Tp>
inline Tp *SampleAlloc<Tp>::Allocate()
{
    Tp* ret=nullptr;
    if(_freelist)
    {
        ret=(Tp*)_freelist;
        _freelist=GetNextBlock(_freelist);
    }
    else
        ret=(Tp*)_arena.Alloc(sizeof(Tp));
    ++_used;
    return ret;
}

template <class Tp>
void SampleAlloc<Tp>::Deallocate(Tp *p)
{
    --_used;
    SetNextBlock(p,_freelist);
    _freelist=p;
}

void* SystemAlloc(size_t page_num);


}