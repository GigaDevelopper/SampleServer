#include "lrucache.h"

#include <mutex>

server::utils::lrucache::lrucache(int capacity)
    :capacity_{capacity}
{
    if(capacity <= 0)
        throw std::invalid_argument("Capacity must be greater than 0");
}
std::string server::utils::lrucache::get(const std::string &key)
{
    //read data more than one threads
    std::shared_lock<std::shared_mutex> readLock(mutex_);
    auto it = keyValue_.find(key);
    if (it == keyValue_.end()) {
        return "";  // Key not found return empty string
    }
    readLock.unlock();

    //change order least recently used
    std::unique_lock<std::shared_mutex> writeLock(mutex_);
    auto listIterator = keyIterator_[key];
    if(listIterator != order_.end() && listIterator != order_.begin()){
        order_.erase(listIterator);
        order_.push_front(key);
        keyIterator_[key] = order_.begin();
    }
    return it->second;
}
void server::utils::lrucache::put(const std::string &key, std::string &&value)
{
    //update key if key has and order position
    if(!get(key).empty()){
        std::unique_lock<std::shared_mutex> writeLock(mutex_);
        keyValue_[key] = std::move(value);
        return;
    }
    std::unique_lock<std::shared_mutex> writeLock(mutex_);
    if((int)order_.size() == capacity_){
        auto lastKey = order_.back();
        order_.pop_back();
        keyValue_.erase(lastKey);
        keyIterator_.erase(lastKey);
    }
    order_.push_front(key);
    keyIterator_[key] = order_.begin();
    keyValue_[key] = value;
}
