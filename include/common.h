#pragma once
#include <stddef.h>
#include <algorithm>
#include <mutex>

namespace memorypool
{
//超过2页,8k为大对象
const int BIG_OBJ_SIZE=1<<13;

//PageHeap管理的Span容纳最大页数,
//MAX_SPAN_SIZE >= BIG_OBJ_SIZE>>12 注意保证Span至少足以容纳一个小对象
const int MAX_SPAN_SIZE=10;

// 注意空闲链表颗粒度的变化，较小对象的更细化，较大的更粗，
// 保证常用的较小对象内存碎片少以及链表总数不会太多
// 控制在12%左右的内碎片浪费
// [1,128]				8byte对齐 freelist[0,16)
// [129,1024]			16byte对齐 freelist[16,72)
// [1025,8*1024]		128byte对齐 freelist[72,128)

//管理空闲链表个数即空闲块尺寸个数
const int FREELIST_COUNTS=128;


//页地址右移
typedef unsigned long long PageId;

//转换成指向void*类型的指针即可赋值
inline void SetNextBlock(void* item,void* next){ *(void**)item=next; }
inline void* GetNextBlock(void* item){ return *(void**)item;}

class SizeCalc
{
public:
    static size_t Align(size_t size,size_t align)
    {
        size_t a=(1<<align)-1;
        return (size+a)&(~a);
    }

    static size_t RoundUp(size_t size)
    {
        if(size>1024) return Align(size,7);
        else if(size>128) return Align(size,4);
        else return Align(size,3);
    }

    //size对齐后所属空闲链表的下表
    static size_t Index(size_t size)
    {
        if(size>1024)
        {
            size_t off=RoundUp(size)-1024;
            return (off>>7)-1+72;
        }
        else if(size>128)
        {
            size_t off=RoundUp(size)-128;
            return (off>>4)-1+16;
        }
        else
        {
            size_t off=RoundUp(size);
            return (off>>3)-1;
        }
    }
};

class FreeList
{
private:
    void* _start=nullptr;
    size_t _size=0;
    size_t _max_size=0;
public:
    void Push(void* start,void* end,size_t size)
    {
        SetNextBlock(end,_start);
        _start=start;
        _size+=size;
    }

    void Push(void* item){ Push(item,item,1); }

    void* Pop() {return Pop(1);}

    void* Pop(size_t size)
    {
        void* ret=_start;

        if(ret)
        {
            void* end=ret;
            --_size;
            while(--size && GetNextBlock(end)){
                --_size;
                end=GetNextBlock(end);
            }
            _start=GetNextBlock(end);
            SetNextBlock(end,nullptr);
        }
        return ret;
    }

    size_t Size(){return _size;}
    size_t GetMax(){return _max_size;}
    void IncMax(int inc){_max_size+=inc;}
    void* Start(){return _start;}
};

//page整页用于空闲链表，span单独记录page信息
//可以考虑建立page头，避免额外内存开销
struct Span
{
    //起始页id
    PageId id;
    //连续页数量
    int page_counts;
    //空闲块使用数量
    int use_counts=0;
    //空闲链表，nullptr表示用完
    void* freelist=nullptr;
    size_t block_size=0;

    Span* prev=nullptr;
    Span* next=nullptr;

};

class SpanList
{
public:
    SpanList(){
        head.prev=&head;
        head.next=&head;
    }

    Span* Begin(){return head.next;}
    Span* End(){return &head;}
    void Erase(Span* item)
    {
        if(item == &head)
            return;
        item->prev->next=item->next;
        item->next->prev=item->prev;
        item->prev=item->next=nullptr;   
        --size;
    }
    void PushBack(Span* item){ Insert(&head,item); }
    void PushFront(Span* item){ Insert(head.next,item);}

    void Insert(Span* target,Span* item)
    {
        item->prev=target->prev;
        target->prev->next=item;
        item->next=target;
        target->prev=item;
        ++size;
    }

    int Size(){return size;}

private:
    Span head;
    int size=0;
public:
    std::mutex _lock;
};
}