//
// Created by swagger on 2022/6/2.
//
#include <map>
#include <memory>
#include "chunk_route.h"
#include "src/util/lock.h"
#include "src/chunk/chunk_client.h"


namespace gfs {


static RWLOCK lock {};
static std::map<int, ChunkClientPtr> chunks {};


int add_chunk_server(const std::string& route) {
  WriteLockGuard guard(&lock);
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
  ReadLockGuard guard(&lock);
  auto it = chunks.find(id);
  if ( it == chunks.end() ) {
    return false;
  }
  ptr = it->second;
  return true;
}






}; // namespace gfs