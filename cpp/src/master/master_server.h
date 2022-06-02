//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_MASTER_SERVER_H
#define MYGFS_MASTER_SERVER_H

#include <map>
#include <vector>
#include "src/util/lock.h"
#include "proto/out/gfs.pb.h"
#include "src/chunk/chunk_client.h"

namespace gfs {

class ChunkServerClient;
struct ChunkServerInfo;

typedef std::vector<ChunkServerClient*> ChunkServerClientPtrs;


class MasterServerImpl : public MasterServer {
public:

  // 链接至chunk服务器
  void start();

  MasterServerImpl();
  ~MasterServerImpl();

  void ListFiles (google::protobuf::RpcController* cntl,
                  const ListFilesArgs* args,
                  ListFilesReply* reply,
                  google::protobuf::Closure* done) override;


  void FileRouteInfo (google::protobuf::RpcController* cntl,
                      const FileRouteInfoArgs* args,
                      FileRouteInfoReply* reply,
                      google::protobuf::Closure* done) override;

  /*
   * 找出当前Master所记录的Lease
   */
  void FindLeaseHolder(google::protobuf::RpcController* cntl,
                       const FindLeaseHolderArgs* args,
                       FindLeaseHolderReply* reply,
                       google::protobuf::Closure* done) override;

private:





  // ******************* DEBUG ************************

  /*
   * 以debug的形式启动Master服务器
   */
  void start_debug();
  /*
   * 填充测试的chunk到Master服务器内存中
   */
  void padding_test_file();


private:


  RWLOCK files_rw_lock_;
  // 文件名 -> 对应文件的chunk UUID名
  std::map<std::string, std::vector<uint64_t>> files_;

  RWLOCK chunk_info_rw_lock_;
  // UUID -> 保存此UUID的chunk server
  std::map<uint64_t, std::vector<std::string>> chunk_route_info_;

  // route -> chunk server
  std::map<std::string, ChunkClient*> chunk_servers_;

  RWLOCK lease_info_rw_lock_;
  // UUID -> 获得Lease的服务器，每个chunk都有一个id标识
  std::map<uint64_t, uint64_t> lease_info_;

};


}; // namespace gfs


#endif //MYGFS_MASTER_SERVER_H
