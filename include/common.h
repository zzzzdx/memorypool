namespace memorypool
{
//页地址右移
typedef unsigned int PageId;

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