# tcmalloc分析
commit_id 674ee53f07374b715e134a4bf210d3746a1712c3

## tcmalloc架构图
![tcmalloc架构图](description/tcmalloc架构.jpg)

tcmalloc采用三级缓存,小对象从ThreadCache获取，大对象从PageAllocator获取。

## PageAllocator
PageAllocator负责分配Span(管理连续n个页)，类似代理真正的工作由PageHeap或HugePageAwareAllocator负责。单例，定义在static_var.h。

### PageHeap
PageHeap以数组形式管理kMaxPages(128)个SpanListPair链表，链表中每个Span包含的页都相同，和一个单独的包含大于kMaxPages页Span的链表。
PageHeap主要管理未使用的Span，由PageMap负责映射PageId到Span*的映射。考虑线程安全。PageMap内部使用PageMap2基数树，无哈希冲突，空间换时间。

#### Span* New(Length n, SpanAllocInfo span_alloc_info)
负责分配包含n个页的Span。调用AllocateSpan，先从n对应链表中找，没有就从更大的链表中找，找到后分割Span，余下部分放到对应大小的链表中。若还未找到则GrowHeap调用SystemAlloc，获取页，构建Span。

####  void Delete(Span* span, size_t objects_per_span)
尝试合并前后的Span放回对应链表中

## TransferCacheManager
TransferCacheManager管理kNumClasses个TransferCache。每个size空闲块对应一个。负责选择合适的TransferCache，并用其进行空闲块的分配与释放。空闲块分配与释放采用数组而非链表的形式。

### TransferCache
TransferCache管理特定size的空闲块，利用数组缓存部分空闲块，不够时向CentralFreeList申请，多余时还给CentralFreeList。

### CentralFreeList
CentralFreeList管理正在使用的Span链表，将Span链表分成kNumLists个链表进行管理。

#### int CentralFreeList<Forwarder>::RemoveRange(void** batch, int N)
分配时从链表中选取非空的Span获取空闲块，没有就调用Populate获取Span，在其上建立空闲链表，映射PageId与Span*。获取Span的页数由SizeClassInfo决定。

#### void CentralFreeList<Forwarder>::InsertRange(absl::Span<void*> batch)
负责释放空闲块到对应Span中，若Span不再被使用，则调用forwarder_.DeallocateSpans立刻释放回PageAllocator。将返回的每个空闲块对应的Span保存到Span* spans[kMaxObjectsToMove]。一批空闲块最多128个。

## ThreadCache
thread_local变量。负责为其线程申请释放小对象。

### FreeList
单向空闲链表，缓存固定size空闲块

#### void* Allocate(size_t size_class)
从对应FreeList获取一块空闲块，若没有则调用FetchFromTransferCache获取一批空闲块。空闲块数量：std::min<int>(list->max_length(), batch_size)。batch_size由SizeClassInfo决定。获取后对FreeList.max_length()+1或std::min(list->max_length() + batch_size, kMaxDynamicFreeListLength);

#### ThreadCache::Deallocate(void* ptr, size_t size_class)
将空闲块释放回FreeList，FreeList超过max_length则调用DeallocateSlow。

#### void DeallocateSlow(void* ptr, FreeList* list, size_t size_class)
调用ListTooLong和Scavenge。

ListTooLong调用ReleaseToTransferCache释放batch_size个空闲块。然后尝试调整max_length。调整策略为：当max_length小于batch_size则max_length+1,当大于且被调用三次则max_length-batch_size。

## 底层内存管理

### SystemAlloc与SystemRelease
MmapRegion线性管理MmapAligned利用mmap系统调用申请的虚拟内存。MmapRegion的内存由MallocInternal(没看懂)申请。RegionManager管理多个AddressRegion，通过AddressRegion分配内存。SystemAlloc通过RegionManager分配内存。
SystemRelease利用madvise系统调用释放内存。

## 元数据的内存管理

### Arena
Arena一次至少从SystemAlloc申请128KB，线性管理，只分配不回收。不够一次分配的部分直接丢弃，重新申请。位于static_var.h的静态变量arena_负责元数据内存分配。PageMap基数树结点直接从arena_分配。

### PageHeapAllocator
PageHeapAllocator模板类，用于元数据内存分配。内部有空闲链表和arena。从arena申请，释放到空闲链表。
静态变量PageHeapAllocator<Span> span_allocator_、PageHeapAllocator<ThreadCache> threadcache_allocator_都使用arena_。