//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_DISK_MANAGER_H
#define MYGFS_DISK_MANAGER_H

#include "src/util/state_code.h"

namespace gfs {

class DiskManager {
public:

  DiskManager(int port);
  ~DiskManager() = default;


  /*
   * delete by caller for now
   * TODO: delete by callee
   */
  char* fetch_chunk(uint64_t chunk_handle, uint32_t version, uint64_t* chunk_size);

  bool create_chunk(uint64_t chunk_handle, uint32_t version);

  // *************************** DEBUG **********************************

  void padding_debug_chunk();


private:

  // TODO: LRU
  int port_;
  std::string root_;


};

};

#endif //MYGFS_DISK_MANAGER_H
