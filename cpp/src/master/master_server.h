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
#include "src/master/file_info.h"

namespace gfs {

class ChunkServerClient;

typedef std::shared_ptr<FileInfo> FileInfoPtr;

class MasterServerImpl : public MasterServer {
public:

  // 链接至chunk服务器
  void start();

  MasterServerImpl();
  ~MasterServerImpl();

  /**
   * @brief 创建文件，返回一个chunk_handle的路由信息，类似FindLeaseHolder的响应
   * @param cntl
   * @param args
   * @param reply
   * @param done
   */
  void CreateFile (google::protobuf::RpcController* cntl,
                   const CreateFileArgs* args,
                   CreateFileReply* reply,
                   google::protobuf::Closure* done) override;

  void ListFiles (google::protobuf::RpcController* cntl,
                  const ListFilesArgs* args,
                  ListFilesReply* reply,
                  google::protobuf::Closure* done) override;


  void FileRouteInfo (google::protobuf::RpcController* cntl,
                      const FileRouteInfoArgs* args,
                      FileRouteInfoReply* reply,
                      google::protobuf::Closure* done) override;

  /**
   * @brief 找出当前Master所记录的Lease
   */
  void FindLeaseHolder(google::protobuf::RpcController* cntl,
                       const FindLeaseHolderArgs* args,
                       FindLeaseHolderReply* reply,
                       google::protobuf::Closure* done) override;


  // ******************* DEBUG ************************

  /**
   * @brief 以debug的形式启动Master服务器
   */
  void start_debug();
  /*
   * 填充测试的chunk到Master服务器内存中
   */
  void padding_test_file();


private:

  std::mutex files_mutex_;
  std::map<std::string, FileInfoPtr> files_;

};


}; // namespace gfs


#endif //MYGFS_MASTER_SERVER_H
