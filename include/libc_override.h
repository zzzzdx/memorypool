#pragma once

#include "memory_pool.h"
#include <stdio.h>

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