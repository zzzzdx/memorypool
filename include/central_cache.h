#pragma once
#include "common.h"
#include <mutex>

namespace memorypool
{
//多线程要注意：
//1.判断共享区域代码+数据，细化锁粒度
//2.考虑线程安全时不仅考虑共享区域的读写，还要考虑对连贯操作的保证
//先读，如果没有就写入，这是一个整体应该一起加锁
//3.注意不可重入问题

//CentralCache负责管理正在使用的span,从PageHeap中申请和释放span.
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
    ~CentralCache();
    //注意不可重入，由外围加锁 
    void GetSpan(size_t block_size,size_t page_num);

public:
    static CentralCache& GetInstance();

    //end不是尾后，MAX_SPAN_SIZE保证至少返回一个block，len只是建议值
    //每个span内部有自己的空闲链表,GetFreeList能融合各span的空闲链表向ThreadCache分配，在释放时归还各自span
    size_t GetFreeList(void** batch,size_t size,size_t len);

    void RelFreeList(void** batch,size_t len, size_t idx);
    void Debug(){
        for(int i=0;i<FREELIST_COUNTS;++i)
            if(_span_lists[i].Size())
                printf("%d, ",i);
        printf("\n");
    }
};
}