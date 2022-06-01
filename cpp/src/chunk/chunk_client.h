//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_CHUNK_CLIENT_H
#define MYGFS_CHUNK_CLIENT_H

#include "proto/out/gfs.pb.h"
#include "brpc/channel.h"

namespace gfs {

class ChunkClient {
public:

  ChunkClient(const std::string& route);
  ~ChunkClient();

  bool init();

  std::string name();

  bool connected();

  // ********** for client ***********

  /*
   * @param chunk_handle: chunk的UUID
   * @param version: chunk版本
   * @param buf: 读取时的缓冲区，由调用者提供
   * @param offset: 读取对应开始位置
   * @param length: 期望读取的长度
   * @return: 真实的读取长度，0为结束，负数为异常
   */
  int64_t read_chunk(uint64_t chunk_handle, uint32_t version, char* buf,
                int64_t offset, int64_t length);




  // ********** for master ***********
  bool heartbeat();

  /*
   * 在chunk server上生成一个新的chunk
   */
  bool init_chunk(uint64_t chunk_handle);

private:

  brpc::Channel channel_;
  std::string route_;

  std::atomic<bool> connected_;

};



};




#endif //MYGFS_CHUNK_CLIENT_H
