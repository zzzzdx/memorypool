#pragma once
#include "common.h"
#include <mutex>

namespace memorypool
{

//CentralCache负责管理正在使用的span,从PageCache中申请和释放span.
//将小对象span构建空闲链表,为ThreadCache提供空闲链表的申请和释放服务

//需要思考释放策略，负载因子
class CentralCache
{
private:
    //block size的划分与ThreadCache相同
    SpanList _span_lists[FREELIST_COUNTS];
    std::mutex _lock;

private:
    CentralCache(){}
    void GetSpan(size_t block_size,size_t page_num);

public:
    static CentralCache& GetInstance();

    //end不是尾后，MAX_SPAN_SIZE保证至少返回一个block，len只是建议值
    //每个span内部有自己的空闲链表,GetFreeList能融合各span的空闲链表向ThreadCache分配，在释放时归还各自span
    size_t GetFreeList(void*& start,void*& end,size_t size,size_t len);
};
}