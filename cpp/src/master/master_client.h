//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_MASTER_CLIENT_H
#define MYGFS_MASTER_CLIENT_H

#include "proto/out/gfs.pb.h"
#include "brpc/channel.h"

namespace gfs {

class MasterClient {
public:

  MasterClient(const std::string& route);
  ~MasterClient() = default;

  bool init();

  /*
   * 列出当前目录下存在的所有文件
   * TODO: list
   */
  bool list_file(std::vector<std::string>& ret);

  /*
   * 根据文件名和文件index获取对应的路由信息
   * 通常是由read操作之前调用，master不会区别对待副本是否持有Lease
   *  @param filename : 文件名
   *  @param chunk_index: 文件对应的chunk
   *  @param routes: 路由信息
   *  @param chunk_handle: chunk对应的UUID
   */
  bool file_info_at(const std::string& filename, uint64_t chunk_index,
                    std::vector<std::string>& routes, uint64_t* chunk_handle);


  // ************** DEBUG ************************

  /*
   * 调用list_file并且显示服务器上的文件内容
   */
  void print_files();


private:

  std::atomic<bool> connected_;

  std::string route_;
  brpc::Channel channel_;


};



};
#endif //MYGFS_MASTER_CLIENT_H
