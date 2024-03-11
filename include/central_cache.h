namespace memorypool
{

//CentralCache负责管理正在使用的span,从PageCache中申请和释放span.
//将小对象span构建空闲链表,为ThreadCache提供空闲链表的申请和释放服务

//需要思考释放策略，负载因子
class CentralCache
{
private:
    CentralCache(){}

public:
    static CentralCache& GetInstance();
};
}