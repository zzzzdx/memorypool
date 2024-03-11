#include "gtest.h"
#include <thread>
#include <vector>
#include "thread_cache.h"
#include "central_cache.h"
#include "common.h"

using namespace memorypool;

GTEST_TEST(memory_pool,DISABLED_t_local)
{
    auto run=[]()
    {
        ThreadCache& c=ThreadCache::GetInstance();
        printf("%d %p\n",std::this_thread::get_id(),&c);
    };
    int num=2;
    std::vector<std::thread> tv;
    for(int i=0;i<num;++i)
    {
        std::thread t(run);
        tv.push_back(std::move(t));
    }
    for(int i=0;i<num;++i)
        tv[i].join();
}

GTEST_TEST(memory_pool,DISABLED_cent_static)
{
    auto run=[](){
        CentralCache& c=CentralCache::GetInstance();
        printf("%p\n",&c);
    };
    int num=2;
    std::vector<std::thread> tv;
    for(int i=0;i<num;++i)
    {
        std::thread t(run);
        tv.push_back(std::move(t));
    }
    for(int i=0;i<num;++i)
        tv[i].join();
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