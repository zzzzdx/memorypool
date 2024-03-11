
//ThreadCache只负责缓存空闲块链表，申请和释放交给CentralCache

namespace memorypool
{

class ThreadCache
{
public:
    ~ThreadCache(){}
    static ThreadCache& GetInstance();
private:
    ThreadCache(){}
};

}