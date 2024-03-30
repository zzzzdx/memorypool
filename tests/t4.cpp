#include "memory_pool.h"
#include <stdio.h>
#include <memory>
#include <vector>
#include <mutex>

class F
{
public:
    F(int a){printf("F\n");}
    F(const F& f){printf("copy F\n");}
    F(F&& f){printf("move F\n");}
    ~F(){printf("dis F\n");}
};

class C:public F
{
public:
    C():F(1){printf("C\n");}
};

F p(int i){
    F a(1);
    F b(2);
    if(i)
        return a;
    else
        return b;
}

std::mutex lock;

void work(){
    int a;
    std::lock_guard<std::mutex> guard(lock);
    printf("%p\n",&a);
}

int main(int argc, char **argv)
{
    std::vector<std::thread> t;
    work();
    for(int i=0;i<3;++i)
        t.push_back(std::thread(work));
    for(auto& i:t)
        i.join();
    return 0;
}