//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_CLIENT_H
#define MYGFS_CLIENT_H

#include "src/chunk/chunk_client.h"
#include "src/master/master_client.h"
#include "src/util/lock.h"

namespace gfs {

class FileContext;
struct Context;

class Client {
public:

  /*
   * @param route: MasterServer路由信息
   */
  Client(const std::string& route);

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

private:


  // int64_t first_read(const std::string& filename, char* buf, int64_t length);

  void make_context(const std::string& filename, FileContext** context_ret);

  void update_chunk_server(std::vector<std::string>& routes);

  ChunkClient* get_chunk_client(const std::string& route);


  /*
   * 获得文件的读取上下文，如果没有，那就创建一个
   */
  Context* get_context(const std::string& filename);

  /*
   * 生成文件对应的上下文，一次fetch服务器上的所有信息
   */
  bool make_context(const std::string& filename);


private:

  MasterClient master_;

  RWLOCK chunk_servers_rw_lock_;
  // route -> chunk server
  std::map<std::string, ChunkClient*> chunk_servers_;


  RWLOCK file_context_map_rw_lock_;
  // filename -> 文件读取上下文
  std::map<std::string, FileContext*> file_context_map_;

  RWLOCK context_rw_lock_;
  // 当前正在读取的文件上下文
  std::map<std::string, Context*> context_;

};


};

#endif //MYGFS_CLIENT_H
