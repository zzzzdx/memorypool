#pragma once
#include "common.h"
#include <mutex>
#include "sys_alloc.h"

namespace memorypool
{
//多线程要注意：
//1.判断共享区域代码+数据，细化锁粒度
//2.考虑线程安全时不仅考虑共享区域的读写，还要考虑对连贯操作的保证
//先读，如果没有就写入，这是一个整体应该一起加锁
//3.注意不可重入问题

//CentralFreeList负责管理正在使用的span,从PageHeap中申请和释放span.
//将小对象span构建空闲链表,为ThreadCache提供空闲链表的申请和释放服务

//需要思考释放策略，负载因子
class CentralFreeList
{
private:
    size_t _idx;
    SpanList _list;
    std::mutex _lock;

private:
    //注意不可重入，由外围加锁 
    void GetSpan(size_t block_size,size_t page_num);

public:
    CentralFreeList(size_t i):_idx(i){}
    ~CentralFreeList();

    //MAX_SPAN_SIZE保证至少返回一个block，len只是建议值
    //每个span内部有自己的空闲链表,GetFreeList能融合各span的空闲链表向ThreadCache分配，在释放时归还各自span
    size_t GetFreeList(void** batch,size_t len);

    void RelFreeList(void** batch,size_t len);
};

class TransferCacheManager;

class TransferCache
{
private:
    size_t _capacity;
    void** _slots;
    size_t _size;
    CentralFreeList _freelist;
    TransferCacheManager* _manger;
    std::mutex _lock;

public:
    TransferCache(size_t idx,TransferCacheManager* manger,size_t capacity=64);
    size_t GetFreeList(void** batch,size_t len);
    void RelFreeList(void** batch,size_t len);
    void clear(){
        if(_size)
            _freelist.RelFreeList(_slots,_size);
    }
};

class TransferCacheManager
{
private:
    Arena _arena;

    //利用union推迟TransferCache初始化
    //block size的划分与ThreadCache相同
    union Cache{
        Cache():dummy(false){}
        ~Cache(){}
        TransferCache tc;
        bool dummy;
    };
    Cache _cache[FREELIST_COUNTS];

    TransferCacheManager(){
        for(int i=0;i<FREELIST_COUNTS;++i)
            ::new(&_cache[i].tc) TransferCache(i,this);
    }
public:
    static TransferCacheManager& GetInstance(){
        static TransferCacheManager manager;
        return manager;
    }
    void* Alloc(size_t size);
    size_t GetFreeList(void** batch,size_t len,size_t idx){
        return _cache[idx].tc.GetFreeList(batch,len);
    }
    void RelFreeList(void** batch,size_t len,size_t idx){
        _cache[idx].tc.RelFreeList(batch,len);
    }

    void clear(){
        for(int i=0;i<FREELIST_COUNTS;++i)
            _cache[i].tc.clear();
    }
};

}