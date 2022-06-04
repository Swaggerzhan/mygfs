//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_DISK_MANAGER_H
#define MYGFS_DISK_MANAGER_H

#include "src/util/state_code.h"
#include "src/util/lock.h"
#include "src/util/time.h"
#include "src/util/lru.hpp"
#include "page.h"

namespace gfs {

struct ChunkHandle;
typedef std::shared_ptr<ChunkHandle> ChunkHandlePtr;

class DiskManager {
public:

  explicit DiskManager(int port);
  ~DiskManager() = default;


  /*
   * 获取chunk，chunk将以PagePtr形式展现，可以通过对page写入来
   * 对真实的磁盘进行写入，page采用LRU缓存，但PagePtr是shared_ptr
   * 最后一个释放的人负责对page的更新刷入磁盘
   * @param chunk_handle : UUID
   * @param version: TODO:impl
   * @param ptr: 需要获取的page指针
   * @return: true为成功
   */
  bool fetch_chunk(uint64_t chunk_handle, uint32_t version, PagePtr& ptr);


  /*
   * 创建一个chunk_handle
   *
   */
  bool create_chunk(uint64_t chunk_handle, uint32_t version);

  // *************************** DEBUG **********************************

  void padding_debug_chunk();


private:

  int port_;
  std::string root_;


  RWLOCK chunks_rw_lock_;
  // chunk handle:UUID -> struct chunk handle
  std::map<uint64_t, ChunkHandlePtr> chunks_;


  std::mutex cache_mutex_;
  LRU<uint64_t, PagePtr> cache_;


};

};

#endif //MYGFS_DISK_MANAGER_H
