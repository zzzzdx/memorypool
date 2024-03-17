#include "memory_pool.h"
#include <vector>
#include <thread>
#include <ctime>
#include <string.h>
using namespace memorypool;

void AllocateTest(int works,std::vector<int> sizes,int rounds)
{
    printf("allocate\n");
    clock_t start,end;
    start=clock();
    for(int r=0;r<rounds;++r)
    {
        std::vector<std::thread> threads;
        threads.reserve(works);
        auto work=[](std::vector<int> sizes){
            void** ps=(void**)Allocate(sizes.size()*206*8);
            int idx=0;
            for(auto s : sizes)
            {
                for(int i=0;i<206;++i)
                {
                    void* p=Allocate(s);
                    *(int*)p=15;
                    ps[idx++]=p;
                }
            }

            idx=0;
            for(auto s : sizes)
            {
                for(int i=0;i<206;++i)
                {
                    Deallocate(ps[idx++]);
                }
            }
            Deallocate(ps);
        };

        for(int i=0;i<works;++i)
            threads.push_back(std::thread(work,sizes));
        for(auto& t :threads)
            t.join();
    }
    end=clock();
    PageHeap& ph=PageHeap::GetInstance();
    printf("isfree %d\n",ph.FreeAllSpans());
    printf("use time %ld\n",end-start);
}

void MallocTest(int works,std::vector<int> sizes,int rounds)
{
    {
    printf("malloc\n");
    clock_t start,end;
    start=clock();
    for(int r=0;r<rounds;++r)
    {
        std::vector<std::thread> threads;
        threads.reserve(works);
        auto work=[](std::vector<int> sizes){
            void** ps=(void**)malloc(sizes.size()*206*8);
            int idx=0;
            for(auto s : sizes)
            {
                for(int i=0;i<206;++i)
                {
                    void* p=malloc(s);
                    *(int*)p=15;
                    ps[idx++]=p;
                }
            }

            idx=0;
            for(auto s : sizes)
            {
                for(int i=0;i<206;++i)
                {
                    free(ps[idx++]);
                }
            }
            free(ps);
        };

        for(int i=0;i<works;++i)
            threads.push_back(std::thread(work,sizes));
        for(auto& t :threads)
            t.join();
    }
    end=clock();
    printf("use time %ld\n",end-start);
}
}

int main(int args,char** argv)
{
    if(args!=2)
        return 0;
    if(strcmp(argv[1],"a")==0)
        AllocateTest(5,{8,32,48},3);
    else if(strcmp(argv[1],"m")==0)
        MallocTest(5,{8,32,48},3);
    return 0;
}