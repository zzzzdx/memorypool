#pragma once
#include "common.h"
#include "sys_alloc.h"

using namespace memorypool;
namespace memorypool
{

//3 level radix tree
//bits=48(64 address)-12(page offset)=36
template<size_t bits>
class PageMap
{
private:
    constexpr static size_t _node_bit=bits/3;
    constexpr static size_t _root_bit=bits-(_node_bit<<1);
    constexpr static size_t _node_len=1<<_node_bit;
    constexpr static size_t _root_len=1<<_root_bit;

    struct Node
    {
        void* vec[_node_len]={0};
    };

    Node* _root[_root_len]={0};
    SampleAlloc<Node> _node_alloc;

public:
    void insert(PageId id,Span* span)
    {
        //避免溢出
        size_t root_idx=(id>>(_node_bit<<1))&(_root_len-1);
        size_t mid_idx=(id>>_node_bit)&(_node_len-1);
        size_t leaf_idx=id&(_node_len-1);
        
        if(_root[root_idx]==nullptr)
            _root[root_idx]=_node_alloc.Allocate();
        Node* middel=_root[root_idx];

        if(middel->vec[mid_idx]==nullptr)
            middel->vec[mid_idx]=_node_alloc.Allocate();
        Span** leaf=(Span**)middel->vec[mid_idx];

        leaf[leaf_idx]=span;
    }

    Span* find(PageId id)
    {
        size_t root_idx=(id>>(_node_bit<<1))&(_root_len-1);
        size_t mid_idx=(id>>_node_bit)&(_node_len-1);
        size_t leaf_idx=id&(_node_len-1);

        if(_root[root_idx]==nullptr)
            return nullptr;
        Node* middel=_root[root_idx];

        if(middel->vec[mid_idx]==nullptr)
            return nullptr;
        Span** leaf=(Span**)middel->vec[mid_idx];
        return leaf[leaf_idx];
    }

};

}