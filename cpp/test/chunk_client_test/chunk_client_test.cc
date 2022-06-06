//
// Created by swagger on 2022/5/29.
//
#include <iostream>
#include "src/chunk/chunk_client.h"
#include "src/util/conf.h"
#include "src/util/time.h"

using namespace std;

/*
 * 读取TEST_FILE_CHUNK_HANDLE_1和TEST_FILE_CHUNK_HANDLE_2的测试
 */
void test_at_chunk_server_1();

/*
 * 测试 id()、init_chunk()、read_chunk()。
 */
void basic_test();

/**
 * @brief put_data测试，放2个临时tmp data
 */
void put_data_test();

/**
 * @brief 进行初始化chunk PutData, AppendChunk，ReadChunk的测试
 */
void write_read_test();



int main() {

  // test_at_chunk_server_1();
  // basic_test();
  // put_data_test();
  write_read_test();

}


void basic_test() {
  gfs::ChunkClient client(GFS_CHUNK_SERVER_1_ROUTE);
  assert ( client.init() );
  cout << "chunk server id: " << client.id() << endl;
  uint64_t chunk_handle = 3;
  cout << "create chunk: " << chunk_handle << endl;
  assert (client.init_chunk(chunk_handle) );
  // client.read_chunk()
}


/*
 * 读取TEST_FILE_CHUNK_HANDLE_1和TEST_FILE_CHUNK_HANDLE_2的测试
 */
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

void put_data_test() {
  std::string data1;
  std::string data2;
  data1.reserve(CHUNK_SIZE);
  data2.reserve(CHUNK_SIZE);
  for (int i=0; i<CHUNK_SIZE; ++i) {
    data1.push_back('1');
    data2.push_back('2');
  }
  gfs::ChunkClient c(GFS_CHUNK_SERVER_1_ROUTE);
  assert ( c.init() == true );
  uint64_t data1_time = gfs::now();
  assert ( c.put_data(data1_time, data1));
  uint64_t data2_time = gfs::now();
  assert ( c.put_data(data2_time, data2));
}


void write_read_test() {
  std::string data1;
  std::string data2;
  data1.reserve(CHUNK_SIZE);
  data2.reserve(CHUNK_SIZE);
  for (int i=0; i<CHUNK_SIZE; ++i) {
    data1.push_back('1');
    data2.push_back('2');
  }
  gfs::ChunkClient c(GFS_CHUNK_SERVER_1_ROUTE);
  assert ( c.init() == true );
  uint64_t data1_time = gfs::now();
  assert ( c.put_data(data1_time, data1) );
  uint64_t data2_time = gfs::now();
  assert ( c.put_data(data2_time, data2) );

  assert ( c.init_chunk(TEST_FILE_CHUNK_HANDLE_1) );
  assert ( c.init_chunk(TEST_FILE_CHUNK_HANDLE_2) );

  assert ( c.append_commit(data1_time, 1, TEST_FILE_CHUNK_HANDLE_1, 0) == CHUNK_SIZE );
  assert ( c.append_commit(data2_time, 1, TEST_FILE_CHUNK_HANDLE_2, 0) == CHUNK_SIZE );

  char* buf = new char[CHUNK_SIZE * 2 + 1];
  buf[CHUNK_SIZE*2] = '\0';

  assert ( c.read_chunk(TEST_FILE_CHUNK_HANDLE_1, 1, buf, 0, CHUNK_SIZE) == CHUNK_SIZE );
  assert ( c.read_chunk(TEST_FILE_CHUNK_HANDLE_2, 1, buf + CHUNK_SIZE, 0, CHUNK_SIZE) == CHUNK_SIZE );
  cout << buf << endl;
  cout << "write read test OK" << endl;
}

