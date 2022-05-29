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

  ChunkServerImpl();
  ~ChunkServerImpl();

  void ReadChunk(google::protobuf::RpcController* cntl,
                 const ReadChunkArgs* args,
                 ReadChunkReply* reply,
                 google::protobuf::Closure* done) override;



  // *********************** DEBUG ***************************

  void start_debug();


private:

  DiskManager disk_;



};

}; // namespace gfs




#endif //MYGFS_CHUNK_SERVER_H
