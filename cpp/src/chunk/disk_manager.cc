//
// Created by swagger on 2022/5/29.
//

#include <unistd.h>
#include <fcntl.h>
#include "brpc/server.h"
#include "disk_manager.h"
#include "src/util/conf.h"

namespace gfs {

DiskManager::DiskManager(int port)
: port_(port)
{
  root_ += GFS_CHUNK_SERVER_ROOT_DIR;
  root_ += std::to_string(port);
  LOG(INFO) << "chunk server root at: " << root_;
}

char* DiskManager::fetch_chunk(uint64_t chunk_handle, uint32_t version, uint64_t *chunk_size) {
  std::string filename;
  filename += root_;
  filename += "/";
  filename += std::to_string(chunk_handle) + std::to_string(DEBUG_CHUNK_VERSION_BEGIN);

  int fd = open(filename.c_str(), O_RDONLY);
  if ( fd < 0 ) {
    *chunk_size = -1;
    LOG(ERROR) << "open file: " << filename << " failed";
    return nullptr;
  }

  // release by caller
  char* buf = new char[CHUNK_SIZE];
  uint64_t bytes_read = ::read(fd, buf, CHUNK_SIZE);
  *chunk_size = bytes_read;
  if ( bytes_read < 0 ) {
    LOG(ERROR) << "read from file: " << filename << " failed";
    return nullptr;
  }
  close(fd);
  return buf;
}

bool DiskManager::create_chunk(uint64_t chunk_handle, uint32_t version) {
  std::string filename;
  filename += root_;
  filename += "/";
  filename += std::to_string(chunk_handle) + std::to_string(version);
  int fd = open(filename.c_str(), O_CREAT | O_RDWR);
  if ( fd < 0 ) {
    return false;
  }
  close(fd);
  return true;
}


// ****************************** DEBUG ******************************
#include <cassert>

static void padding_gfs_test_file(const std::string& root) {
  LOG(INFO) << "padding gfs test file";
  std::string filename;
  filename += root;
  filename += "/";
  filename += std::to_string(TEST_FILE_CHUNK_HANDLE_1) + std::to_string(DEBUG_CHUNK_VERSION_BEGIN);
  int fd = open(filename.c_str(), O_CREAT | O_RDWR );
  assert( fd > 0 );
  char tmp[CHUNK_SIZE];
  for (int i=0; i<CHUNK_SIZE; ++i) {
    tmp[i] = '0';
  }
  int ret = ::write(fd, tmp, CHUNK_SIZE);
  assert(ret == CHUNK_SIZE);
  LOG(INFO) << "PADDING :" << filename << " OK";
  close(fd);
  filename.clear();
  filename += root;
  filename += "/";
  filename += std::to_string(TEST_FILE_CHUNK_HANDLE_2) + std::to_string(DEBUG_CHUNK_VERSION_BEGIN);
  fd = open(filename.c_str(), O_CREAT | O_RDWR );
  assert( fd > 0 );
  for (int i=0; i<CHUNK_SIZE; ++i) {
    tmp[i] = '1';
  }
  ret = ::write(fd, tmp, CHUNK_SIZE);
  assert( ret == CHUNK_SIZE);
  LOG(INFO) << "PADDING :" << filename << " OK";
  close(fd);
}



void DiskManager::padding_debug_chunk() {
  LOG(INFO) << "disk padding debug chunk...";
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


  // for gfs_test_file
  padding_gfs_test_file(root_);

}




};
