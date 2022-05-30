//
// Created by swagger on 2022/5/29.
//

#include "src/chunk/chunk_server.h"
#include "brpc/server.h"
#include "src/util/conf.h"

void select_start_port(int id, int* port);

int main(int argc, char** args) {
  if ( argc < 2 ) {
    LOG(ERROR) << "USAGE: ./target + id";
    return -1;
  }

  int port;

  select_start_port(std::stoi(args[1]), &port);

  gfs::ChunkServerImpl chunk_server;

  // chunk_server.start_debug();

  brpc::Server server;


  int ret = server.AddService(&chunk_server, brpc::SERVER_DOESNT_OWN_SERVICE);
  if ( ret != 0 ) {
    LOG(ERROR) << "add brpc service fail";
    return -1;
  }

  ret = server.Start(port, nullptr);
  if ( ret != 0 ) {
    LOG(ERROR) << "chunk server start at :" << port << " failed";
    return -1;
  }

  LOG(INFO) << "chunk server running....";
  while (!brpc::IsAskedToQuit()) {
    sleep(1);
  }

}

void select_start_port(int id, int* port) {
  switch (id) {
    case 1 : {*port = 30001; break;}
    case 2 : {*port = 30002; break;}
    case 3 : {*port = 30003; break;}
    default: {
      LOG(ERROR) << "Error id: " << id;
    }
  }
}
