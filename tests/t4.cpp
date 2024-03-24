#include "memory_pool.h"
#include <stdio.h>

using namespace memorypool;

extern "C" {
    void* m_malloc(size_t size) noexcept{
        return Allocate(size);
    }

    void m_free(void* ptr) noexcept{
        Deallocate(ptr);
    }

}

void* malloc(size_t size) noexcept __attribute__((alias("m_malloc"), visibility("default")));
void free(void* ptr) noexcept __attribute__((alias("m_free"), visibility("default")));
 
int main(int argc, char **argv)
{
    //printf("hello\n");
    //puts("hello\n");
    write(1,"world",5);
    void* p=malloc(5);
    free(p);
    return 0;
}