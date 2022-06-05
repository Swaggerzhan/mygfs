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

  ChunkClient(const std::string& route, int chunk_client_id = rand());
  ~ChunkClient();

  /*
   * 初始化channel
   */
  bool init();

  std::string name();

  /*
   * @return: 当前路由信息
   */
  std::string route();

  /*
   * 得到服务器的chunk server id
   * @return: chunk server id，-1为失败
   */
  int id();

  /*
   * 客户端ID
   */
  int self_id();

  bool connected();

  // ********** for client ***********

  /**
   * @param chunk_handle: chunk的UUID
   * @param version: chunk版本
   * @param buf: 读取时的缓冲区，由调用者提供
   * @param offset: 读取对应开始位置
   * @param length: 期望读取的长度
   * @return: 真实的读取长度，0为结束，负数为异常
   */
  int64_t read_chunk(uint64_t chunk_handle, uint32_t version, char* buf,
                int64_t offset, int64_t length);

  /**
   * @brief 将数据缓存至chunk server，为之后的write_chunk做准备
   * @param timestamp: 时间
   * @param data: 数据内容
   * @return: true 为成功
   */
  bool put_data(uint64_t timestamp, const std::string& data);

  /**
   * @brief 将之前put_data缓存的数据提交到真正的chunk_handle中
   * @param timestamp: put_data时的时间戳
   * @param version: TODO:
   * @param chunk_handle: 需要写入的chunk_handle, UUID
   * @param offset: 需要写入的chunk_handle中的位置
   * @return int64_t: 写入的长度，负数为错误
   */
  int64_t write_chunk_commit(uint64_t timestamp, uint32_t version,
                             uint64_t chunk_handle, int64_t offset);



  // ********** for master ***********

  /*
   * 检测和chunk server的连接情况
   * @return: 返回chunk server的id，-1为连接中断
   */
  int heartbeat();

  /*
   * 在chunk server上生成一个新的chunk
   */
  bool init_chunk(uint64_t chunk_handle);

private:

  brpc::Channel channel_;
  std::string route_;

  std::atomic<bool> connected_;

  int chunk_server_id_; // for id
  int chunk_client_id_; // for self_id

};



};




#endif //MYGFS_CHUNK_CLIENT_H
