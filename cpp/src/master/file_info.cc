//
// Created by swagger on 2022/6/2.
//
#include <random>
#include "file_info.h"
#include "chunk_route.h"
#include "src/chunk/chunk_client.h"
#include "src/util/time.h"

namespace gfs {

// FIXME
static uint64_t gen_uuid() {
  return random();
}

FileInfo::FileInfo(const std::string &filename)
: filename_(filename)
{}

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

bool FileInfo::init(CreateFileReply *reply) {
  std::unique_lock<std::mutex> lock_guard(chunks_mutex_);
  if ( !chunks_.empty() ) {
    return false; // 不是初始化
  }
  ChunkInfoPtr chunk_info_ptr(new ChunkInfo(gen_uuid()));

  // 主primary和副本ptr
  ChunkClientPtr primary_ptr;
  std::vector<ChunkClientPtr> secondaries_ptr;

  if ( !pick_primary_and_secondaries(primary_ptr, secondaries_ptr) ) {
    return false; // 找不到主primary
  }
  if ( !primary_ptr->init_chunk(chunk_info_ptr->chunk_handle) ) {
    return false; // 创建primary 副本失败
  }
  chunk_info_ptr->primary = primary_ptr->id(); // 主primary
  // 设置primary副本路由信息
  reply->set_primary_route(primary_ptr->route());
  for (auto& ptr : secondaries_ptr) {
    if ( ptr->init_chunk(chunk_info_ptr->chunk_handle) ) { // 成功创建副本，则记录
      chunk_info_ptr->secondaries.push_back(ptr->id());
      // 设置secondaries路由信息
      reply->add_routes(ptr->route());
    }
  }
  chunk_info_ptr->expired = get_lease_time();
  chunks_[0] = chunk_info_ptr; // 第一块chunk
  reply->set_chunk_handle(chunk_info_ptr->chunk_handle);

  // for debug
  LOG(INFO) << "init chunk: " << chunk_info_ptr->chunk_handle;
  LOG(INFO) << "primary chunk server id: " << primary_ptr->id();
  std::string tmp;
  for (auto& ptr : secondaries_ptr ) {
    tmp += " " + std::to_string(ptr->id());
  }
  LOG(INFO)  << "secondaries chunk server ids: " << tmp;

  return true;
}


bool FileInfo::read_info(uint32_t chunk_index, FileRouteInfoReply* reply) {
  std::unique_lock<std::mutex> lock_guard(chunks_mutex_);
  auto it = chunks_.find(chunk_index);
  if ( it == chunks_.end() ) {
    return false;
  }
  // 有primary的，但也是一视同仁直接加入到route中
  // 读取接口不考虑primary副本
  if (it->second->primary != -1 ) {
    std::string primary_route;
    if (!fetch_chunk_server_route(it->second->primary, primary_route)) { // route info error
      return false;
    }
    reply->add_route(primary_route);
  }
  for (auto id : it->second->secondaries ) {
    std::string secondaries_route;
    if ( !fetch_chunk_server_route(id, secondaries_route) ) { // route info error
      return false;
    }
    reply->add_route(secondaries_route);
  }
  reply->set_chunk_handle(it->second->chunk_handle);
  return true;
}


bool FileInfo::write_info(uint32_t chunk_index, FindLeaseHolderReply* reply) {
  ChunkInfoPtr chunk_info;
  {
    std::unique_lock<std::mutex> lock_guard(chunks_mutex_);
    auto it = chunks_.find(chunk_index);
    if ( it == chunks_.end() ) {
      return false;
    }
    chunk_info = it->second;
    chunk_info->lock(); // 锁传递
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
    reply->add_routes(secondary_route);
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

