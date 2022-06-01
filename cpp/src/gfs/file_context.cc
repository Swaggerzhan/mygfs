//
// Created by swagger on 2022/5/29.
//

#include "file_context.h"
#include "src/master/master_client.h"

// for debug
#include <iostream>
using std::cout;
using std::endl;

namespace gfs {


FileContext::FileContext(const std::string &filename, MasterClient* client)
: offset_(0)
, filename_(filename)
, client_(client)
{

}

std::string FileContext::name() {
  return filename_;
}

bool FileContext::chunk_info_fetch(uint32_t begin_chunk_index, uint32_t end_chunk_index,
                                   std::map<uint64_t, std::vector<std::string>>& ret_routes) {
  std::vector<std::string> routes;
  uint64_t chunk_handle;
  for (int i=begin_chunk_index; i<=end_chunk_index; ++i) {
    // 没有对应chunk_index的路由信息，那就到Master那边获取
    if ( chunks_.find(i) == chunks_.end() )  {
      routes.clear();
      // TODO: 区分超出chunk的读取导致的错误
      LOG(INFO) << "fetch chunk index: " << i;
      bool ret = client_->file_info_at(filename_, i, routes, &chunk_handle);
      if ( !ret ) {
        LOG(ERROR) << "file info error";
        return false;
      }
      chunks_[i] = chunk_handle;
      chunk_routes_[chunk_handle] = routes; // deep copy
      ret_routes[chunk_handle] = routes; // deep copy
    }
  }
  return true;
}










};
