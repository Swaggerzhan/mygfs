//
// Created by swagger on 2022/6/2.
//
#include "file_info.h"
#include "chunk_route.h"
#include "src/chunk/chunk_client.h"

namespace gfs {

struct ChunkInfo {
public:

  ChunkInfo(uint64_t uuid)
  : chunk_handle(uuid)
  , primary(-1)
  , secondaries()
  , expired(0)
  {}

  void lock() { lock_.lock(); }
  void unlock() { lock_.unlock(); }


  uint64_t chunk_handle; // UUID
  int primary;            // 主primary的chunk server id
  std::vector<int> secondaries;
  uint64_t expired;

private:
  std::mutex lock_;
};


bool FileInfo::read_info(uint32_t chunk_index, FileRouteInfoReply* reply) {
  ReadLockGuard guard(&chunks_rw_lock_);
  auto it = chunks_.find(chunk_index);
  if ( it == chunks_.end() ) {
    return false;
  }
  if (it->second.primary != -1 ) {
    std::string primary_route;
    if (!fetch_chunk_server_route(it->second.primary, primary_route)) { // route info error
      return false;
    }
    reply->add_route(primary_route);
  }
  for (auto id : it->second.secondaries ) {
    std::string secondaries_route;
    if ( !fetch_chunk_server_route(id, secondaries_route) ) { // route info error
      return false;
    }
    reply->add_route(secondaries_route);
  }
  reply->set_chunk_handle(it->second.chunk_handle);
  return true;
}


bool FileInfo::write_info(uint32_t chunk_index, FindLeaseHolderReply* reply) {
  ChunkInfo* chunk_info = nullptr;
  {
    ReadLockGuard guard(&chunks_rw_lock_);
    auto it = chunks_.find(chunk_index);
    if ( it == chunks_.end() ) {
      return false;
    }
    chunk_info = &(it->second);
    chunk_info->lock(); // 锁传递
  }
  if ( chunk_info == nullptr ) {
    // TODO: FATAL_ERROR
  }
  if ( chunk_info->primary != -1 ) {
    if ( pick_primary(chunk_info->secondaries) < 0 ) { // 无法找出primary节点
      chunk_info->unlock();
      return false;
    }
    std::string primary_route;
    if (!fetch_chunk_server_route(chunk_info->primary, primary_route)) {
      // primary error
      chunk_info->unlock();
      return false;
    }
    reply->set_primary_route(primary_route);
  }
  for (auto id : chunk_info->secondaries ) {
    std::string secondary_route;
    if ( !fetch_chunk_server_route(id, secondary_route)) {
      continue; // nip
    }
    reply->add_route(secondary_route);
  }
  chunk_info->unlock();
  return true;
}

int FileInfo::pick_primary(const std::vector<int> &secondaries) {
  for (auto id : secondaries ) {
    ChunkClientPtr ptr;
    if (!fetch_chunk_server(id, ptr)) { // chunk server不响应，就找下一个
      continue;
    }
    // TODO: pick primary
    return id;
  }
  return -1;
}


}; // namespace gfs

