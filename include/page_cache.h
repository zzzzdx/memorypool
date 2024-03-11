namespace memorypool
{

//PageCache负责span的构造，并管理所有页到span映射,未使用span,以及span合并
//大对象的span是整体，小对象的span要构建每一个页的映射
//合并和暂不释放，munmap需要mmap得到的地址,考虑使用伙伴算法

class PageCache
{
private:
    PageCache(){}
public:
    static PageCache& GetInstance();
};

}