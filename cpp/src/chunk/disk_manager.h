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
typedef std::pair<uint64_t, uint64_t> store_mark_t;

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

  /*
   * 将某个chunk_handle标记为主要副本
   * @param chunk_handle: 被标记的chunk
   * @param version: 设定为这个版本
   * @param lease: lease过期的时间
   * @return: true为成功
   */
  bool mark_as_primary(uint64_t chunk_handle, uint32_t version, uint64_t lease);

  /*
   * 将数据进行临时的缓存，为之后的写入做准备
   * @param client_id: 执行缓存操作的客户端id
   * @param time: 缓存操作提交时的时间
   * @param data: 数据
   * @param len: 数据长度
   * @return: true 为成功
   */
  bool store_tmp_data(int64_t client_id, uint64_t time, const char* data, int len);


  /*
   * 提交由PutData写入的数据
   * @param client_id: 客户端id
   * @param time: PutData缓存时的时间
   * @param chunk_handle: 将写入哪个chunk
   * @param version: 写入chunk的version
   * @param offset: 写入chunk的位置
   * @return: state_code表明状态
   *        state_ok: 成功
   *        state_length_error: 写入长度错误
   *        state_file_not_found: 写入的chunk_handle找不到，或者是tmp cache找不到
   */
  state_code write_chunk_commit(uint64_t client_id, uint64_t time,
                                uint64_t chunk_handle, uint32_t version, int64_t offset);

  // *************************** DEBUG **********************************

  void padding_debug_chunk();


private:

  int port_;
  std::string root_;

  std::string tmp_dir_; // root_  + tmp/


  std::mutex chunks_mutex_;
  // chunk handle:UUID -> struct chunk handle
  std::map<uint64_t, ChunkHandlePtr> chunks_;


  std::mutex cache_mutex_;
  LRU<uint64_t, PagePtr> cache_;

  std::mutex tmp_mutex_;
  std::map<store_mark_t, PagePtr> tmp_cache_;


};

};

#endif //MYGFS_DISK_MANAGER_H
