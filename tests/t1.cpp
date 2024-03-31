#include "gtest/gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"
#include "page_heap.h"
#include "memory_pool.h"

using namespace memorypool;

GTEST_TEST(memory_pool,pcache_getspan)
{
    PageHeap& c=PageHeap::GetInstance();
    Span* s1=c.GetSpan(1);
    Span* s2=c.GetSpan(2);
    Span* s3=c.GetSpan(3);
    Span* s4=c.GetSpan(4);
    EXPECT_EQ(s1->page_counts,1);
    EXPECT_EQ(s2->id,s1->id+s1->page_counts);
    EXPECT_EQ(s3->id,s2->id+s2->page_counts);
    EXPECT_EQ(s4->id,s3->id+s3->page_counts);
}

GTEST_TEST(memory_pool,index)
{
    EXPECT_EQ(0,SizeCalc::Index(7));
    EXPECT_EQ(15,SizeCalc::Index(126));
    EXPECT_EQ(16,SizeCalc::Index(129));
    EXPECT_EQ(71,SizeCalc::Index(1024));
    EXPECT_EQ(72,SizeCalc::Index(1025));
    EXPECT_EQ(127,SizeCalc::Index(8*1024));
}

GTEST_TEST(memory_pool,spanlist)
{
    std::vector<int> ids={1,4,7,2,5,8};
    SpanList l;
    EXPECT_EQ(l.Size(),0);
    for (int i : ids)
    {
        Span* s=new Span;
        s->id=i;
        l.PushBack(s);
    }
    EXPECT_EQ(l.Size(),6);

    int idx=0;
    for(auto i=l.Begin();i!=l.End();i=i->next)
    {
        EXPECT_EQ(i->id,ids[idx++]);
    }

    idx=0;
    for(auto i=l.Begin();i!=l.End();)
    {
        if(idx>3)
            break;
        auto t=i->next;
        l.Erase(i);
        l.PushBack(i);
        ++idx;
        i=t;
    }

    for(auto i=l.Begin();i!=l.End();i=i->next)
    {
        EXPECT_EQ(i->id,ids[(idx++)%ids.size()]);
    }

    for(auto i=l.Begin();i!=l.End();)
    {
        auto tmp=i;
        i=i->next;
        l.Erase(tmp);
        delete tmp;
    }
    EXPECT_EQ(l.Size(),0);
}


GTEST_TEST(memory_pool,allocate)
{
    auto work=[]()
    {
        ThreadCache& c=ThreadCache::GetInstance();
        void** vec=(void**)c.Allocate(4096);
        for(int i=0;i<256;++i)
        {
            vec[i]=c.Allocate(8);
            *(int*)vec[i]=i;
        }
        for(int i=256;i<512;++i)
        {
            vec[i]=c.Allocate(16);
            *(int*)vec[i]=i;
        }
    };
    
    CentralCache& cc=CentralCache::GetInstance();
    PageHeap& pc=PageHeap::GetInstance();
    std::thread t1(work);
    std::thread t2(work);
    std::thread t3(work);
    t1.join();
    t2.join();
    t3.join();
    cc.Debug();
    int a=1;
}

GTEST_TEST(memory_pool,DISABLED_deallocate)
{
    auto work=[]()
    {
        ThreadCache& c=ThreadCache::GetInstance();
        void** vec=(void**)c.Allocate(4096);
        for(int i=0;i<256;++i)
        {
            vec[i]=c.Allocate(8);
            *(int*)vec[i]=i;
        }
        for(int i=256;i<512;++i)
        {
            vec[i]=c.Allocate(16);
            *(int*)vec[i]=i;
        }
        
        for(int i=0;i<512;++i){
            EXPECT_EQ(i,*(int*)vec[i]);
            int size=8;
            if(i>=256)
                size=16;
            c.Deallocate(vec[i],size);
        }
        c.Deallocate(vec,4096);
        
    };
    
    CentralCache& cc=CentralCache::GetInstance();
    PageHeap& pc=PageHeap::GetInstance();
    std::thread t1(work);
    std::thread t2(work);
    std::thread t3(work);
    std::thread t4(work);
    std::thread t5(work);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    EXPECT_EQ(true,pc.FreeAllSpans());
}

GTEST_TEST(memory_pool,pagemap)
{
    PageMap<36> m;
    int size=128;
    for(int i=0;i<size;++i)
    {
        size_t id=i<<12;
        m.insert(id,(Span*)id);
    }
        
    for(int i=0;i<size;++i)
    {
        size_t id=i<<12;
        size_t res=(size_t)m.find(id);
        EXPECT_EQ(res,id);
    }
}

GTEST_TEST(memory_pool,freelist)
{
    FreeList l;
    void* p[15];
    for(int i=0;i<15;++i)
        l.Push(p+i);
    
    for(int i=5;i>0;--i)
    {
        void* batch[5];
        size_t size=l.Pop(batch,i);

        EXPECT_EQ(i,size);
    }
    EXPECT_EQ(0,l.Size());
}