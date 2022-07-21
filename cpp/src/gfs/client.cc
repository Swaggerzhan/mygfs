//
// Created by swagger on 2022/5/29.
//
#include "client.h"
#include "src/util/conf.h"
#include "src/master/chunk_route.h"

#include <iostream>
using std::cout;
using std::endl;

namespace gfs {


//struct Context {
//public:
//
//  struct ChunkInfo {
//
//  };
//
//
//  Context(const std::string& name)
//  : offset(0)
//  , chunks()
//  , routes()
//  , filename(name)
//  {}
//
//  /**
//   * @brief 通过master和chunk_index来获取响应的chunk_handle
//   * 同时，进行route缓存，缓存至Context结构体中
//   * @param master
//   * @param chunk_index
//   * @param chunk_handle
//   * @return true 为成功
//   */
//  bool get_chunk_handle(MasterClient* master, uint32_t chunk_index, uint64_t& chunk_handle) {
//    auto it = chunks.find(chunk_index);
//    if ( it != chunks.end() ) {
//      chunk_handle = it->second;
//      return true;
//    }
//    if ( !master->file_info(filename, chunks, routes) ) {
//      return false;
//    }
//    it = chunks.find(chunk_index);
//    if ( it == chunks.end() ) {
//      return false;
//    }
//    chunk_handle = it->second;
//    return true;
//  }
//
//  bool get_chunk_client(uint64_t chunk_handle, ChunkClientPtr& ptr) {
//    auto it = routes.find(chunk_handle);
//    if ( it == routes.end() ) {
//      return false;
//    }
//    for (auto& r: it->second) {
//      // 仅需要成功一次即可
//      if ( fetch_chunk_server(r, ptr) ) {
//        return true;
//      }
//    }
//    return true;
//  }
//
//  /**
//   * @brief 找出当前文件中最后的一个chunk_index对应的chunk_handle
//   * @param master: MasterServer rpc桩
//   * @param chunk_handle: 响应的chunk_handle
//   * @return: true 为成功
//   */
//  bool last_chunk_handle(MasterClient* master, uint64_t& chunk_handle) {
//    if ( !master->file_info(filename, chunks, routes) ) {
//      return false;
//    }
//    uint32_t last_chunk_index = 0;
//    for (auto it : chunks) {
//      if ( it.first > last_chunk_index ) {
//        last_chunk_index = it.first;
//      }
//    }
//    chunk_handle = last_chunk_index;
//    return true;
//  }
//
//
//public:
//  std::string filename;
//  int64_t offset;
//
//  // chunk_index -> chunk_handle: UUID
//  std::map<uint32_t, uint64_t> chunks;
//
//  // chunk_handle -> route
//  std::map<uint64_t, std::vector<std::string>> routes;
//};


Client::Client(const std::string &route)
: master_(route)
{
}


bool Client::init() {
  return master_.init();
}

// ***************** PRIVATE MAKE CONTEXT *****************

bool Client::make_context(const std::string& filename, ContextPtr& ptr) {
  // TODO: impl
  return true;
}

bool Client::get_context(const std::string &filename, ContextPtr& ptr) {
  std::unique_lock<std::mutex> lock_guard(context_mutex_);
  auto it = context_.find(filename);
  if ( it != context_.end() ) {
    ptr = it->second;
    return true;
  }
  // 创建
  ptr.reset(new Context(filename, &master_));
  context_[filename] = ptr;
  return true;
}


int64_t Client::read(const std::string &filename, char *buf, int64_t length) {
  ContextPtr ptr;
  if ( !get_context(filename, ptr)) {
    return -1;
  }

  int64_t remain = length;
  int64_t total_read = 0; // 总计读取
  while ( remain != 0 ) {
    int64_t bytes_read = ptr->do_chunk_read(buf + ptr->read_offset_, remain);
    if ( bytes_read <= 0 ) {
      LOG(ERROR) << "filename: " << filename
      << " read error happened at index: " << ptr->read_offset_
      << " want read length: " << remain;
      break;
    }
    ptr->read_offset_ += bytes_read;
    total_read += total_read;
    remain -= bytes_read;
  }
  return total_read;
}

int64_t Client::read2(const std::string &filename, char *buf, int64_t length) {
//  ContextPtr ptr;
//  if ( !get_context(filename, ptr) ) {
//    return -1;
//  }
//  int64_t total_read = 0;
//  int64_t remain = length;
//  int64_t round_need_read; // 每一次循环需要读取的长度
//
//  while ( remain != 0 ) {
//    uint32_t chunk_index_begin = ptr->offset / CHUNK_SIZE;
//    int64_t chunk_offset = ptr->offset % CHUNK_SIZE; // chunk 内读取的进度
//
//    if ( chunk_offset + remain < CHUNK_SIZE ) { // 本次CHUNK都读不完
//      round_need_read = remain;
//    }else {
//      round_need_read = CHUNK_SIZE - chunk_offset; // 读完本次CHUNK都还有剩余
//    }
//    remain -= round_need_read; // 扣去本次读取的
//    uint64_t chunk_handle;
//    if (!ptr->get_chunk_handle(&master_, chunk_index_begin, chunk_handle)) {
//      // 错误发生，无法找到对应的chunk_handle，提前终止
//      return total_read;
//    }
//    ChunkClientPtr chunk_ptr;
//    if (!ptr->get_chunk_client(chunk_handle, chunk_ptr)) {
//      // 错误，连接到chunk_server
//      return total_read;
//    }
//    int64_t real_read = chunk_ptr->read_chunk(chunk_handle, 1, buf + ptr->offset,
//                          chunk_offset, round_need_read);
//    // 记录本次所读
//    total_read += real_read;
//    ptr->offset += real_read;
//    // chunk server中没有那么多数据，提前返回了
//    if ( real_read != round_need_read ) {
//      return total_read;
//    }
//  }
//  return total_read;
}

int64_t Client::append(const std::string &filename, char *buf, int64_t length) {
  ContextPtr ptr;
  if ( !get_context(filename, ptr) ) {
    return -1;
  }



}

//bool Client::pick_read_chunk_ptr(const ContextPtr &ptr, uint32_t chunk_index, ChunkClientPtr &ret) {
//  auto it = ptr->chunks.find(chunk_index);
//  if ( it == ptr->chunks.end() ) { // fetch from master
//    if ( !master_.file_info(ptr->filename, ptr->chunks, ptr->routes) ) {
//      LOG(ERROR) << "fetch from master failed";
//      return false;
//    }
//    it = ptr->chunks.find(chunk_index);
//  }
//  // 成功了，但MasterServer确实没有相关信息，则失败
//  if ( it == ptr->chunks.end() ) {
//    return false;
//  }
//  auto r = ptr->routes.find(it->second);
//
//}

};
