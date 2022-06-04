//
// Created by swagger on 2022/6/3.
//

#ifndef MYGFS_LRU_HPP
#define MYGFS_LRU_HPP

#include <list>
#include <map>

#include <iostream>

using namespace std;

namespace gfs {


// ShardPtr?
template<typename Key, typename Value>
class LRU {
public:
  typedef typename std::pair<Key, Value> KeyValuePair;
  typedef typename std::list<KeyValuePair>::iterator iter;

  LRU(size_t capacity = 1024)
  : capacity_(capacity)
  , cache_map_()
  , cache_list_()
  {}

  void put(const Key& key, const Value& value) {
    auto it = cache_map_.find(key);
    cache_list_.push_front(KeyValuePair(key, value));
    if ( it != cache_map_.end() ) { // 已经缓存，则更新
      cache_list_.erase(it->second);
      cache_map_.erase(it);
    }

    cache_map_[key] = cache_list_.begin();
    if ( cache_map_.size() > capacity_ ) {
      evict();
    }
  }

  bool get(const Key& key, Value& value) {
    auto map_it = cache_map_.find(key);
    if ( map_it == cache_map_.end() ) {
      return false;
    }
    // update
    value = map_it->second->second;
    cache_list_.splice(cache_list_.begin(), cache_list_, map_it->second);
    return true;
  }

  bool exist(const Key& key) {
    auto it = cache_map_.find(key);
    return it != cache_map_.end();
  }

  void clear() {
    cache_map_.clear();
    cache_list_.clear();
  }

private:

  void evict() {
     auto last = -- (cache_list_.end());
     cache_map_.erase(last->first); // KeyValuePair first is key
     cache_list_.pop_back();
  }

private:

  size_t capacity_;

  std::list<KeyValuePair> cache_list_;

  std::map<Key, iter> cache_map_;

};

}; // namespace gfs

#endif //MYGFS_LRU_HPP
