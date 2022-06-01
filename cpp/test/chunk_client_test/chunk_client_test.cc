//
// Created by swagger on 2022/5/29.
//
#include <iostream>
#include "src/chunk/chunk_client.h"
#include "src/util/conf.h"

using namespace std;

void test_at_chunk_server_1();

int main() {

  test_at_chunk_server_1();

}

void test_at_chunk_server_1() {
  gfs::ChunkClient client(GFS_CHUNK_SERVER_1_ROUTE);
  bool ret = client.init();

  assert ( ret == true );

  char buf[CHUNK_SIZE * 2];


  int64_t byte_read = client.read_chunk(TEST_FILE_CHUNK_HANDLE_1, DEBUG_CHUNK_VERSION_BEGIN,
                                  buf, 0, CHUNK_SIZE);

  assert ( byte_read == CHUNK_SIZE );

  byte_read = client.read_chunk(TEST_FILE_CHUNK_HANDLE_2, DEBUG_CHUNK_VERSION_BEGIN,
                                buf + CHUNK_SIZE, 0, CHUNK_SIZE);

  assert( byte_read == CHUNK_SIZE );
  for (int i=0; i<CHUNK_SIZE * 2; ++i) {
    cout << buf[i] << endl;
  }
  cout << endl;

}

