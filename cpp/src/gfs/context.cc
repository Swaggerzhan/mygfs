//
// Created by swagger on 2022/6/5.
//
#include "context.h"
#include "src/util/conf.h"
#include "src/master/chunk_route.h"
#include "src/chunk/chunk_client.h"
#include "src/util/time.h"

namespace gfs {

Context::Context(const std::string &filename, MasterClient *m)
: filename_(filename)
, m_(m)
, read_offset_(0)
{}

void Context::read_update() {
  for (uint32_t chunk_index = 0; ; ++chunk_index) {
    ChunkInfoPtr ptr(new ChunkInfo);
    bool ret = m_->file_info_at(filename_, chunk_index,
                     ptr->secondaries, &ptr->chunk_handle);
    if ( !ret ) {
      break;
    }
    chunks_[chunk_index] = ptr;
  }
}

bool Context::append_update(uint32_t& chunk_index) {
  ChunkInfoPtr ptr(new ChunkInfo);
  bool ret = m_->append_info(filename_, ptr->primary,
                             ptr->secondaries, chunk_index,
                             ptr->chunk_handle);
  if ( !ret ) {
    return false;
  }
  chunks_[chunk_index] = ptr;
  return true;
}

int64_t Context::do_chunk_read(char *buf, int64_t length) {
  // 真实读取长度
  int64_t bytes_need_to_read;
  // chunk内偏移
  int64_t chunk_in_offset = read_offset_ % CHUNK_SIZE;
  if ( chunk_in_offset + length < CHUNK_SIZE ) {
    bytes_need_to_read = length;
  }else {
    bytes_need_to_read = CHUNK_SIZE - chunk_in_offset;
  }
  uint32_t chunk_index = read_offset_ / CHUNK_SIZE;
  auto it = chunks_.find(chunk_index);
  // 仅当本地没有缓存时才会进行read_update
  if ( it == chunks_.end() ) {
    read_update();
  }
  // 还是没有找到路由信息，则直接返回错误
  it = chunks_.find(chunk_index);
  if ( it == chunks_.end() ) {
    return -1;
  }
  ChunkInfoPtr chunk_info_ptr = it->second;
  ChunkClientPtr chunk_client_ptr;
  // 随机找出一个副本读取
  bool ok = false;
  for (auto& r: chunk_info_ptr->secondaries ) {
    if ( fetch_chunk_server(r, chunk_client_ptr) ) {
      ok = true;
      break;
    }
  }
  if ( !ok ) {
    return -1;
  }
  return chunk_client_ptr->read_chunk(chunk_info_ptr->chunk_handle,
                               1, buf,
                               chunk_in_offset,
                               bytes_need_to_read);
}

int64_t Context::do_append(char *buf, int64_t length) {
  uint32_t chunk_index;
  if ( !append_update(chunk_index) ) {
    return -1;
  }

  auto it = chunks_.find(chunk_index);
  if ( it == chunks_.end() ) {
    return -1;
  }
  ChunkInfoPtr chunk_info = it->second;
  ChunkClientPtr chunk_client_ptr;
  if ( !fetch_chunk_server(chunk_info->primary, chunk_client_ptr) ) {
    return -1;
  }
  uint64_t timestamp = now();
  // 先将数据放到服务器上
  if ( !chunk_client_ptr->put_data(timestamp, std::string(buf, length)) ) {
    return -1;
  }



}

}; // namespace gfs

