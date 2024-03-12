#pragma once
#include <stddef.h>

namespace memorypool
{
//超过2页,8k为大对象
const int BIG_OBJ_SIZE=2<<14;

//PageCache管理的Span容纳最大页数,
//MAX_SPAN_SIZE >= BIG_OBJ_SIZE>>12 注意保证Span至少足以容纳一个小对象
const int MAX_SPAN_SIZE=10;



//页地址右移
typedef unsigned long long PageId;

class SizeCalc
{
public:
    static size_t RoundUp(size_t size,size_t align){
        size_t a=(1<<align)-1;
        return (size+a)&(~a);
    }
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
    void PushBack(Span* item)
    {
        item->prev=head.prev;
        head.prev->next=item;
        item->next=&head;
        head.prev=item;
        ++size;
    }

    int Size(){return size;}

private:
    Span head;
    int size=0;
};
}