//
// Created by swagger on 2022/5/29.
//
#include "client.h"
#include "src/util/conf.h"
#include "file_context.h"

#include <iostream>
using std::cout;
using std::endl;

namespace gfs {


struct Context {
public:
  Context(const std::string& name)
  : offset(0)
  , chunks()
  , routes()
  , filename(name)
  {}


public:
  std::string filename;
  int64_t offset;

  // chunk_index -> chunk_handle: UUID
  std::map<uint64_t, uint64_t> chunks;


  // chunk_handle -> route
  std::map<uint64_t, std::vector<std::string>> routes;
};


bool Client::make_context(const std::string& filename) {
  Context* c = new Context(filename);
  bool ret = master_.file_info(filename, c->chunks, c->routes);
  if ( !ret ) {
    LOG(ERROR) << "file info error";
    delete c;
    return false;
  }
  context_[filename] = c;
  return true;
}


Client::Client(const std::string &route)
: master_(route)
, file_context_map_()
, file_context_map_rw_lock_()
{
}


bool Client::init() {
  return master_.init();
}

void Client::make_context(const std::string& filename, FileContext** context_ret) {
  WriteLockGuard guard(&file_context_map_rw_lock_);
  FileContext* c = new (std::nothrow)FileContext(filename, &master_);
  file_context_map_[filename] = c;
  *context_ret = c;
}

Context* Client::get_context(const std::string &filename) {
  {
    ReadLockGuard guard(&context_rw_lock_);
    auto it = context_.find(filename);
    if ( it != context_.end() ) {
      return it->second;
    }
  }
  make_context(filename);
  ReadLockGuard guard(&context_rw_lock_);
  auto it = context_.find(filename);
  if ( it != context_.end() ) {
    return it->second;
  }
  return nullptr;
}

ChunkClient* Client::get_chunk_client(const std::string &route) {
  {
    ReadLockGuard guard(&chunk_servers_rw_lock_);
    auto it = chunk_servers_.find(route);
    if ( it != chunk_servers_.end() ) {
      return it->second;
    }
  }
  WriteLockGuard guard(&chunk_servers_rw_lock_);
  // check again !
  auto it = chunk_servers_.find(route);
  if ( it != chunk_servers_.end() ) {
    return it->second;
  }
  ChunkClient* client = new ChunkClient(route);
  if ( !client->init() ) {
    return nullptr;
  }
  chunk_servers_[route] = client;
  return client;

}


int64_t Client::read(const std::string &filename, char *buf, int64_t length) {

  Context* c = get_context(filename);
  if ( c == nullptr ) {
    return -1;
  }
  int64_t bytes_read = 0; // 总共真正读取的长度
  int64_t remain = length; // 需要读取的长度
  int64_t chunk_index = c->offset / CHUNK_SIZE; // 读取的块
  int64_t chunk_offset = c->offset % CHUNK_SIZE;  // 块内偏移量
  int64_t read_per_round = 0; // 每次循环读取量
  while ( remain != 0 ) {
    cout << "remain: " << remain << endl;
    if ( chunk_offset + remain < CHUNK_SIZE ) {
      read_per_round = remain;  // 一次读取完
      remain = 0;
    }else {
      read_per_round = CHUNK_SIZE - chunk_offset; // 读完当前块后还有剩余，需要到其他块里读取
      remain -= read_per_round;
    }
    cout << 1 << endl;
    uint64_t chunk_handle = 0;
    {
      auto it = c->chunks.find(chunk_index);
      if ( it == c->chunks.end() ) {
        // TODO: error
      }else {
        chunk_handle = it->second;
      }
    }
    cout << 2 << endl;
    auto it = c->routes.find(chunk_handle);
    if ( it == c->routes.end() ) {
      LOG(ERROR) << "empty route";
      // TODO:
    }
    LOG(INFO) << "route size: " << it->second.size();
    cout << 3 << endl;
    ChunkClient* chunk_client = nullptr;
    for (auto r : it->second ) { // 随机选取出一个chunk server，然后连接
      cout << "loop" << endl;
      chunk_client = get_chunk_client(r);
      if (chunk_client != nullptr ) {
        break;
      }
    }
    cout << 4 << endl;
    int64_t real_read_bytes = chunk_client->read_chunk(chunk_handle, DEBUG_CHUNK_VERSION_BEGIN,
                                                       buf + bytes_read,
                                                       chunk_offset,
                                                       read_per_round);
    cout << 5 << endl;
    if ( real_read_bytes != read_per_round ) {
      bytes_read += real_read_bytes;
      c->offset += real_read_bytes;
      return bytes_read;
    }
    cout << 6 << endl;
    // 准备下一次读取
    bytes_read += read_per_round;
    c->offset += read_per_round;
    ++ chunk_index;
    chunk_offset = c->offset % CHUNK_SIZE;
  }
  return bytes_read;
}

void Client::update_chunk_server(std::vector<std::string> &routes) {
  WriteLockGuard gurad(&chunk_servers_rw_lock_);

  for (auto it : routes ) {
    if ( chunk_servers_.find(it) != chunk_servers_.end() ) {
      continue;
    }
    // update
    ChunkClient* client = new ChunkClient(it);
    if ( !client->init() ) {
      // TODO: error
    }else {
      chunk_servers_[it] = client;
    }
  }
}


};



