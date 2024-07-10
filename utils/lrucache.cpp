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
    // Move the accessed key to the front of the order list
    order_.splice(order_.begin(), order_, keyIterator_[key]);
    return it->second;
}
void server::utils::lrucache::put(const std::string &key, std::string &&value)
{
    std::unique_lock<std::shared_mutex> writeLock(mutex_);

    auto it = keyValue_.find(key);
    if (it != keyValue_.end()) {
        // Key exists, update the value and move it to the front
        it->second = std::move(value);
        order_.splice(order_.begin(), order_, keyIterator_[key]);
    }
    else {
        // Key does not exist, check if we need to evict the least recently used item
        if (order_.size() == capacity_) {
            auto leastUsedKey = std::move(order_.back());
            order_.pop_back();
            keyValue_.erase(leastUsedKey);
            keyIterator_.erase(leastUsedKey);
        }

        // Insert the new key-value pair
        order_.push_front(key);
        keyValue_[key] = std::move(value);
        keyIterator_[key] = order_.begin();
    }
}

void server::utils::lrucache::put(const std::string &key, const std::string &value)
{
    std::unique_lock<std::shared_mutex> writeLock(mutex_);

    auto it = keyValue_.find(key);
    if (it != keyValue_.end()) {
        // Key exists, update the value and move it to the front
        it->second = value;
        order_.splice(order_.begin(), order_, keyIterator_[key]);
    }
    else {
        // Key does not exist, check if we need to evict the least recently used item
        if (order_.size() == capacity_) {
            auto leastUsedKey = order_.back();
            order_.pop_back();
            keyValue_.erase(leastUsedKey);
            keyIterator_.erase(leastUsedKey);
        }

        // Insert the new key-value pair
        order_.push_front(key);
        keyValue_[key] = value;
        keyIterator_[key] = order_.begin();
    }
}

std::list<std::string>::iterator server::utils::lrucache::begin()
{
    if(order_.empty())
        return order_.end();

    return order_.begin();
}

std::list<std::string>::iterator server::utils::lrucache::end()
{
    return order_.end();
}
