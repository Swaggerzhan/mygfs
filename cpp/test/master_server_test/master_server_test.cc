//
// Created by swagger on 2022/5/30.
//

#include "src/master/master_server.h"
#include "brpc/server.h"
#include "src/util/conf.h"

void select_start_port(int id, int* port);

int main(int argc, char** args) {



  gfs::MasterServerImpl master_server;

  master_server.start_debug();

  brpc::Server server;


  int ret = server.AddService(&master_server, brpc::SERVER_DOESNT_OWN_SERVICE);
  if ( ret != 0 ) {
    LOG(ERROR) << "add brpc service fail";
    return -1;
  }

  ret = server.Start(GFS_MASTER_SERVER_START_PROT, nullptr);
  if ( ret != 0 ) {
    LOG(ERROR) << "chunk server start at :" << GFS_MASTER_SERVER_START_PROT << " failed";
    return -1;
  }

  LOG(INFO) << "chunk server running....";
  while (!brpc::IsAskedToQuit()) {
    sleep(1);
  }

}


