//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_DISK_MANAGER_H
#define MYGFS_DISK_MANAGER_H

#include "src/util/state_code.h"

namespace gfs {

class DiskManager {
public:

  DiskManager() = default;
  ~DiskManager() = default;


  /*
   * delete by caller for now
   * TODO: delete by callee
   */
  char* fetch_chunk(uint64_t chunk_handle, uint32_t version, uint64_t* chunk_size);


  // *************************** DEBUG **********************************

  void padding_debug_chunk();


private:

  // TODO: LRU


};

};

#endif //MYGFS_DISK_MANAGER_H
