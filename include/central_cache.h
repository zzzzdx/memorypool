namespace memorypool
{
class CentralCache
{
private:
    CentralCache(){}

public:
    static CentralCache& GetInstance();
};
}