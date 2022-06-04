//
// Created by swagger on 2022/5/29.
//

#include <unistd.h>
#include <fcntl.h>
#include "brpc/server.h"
#include "disk_manager.h"
#include "src/util/conf.h"

namespace gfs {

static char garbage = '\0';
static void padding_garbage(int fd) {
  for (int i=0; i<CHUNK_SIZE; ++i) {
    ::write(fd, &garbage, 1);
  }
}

struct ChunkHandle {
public:

  ChunkHandle(const std::string& p)
  : chunk_handle(0)
  , lease(0)
  , version(DEBUG_CHUNK_VERSION_BEGIN) // 1
  , length(0)
  , path(p)
  {}

  std::string path;
  uint64_t chunk_handle; // placeholder
  uint64_t lease;
  int64_t length; // chunk大小
  uint32_t version; // TODO: impl
};

DiskManager::DiskManager(int port)
: port_(port)
{
  root_ += GFS_CHUNK_SERVER_ROOT_DIR;
  root_ += std::to_string(port) + "/";
  LOG(INFO) << "chunk server root at: " << root_;
}


bool DiskManager::fetch_chunk(uint64_t chunk_handle, uint32_t version, PagePtr& ptr) {
  // 获取chunk 信息
  ChunkHandlePtr chunk_info_ptr;
  {
    ReadLockGuard guard(&chunks_rw_lock_);
    auto it = chunks_.find(chunk_handle);
    if ( it == chunks_.end() ) {
      return false;
    }
    chunk_info_ptr = it->second;
  }
  std::unique_lock<std::mutex> cache_guard(cache_mutex_);
  // 试图从缓存中直接拿到
  if ( cache_.get(chunk_handle, ptr) ) {
    return true;
  }
  // get it from disk
  ptr.reset(new Page());
  if (!ptr->init(chunk_info_ptr->path)) {
    return false;
  }
  // 缓存
  cache_.put(chunk_handle, ptr);
  ptr->chunk_info = chunk_info_ptr; // weak ptr
  return true;
}


bool DiskManager::create_chunk(uint64_t chunk_handle, uint32_t version) {
  LOG(INFO) << 4;
  // WriteLockGuard guard(&chunks_rw_lock_);
  // 检测是否已经存在
  LOG(INFO) << 4;
  if ( chunks_.find(chunk_handle) != chunks_.end() ) {
    return false;
  }
  LOG(INFO) << 5;
  std::string path = root_ + std::to_string(chunk_handle) + std::to_string(version);
  ChunkHandlePtr ptr(new ChunkHandle(path));
  LOG(INFO) << 6;
  int fd = open(path.c_str(), O_CREAT | O_RDWR);
  if ( fd < 0 ) {
    return false;
  }
  padding_garbage(fd);
  LOG(INFO) << 7;
  close(fd);
  chunks_[chunk_handle] = ptr;
  return true;
}


// ****************************** DEBUG ******************************
#include <cassert>


void DiskManager::padding_debug_chunk() {
  LOG(INFO) << "disk padding debug chunk at root: " << root_;

  assert ( create_chunk(DEBUG_CHUNK_UUID, DEBUG_CHUNK_VERSION_BEGIN) );
  LOG(INFO) << 1;
  PagePtr ptr;
  assert ( fetch_chunk(DEBUG_CHUNK_UUID, DEBUG_CHUNK_VERSION_BEGIN, ptr) );
  LOG(INFO) << 2;
  char* mem = ptr->write_expose();
  LOG(INFO) << 3;

  for (int i=0; i<CHUNK_SIZE; ++i) {
    mem[i] = '0' + (i % 10);
  }
  LOG(INFO) << "start flush...";
  ptr->flush();
  LOG(INFO) << "flush ok...";
  // for gfs_test_file
  LOG(INFO) << "padding gfs test file";

  assert ( create_chunk(TEST_FILE_CHUNK_HANDLE_1, DEBUG_CHUNK_VERSION_BEGIN));
  assert ( create_chunk(TEST_FILE_CHUNK_HANDLE_2, DEBUG_CHUNK_VERSION_BEGIN));

  ptr.reset();

  assert ( fetch_chunk(TEST_FILE_CHUNK_HANDLE_1, DEBUG_CHUNK_VERSION_BEGIN, ptr) );
  mem = ptr->write_expose();
  for (int i=0; i<CHUNK_SIZE; ++i) {
    mem[i] = '1';
  }
  ptr->flush();

  ptr.reset();
  assert ( fetch_chunk(TEST_FILE_CHUNK_HANDLE_2, DEBUG_CHUNK_VERSION_BEGIN, ptr) );
  mem = ptr->write_expose();
  for (int i=0; i<CHUNK_SIZE; ++i) {
    mem[i] = '2';
  }
  ptr->flush();

}

};
