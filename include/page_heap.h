#pragma once
#include "common.h"
#include <unordered_map>
#include <sys/mman.h>
#include <mutex>
#include <shared_mutex>
#include "sys_alloc.h"
#include "page_map.h"

using namespace memorypool;
namespace memorypool
{

//PageHeap负责span的构造，并管理所有页到span映射,未使用span,以及span合并
//大对象的span是整体，小对象的span要构建每一个页的映射
//合并和暂不释放，munmap需要mmap得到的地址,考虑使用伙伴算法

class PageHeap
{
private:
    SpanList _span_lists[MAX_SPAN_SIZE];
    std::shared_mutex _lock;
    size_t _total_get=0;
    SampleAlloc<Span> _span_alloc;
    PageMap<36> _id_span_map;

private:
    PageHeap(){}
    void FreePages(void* origin_page_ptr,size_t size);
    //用无锁版本解决递归不可重入锁
    Span* GetSpanNoLock(size_t page_num);
public:
    static PageHeap& GetInstance();
    void* GetBigObj(int size);
    void FreeBigObj(void* ptr);
    Span* GetSpan(size_t page_num);
    void FreeSpan(Span* span);

    Span* GetSpanFromBlock(void* block);
    bool FreeAllSpans();
    bool FreeAllSample(){return _span_alloc.Used()==_span_lists[MAX_SPAN_SIZE-1].Size();}

    void Debug(){
        for(int i=0;i<MAX_SPAN_SIZE;++i)
        {
            if(_span_lists[i].Size()!=0)
                printf("idx %d has %d,",i,_span_lists[i].Size());
        }
        printf("\n");
    }
};

}