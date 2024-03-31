#include "memory_pool.h"
#include <stdio.h>
#include <memory>
#include <vector>
#include <mutex>
#include "libc_override.h"

int main(int argc, char **argv)
{
    void* a[10];
    printf("%p\n",a);
    printf("%p\n",&a+1);
    printf("%p\n",a+1);
    printf("%p\n",&a[1]);
    return 0;
}