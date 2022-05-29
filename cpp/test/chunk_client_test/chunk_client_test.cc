//
// Created by swagger on 2022/5/29.
//
#include <iostream>
#include "src/chunk/chunk_client.h"
#include "src/util/conf.h"

using namespace std;

int main() {

  gfs::ChunkClient client(GFS_CHUNK_SERVER_1_ROUTE);
  bool ret = client.init();
  if ( !ret ) {
    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_1_ROUTE << " failed";
    return -1;
  }

  char buf[CHUNK_SIZE];

  int64_t byte_read = client.read(DEBUG_CHUNK_UUID, DEBUG_CHUNK_VERSION_BEGIN,
                                  buf, 0, 10);
  if ( byte_read != 10 ) {
    LOG(INFO) << "error";
  }
  for (int i=0; i<byte_read; ++i) {
    cout << i << ": " << buf[i] << endl;
  }

}

