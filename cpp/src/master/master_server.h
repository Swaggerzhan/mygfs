//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_MASTER_SERVER_H
#define MYGFS_MASTER_SERVER_H

#include <map>
#include <vector>
#include "src/util/lock.h"
#include "proto/out/gfs.pb.h"

namespace gfs {

class ChunkServerClient;

typedef std::vector<ChunkServerClient*> ChunkServerClientPtrs;


class MasterServerImpl : public MasterServer {
public:

  void ListFiles (google::protobuf::RpcController* cntl,
                  const ListFilesArgs* args,
                  ListFilesReply* reply,
                  google::protobuf::Closure* done) override;


  void FileRouteInfo (google::protobuf::RpcController* cntl,
                      const FileRouteInfoArgs* args,
                      FileRouteInfoReply* reply,
                      google::protobuf::Closure* done) override;


private:


  RWLOCK files_rw_lock_;
  // 文件名 -> 对应文件的chunk UUID名
  std::map<std::string, std::vector<uint64_t>> files_;

  RWLOCK chunk_info_rw_lock_;
  // UUID -> 保存此UUID的chunk server
  std::map<uint64_t, ChunkServerClientPtrs> chunk_info_;

  // UUID -> 获得Lease的服务器，每个chunk都有一个id标识
  std::map<uint64_t, uint64_t> lease_info_;

};


}; // namespace gfs


#endif //MYGFS_MASTER_SERVER_H
