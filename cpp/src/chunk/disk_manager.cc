//
// Created by swagger on 2022/5/29.
//

#include <unistd.h>
#include <fcntl.h>
#include "disk_manager.h"
#include "src/util/conf.h"

namespace gfs {


char* DiskManager::fetch_chunk(uint64_t chunk_handle, uint32_t version, uint64_t *chunk_size) {
  std::string filename;
  filename += GFS_CHUNK_SERVER_ROOT_DIR;
  filename += "/";
  filename += std::to_string(chunk_handle) + std::to_string(version);

  int fd = open(filename.c_str(), O_RDONLY);
  if ( fd < 0 ) {
    *chunk_size = -1;
    return nullptr;
  }

  // release by caller
  char* buf = new char[CHUNK_SIZE];
  uint64_t bytes_read = ::read(fd, buf, CHUNK_SIZE);
  *chunk_size = bytes_read;
  if ( bytes_read < 0 ) {
    return nullptr;
  }
  close(fd);
  return buf;
}

bool DiskManager::create_chunk(uint64_t chunk_handle, uint32_t version) {
  std::string filename;
  filename += GFS_CHUNK_SERVER_ROOT_DIR;
  filename += "/";
  filename += std::to_string(chunk_handle) + std::to_string(version);
  int fd = open(filename.c_str(), O_CREAT | O_RDWR);
  if ( fd < 0 ) {
    return false;
  }
  close(fd);
  return true;
}


#include <cassert>
// ****************************** DEBUG ******************************
void DiskManager::padding_debug_chunk() {
  std::string filename;
  filename += GFS_CHUNK_SERVER_ROOT_DIR;
  filename += "/";
  filename += std::to_string(DEBUG_CHUNK_UUID) + std::to_string(DEBUG_CHUNK_VERSION_BEGIN);

  int fd = open(filename.c_str(), O_CREAT | O_RDWR);
  assert( fd > 0 );
  char tmp[CHUNK_SIZE];
  for (int i=0; i<CHUNK_SIZE; ++i) {
    tmp[i] = '0' + (i % 10);
  }
  int ret = ::write(fd, tmp, CHUNK_SIZE);
  assert( ret > 0 );
}



};
