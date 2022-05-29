//
// Created by swagger on 2022/5/29.
//

#include "src/chunk/chunk_server.h"
#include "brpc/server.h"
#include "src/util/conf.h"



int main() {

  gfs::ChunkServerImpl chunk_server;
  chunk_server.start_debug();

  brpc::Server server;


  int ret = server.AddService(&chunk_server, brpc::SERVER_DOESNT_OWN_SERVICE);
  if ( ret != 0 ) {
    LOG(ERROR) << "add brpc service fail";
    return -1;
  }

  ret = server.Start(GFS_CHUNK_SERVER_START_PROT, nullptr);
  if ( ret != 0 ) {
    LOG(ERROR) << "chunk server start at :" << GFS_CHUNK_SERVER_START_PROT << " failed";
    return -1;
  }

  LOG(INFO) << "chunk server running....";
  while (!brpc::IsAskedToQuit()) {
    sleep(1);
  }

}

