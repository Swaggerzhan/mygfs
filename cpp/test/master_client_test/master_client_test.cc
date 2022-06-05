//
// Created by swagger on 2022/5/30.
//
#include <iostream>
#include <cassert>
#include "src/master/master_client.h"
#include "src/util/conf.h"

using namespace std;

/**
 * @brief 向MasterServer请求并创建一个文件出来
 * 打印得到的chunk server中primary和secondaries
 */
void create_file_test();

int main() {

  create_file_test();

}

void create_file_test() {
  gfs::MasterClient c(GFS_MASTER_SERVER_ROUTE);
  assert ( c.init() );

  uint64_t chunk_handle;
  std::string primary;
  std::vector<std::string> secondaries;

  assert ( c.create_file(TEST_FILE_NAME, chunk_handle, primary, secondaries) );

  cout << "got first chunk_handle: " << chunk_handle << endl;
  cout << "primary route: " << primary << endl;
  cout << "secondaries routes: ";
  for (auto& r : secondaries ) {
    cout << " " << r;
  }
  cout << endl;
}

