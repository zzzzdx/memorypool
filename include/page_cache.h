#pragma once
#include "common.h"
#include <unordered_map>
#include <sys/mman.h>
#include <mutex>

using namespace memorypool;
namespace memorypool
{

//PageCache负责span的构造，并管理所有页到span映射,未使用span,以及span合并
//大对象的span是整体，小对象的span要构建每一个页的映射
//合并和暂不释放，munmap需要mmap得到的地址,考虑使用伙伴算法

class PageCache
{
private:
    SpanList _list_container[MAX_SPAN_SIZE];
    //Span*如何释放？
    std::unordered_map<PageId,Span*> _id_span_map;
    std::mutex _lock;
private:
    PageCache(){}
    void* GetPages(int page_num);
    void FreePages(void* origin_page_ptr,size_t size);
    //用无锁版本解决递归不可重入锁
    Span* GetSpanNoLock(size_t page_num);
public:
    static PageCache& GetInstance();
    Span* GetBigObj(int size);
    void FreeBigObj(void* ptr);
    Span* GetSpan(size_t page_num);
    void FreeSpan(Span* span);
};

}