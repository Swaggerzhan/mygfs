//
// Created by swagger on 2022/6/1.
//

#include <iostream>
#include <cassert>
#include "src/gfs/client.h"
#include "src/util/conf.h"

using namespace std;

int main() {
  gfs::Client client(GFS_MASTER_SERVER_ROUTE);

  assert ( client.init() == true );

  LOG(INFO) << "connect to master ok";

  char buf[CHUNK_SIZE * 2];

  int64_t ret = client.read(TEST_FILE_NAME, buf, CHUNK_SIZE * 2);
  cout << "total read: " << ret << endl;

  for (int i=0; i<ret; ++i) {
    cout << buf[i] << endl;
  }

}

