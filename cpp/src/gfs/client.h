//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_CLIENT_H
#define MYGFS_CLIENT_H

#include "src/chunk/chunk_client.h"
#include "src/master/master_client.h"
#include "src/util/lock.h"
#include "context.h"

namespace gfs {

// struct Context;
// typedef std::shared_ptr<Context> ContextPtr;

class Client {
public:

  /**
   * @param route: MasterServer路由信息
   */
  Client(const std::string& route);

  /**
   * @brief 初始化Client，和MasterServer连接
   * @return
   */
  bool init();


  /*
   * 对指定的文件发起读取操作，offset会改变
   * read不同文件是线程安全的，相同文件则不是线程安全的
   * @param filename: 文件名
   * @param buf: caller给定的bug
   * @param length: 读取的长度
   * @return: 真实读取的长度
   */
  int64_t read(const std::string& filename, char* buf, int64_t length);

  /**
   * @brief 读取文件到buf中
   * @param filename: 想要读取的文件
   * @param buf: 读取的缓存地址
   * @param length: 想要读取的长度
   * @return: 真实读取长度，-1为失败
   */
  int64_t read2(const std::string& filename, char* buf, int64_t length);

  /**
   * @brief 对指定的文件追加数据
   * @param filename: 文件名
   * @param buf: 写入的数据
   * @param length: 写入的长度
   * @return: 真实的写入长度
   */
  int64_t append(const std::string& filename, char* buf, int64_t length);

private:

  /**
   * @brief 获得一个文件所对应的读写上下文，如果没有，那就创建一个
   * @param filename: 文件名
   * @param ptr: 读写上下文指针
   * @return true 为成功
   */
  bool get_context(const std::string& filename, ContextPtr& ptr);

  /**
   * @brief 创建一个读写上下文
   * @param filename: 文件名
   * @param ptr: 上下文返回指针
   * @return true 为成功
   */
  bool make_context(const std::string& filename, ContextPtr& ptr);


  bool pick_read_chunk_ptr(const ContextPtr& ptr, uint32_t chunk_index,
                           ChunkClientPtr& ret);


private:

  MasterClient master_;

  std::mutex context_mutex_;
  // 当前正在读取的文件上下文
  std::map<std::string, ContextPtr> context_;

};


};

#endif //MYGFS_CLIENT_H
