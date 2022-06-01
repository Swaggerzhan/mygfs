//
// Created by swagger on 2022/5/30.
//
#include <cassert>
#include "src/master/master_client.h"
#include "src/util/conf.h"

int main() {
  gfs::MasterClient client(GFS_MASTER_SERVER_ROUTE);

  assert ( client.init() );
  client.print_files();

}

