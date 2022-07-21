//
// Created by swagger on 2022/6/5.
//

#ifndef MYGFS_CONTEXT_H
#define MYGFS_CONTEXT_H

#include <memory>
#include <vector>
#include <map>
#include "src/master/master_client.h"

namespace gfs {

class Context;
typedef std::shared_ptr<Context> ContextPtr;

class Context {
private:

  struct ChunkInfo {
    uint64_t chunk_handle;
    std::string primary;
    std::vector<std::string> secondaries;
  };
  typedef std::shared_ptr<ChunkInfo> ChunkInfoPtr;

public:

  Context(const std::string& filename, MasterClient* m);

  /**
   * @brief 更新当前文件的路由信息
   */
  void read_update();

  /**
   * @brief write前的更新操作，用于找到一个最新的 chunk_index
   * @param chunk_index: 需要获取的chunk_index
   */
  bool append_update(uint32_t& chunk_index);

  /**
   * @brief 基于read_offset_进行读取，读取一个length长度至buf中，至多读取一个chunk的长度
   * @param buf: 缓冲区
   * @param length: 想要读取的长度
   * @return 真实读取的长度
   */
  int64_t do_chunk_read(char* buf, int64_t length);

  /**
   * @brief 基于write_offset_进行写入，将buf中length长度写到write_offset_的位置，
   * 至多写入一个chunk的长度
   * @param buf
   * @param length
   * @return
   */
  int64_t do_append(char* buf, int64_t length);


private:
  friend class Client;
  MasterClient* m_;  // belong to Client

  int64_t read_offset_;
  int64_t write_offset_;

  std::string filename_;

  // chunk_index -> chunk info
  std::map<uint32_t, ChunkInfoPtr> chunks_;
};

}; // namespace gfs

#endif //MYGFS_CONTEXT_H
