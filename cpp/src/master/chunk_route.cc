//
// Created by swagger on 2022/6/2.
//
#include <map>
#include <memory>
#include "chunk_route.h"
#include "src/chunk/chunk_client.h"
#include "src/util/conf.h"

namespace gfs {

static std::mutex mutex_ {}; // FIXME: read write lock
static std::map<int, ChunkClientPtr> chunks {};

int add_chunk_server(const std::string& route) {
  std::unique_lock<std::mutex> lock_guard(mutex_);
  bool ok = true;
  for (auto it : chunks) {
     if (it.second->route() == route) {
        ok = false;
     }
  }
  if ( !ok ) {
    return -1;
  }
  ChunkClientPtr ptr(new ChunkClient(route));
  if ( !ptr->init() ) {
    return -1;
  }
  int id = ptr->id(); // get id from chunk server
  if ( id < 0 ) { // failed!
    return -1;
  }
  chunks[id] = ptr;
  return id;
}


bool fetch_chunk_server(int id, ChunkClientPtr& ptr) {
  std::unique_lock<std::mutex> lock_guard(mutex_);
  auto it = chunks.find(id);
  if ( it == chunks.end() ) {
    return false;
  }
  ptr = it->second;
  return true;
}

bool fetch_chunk_server_route(int id, std::string& route) {
  std::unique_lock<std::mutex> lock_guard(mutex_);
  auto it = chunks.find(id);
  if ( it == chunks.end() ) {
    return false;
  }
  route = it->second->route();
  return true;
}

bool route_empty() {
  std::unique_lock<std::mutex> lock_guard(mutex_);
  return chunks.empty();
}

bool pick_primary_and_secondaries(ChunkClientPtr& primary,
                                  std::vector<ChunkClientPtr>& secondaries) {
  std::unique_lock<std::mutex> lock_guard(mutex_);
  if ( chunks.empty() ) {
    return false;
  }
  bool pick = true;
  for (auto& ptr : chunks ) {
    if ( pick ) {
      primary = ptr.second;
      pick = false;
      continue;
    }
    secondaries.push_back(ptr.second);
  }
  return true;
}

// ****************** DEBUG ******************

void debug_start_connect() {
  bool ret = add_chunk_server(GFS_CHUNK_SERVER_1_ROUTE);
  if ( !ret ) {
    LOG(INFO) << "connect to: " << GFS_CHUNK_SERVER_1_ROUTE << " failed";
  }
  ret = add_chunk_server(GFS_CHUNK_SERVER_2_ROUTE);
  if ( !ret ) {
    LOG(INFO) << "connect to: " << GFS_CHUNK_SERVER_2_ROUTE << " failed";
  }
  ret = add_chunk_server(GFS_CHUNK_SERVER_3_ROUTE);
  if ( !ret ) {
    LOG(INFO) << "connect to: " << GFS_CHUNK_SERVER_3_ROUTE << " failed";
  }
}

}; // namespace gfs