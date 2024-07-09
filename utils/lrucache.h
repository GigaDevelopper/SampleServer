#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <list>
#include <shared_mutex>
#include <unordered_map>
#include <string>

//a sample lru cache only for current situation (Not templated)
namespace server
{
namespace utils {
class lrucache
{
public:
    lrucache(int capacity);

    std::string  get(const std::string &key);//get element from cache
    void put(const std::string &key, std::string &&value);//put element to cache

    typename std::list<std::string>::iterator begin(); // iterator to the beginning of cache
    typename std::list<std::string>::iterator end();   // iterator to the end of cache

private:
    int capacity_;
    //fast lru get->O(1) to hash table
    std::unordered_map<std::string, std::string> keyValue_;
    std::unordered_map<std::string, std::list<std::string>::iterator> keyIterator_;
    std::list<std::string> order_;
    //safe lru cache
    mutable std::shared_mutex mutex_;
};
}
}
#endif // LRUCACHE_H
