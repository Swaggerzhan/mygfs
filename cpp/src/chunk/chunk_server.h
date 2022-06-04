//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_CHUNK_SERVER_H
#define MYGFS_CHUNK_SERVER_H

#include "proto/out/gfs.pb.h"
#include "disk_manager.h"

namespace gfs {

class ChunkServerImpl : public ChunkServer {
public:

  ChunkServerImpl(int port=30000);
  ~ChunkServerImpl();

  void ReadChunk(google::protobuf::RpcController* cntl,
                 const ReadChunkArgs* args,
                 ReadChunkReply* reply,
                 google::protobuf::Closure* done) override;

  void PutData(google::protobuf::RpcController* cntl,
               const PutDataArgs* args,
               PutDataReply* reply,
               google::protobuf::Closure* done) override;

  void WriteChunk(google::protobuf::RpcController* cntl,
                  const WriteChunkArgs* args,
                  WriteChunkReply* reply,
                  google::protobuf::Closure* done) override;


  // Master call rpc
  void HeartBeat(google::protobuf::RpcController* cntl,
                 const HeartBeatArgs* args,
                 HeartBeatReply* reply,
                 google::protobuf::Closure* done ) override;

  void InitChunk(google::protobuf::RpcController* cntl,
                 const InitChunkArgs* args,
                 InitChunkReply* reply,
                 google::protobuf::Closure* done) override;

  void MarkPrimaryChunk (google::protobuf::RpcController* cntl,
                         const MarkPrimaryChunkArgs* args,
                         MarkPrimaryChunkReply* reply,
                         google::protobuf::Closure* done) override;


  // *********************** DEBUG ***************************

  void start_debug();


private:

  DiskManager disk_;

  int port_; // for id



};

}; // namespace gfs




#endif //MYGFS_CHUNK_SERVER_H
