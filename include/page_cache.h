namespace memorypool
{
    
class PageCache
{
private:
    PageCache(){}
public:
    static PageCache& GetInstance();
};

}