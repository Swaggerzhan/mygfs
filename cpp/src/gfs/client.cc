//
// Created by swagger on 2022/5/29.
//
#include "client.h"
#include "src/util/conf.h"
#include "file_context.h"

namespace gfs {


Client::Client(const std::string &route)
: master_(route)
, file_context_map_()
, file_context_map_rw_lock_()
{
}


bool Client::init() {
  return master_.init();
}

void Client::make_context(const std::string& filename) {
  WriteLockGuard guard(&file_context_map_rw_lock_);
  FileContext* context = new (std::nothrow)FileContext(filename, &master_);
  file_context_map_[filename] = context;
}


int64_t Client::read(const std::string &filename, char *buf, int64_t length) {
  FileContext* context = nullptr;
  {
    ReadLockGuard guard(&file_context_map_rw_lock_);
    auto it = file_context_map_.find(filename);
    if ( it != file_context_map_.end() ) {
      context = it->second;
    }
  }
  // 第一次读取
  if ( context == nullptr ) {
    make_context(filename);
  }

  // TODO: 文件大小？
  // 计算出需要读取多少个chunk
  uint32_t begin_chunk_index = context->offset_ / CHUNK_SIZE;
  uint32_t end_chunk_index = (context->offset_ + length) / CHUNK_SIZE;

  // 本次需要请求的chunk_handle UUID
  std::map<uint64_t, std::vector<std::string>> request_chunk_handle;

  bool ret = context->chunk_info_fetch(begin_chunk_index, end_chunk_index, request_chunk_handle);
  if ( !ret ) {
    // TODO: 终止？
    LOG(ERROR) << "some chunk index can't found";
  }

  uint32_t remain = length; // 剩余需要读取的
  int64_t total_read = 0;

  for (auto it : request_chunk_handle) {
    // 在本chunk中开始位置
    uint32_t cur_chunk_offset = context->offset_ % CHUNK_SIZE;
    uint32_t cur_chunk_need_to_read;
    if ( cur_chunk_offset + remain > CHUNK_SIZE ) {
      cur_chunk_need_to_read = CHUNK_SIZE - cur_chunk_offset;
    }else {
      cur_chunk_need_to_read = remain;
    }
    // read
    for (auto route: it.second) {
      auto iter = chunk_servers_.find(route);
      if ( iter == chunk_servers_.end()) {
        // TODO: update and fetch again
      }
      // TODO: version 暂定为1
      uint32_t bytes_read = iter->second->read(it.first, 1, buf,
                         context->offset_, cur_chunk_need_to_read);
      if ( bytes_read == 0 ) { // 结束
        remain = 0;
      }
      if ( bytes_read > 0 ) {
        buf = buf + bytes_read;
        context->offset_ += bytes_read; // 下一次读取的位置
        remain -= bytes_read;
        total_read += bytes_read;
        break;
      }
      // TODO: 所有节点都返回读取错误的处理
    } // for (auto route : it.second)

  }
  return total_read;
}


};



