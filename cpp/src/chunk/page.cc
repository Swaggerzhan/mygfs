//
// Created by swagger on 2022/6/4.
//
#include <fcntl.h>
#include <cassert>
#include <sys/mman.h>
#include <unistd.h>
#include "src/util/conf.h"
#include "page.h"

namespace gfs {

int Page::kProt_ = PROT_READ | PROT_WRITE;
int Page::kFlags_ = MAP_PRIVATE;

Page::Page()
: mem_(nullptr)
, path_()
, valid_(false)
, has_modify_(false)
{
}

Page::~Page() {
  if (!valid_) {
    return;
  }
  if ( !has_modify_ ){ // sync
    int ret = msync(mem_, CHUNK_SIZE, kFlags_);
    if ( ret < 0 ) {
      // FIXME:some error
    }
  }
  int ret = munmap(mem_, CHUNK_SIZE);
  if ( ret < 0 ) {
    // FIXME: some error
  }
}

bool Page::init(const std::string &path) {
  path_ = path;
  int fd = open(path.c_str(), O_CREAT | O_RDWR );
  assert ( fd > 0 );
  mem_ = mmap(nullptr, CHUNK_SIZE, kProt_, kFlags_, fd, 0);
  if ( mem_ == (void*)(MAP_FAILED) ) {
    close(fd);
    return false;
  }
  close(fd);
  valid_ = true;
  return true;
}

char* Page::write_expose() {
  has_modify_ = true;
  return (char*)mem_;
}

const char* Page::read_expose() {
  return (const char*)mem_;
}

void Page::flush() {
  msync(mem_, CHUNK_SIZE, kFlags_);
}

}; // namespace gfs
