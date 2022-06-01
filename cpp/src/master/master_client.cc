//
// Created by swagger on 2022/5/29.
//

#include "src/util/state_code.h"
#include "master_client.h"
#include <iostream> // for debug print

namespace gfs {

MasterClient::MasterClient(const std::string &route)
: route_(route)
, connected_(false)
{
}

bool MasterClient::init() {
  brpc::ChannelOptions options;
  options.timeout_ms = 2000; // 2000ms
  int ret = channel_.Init(route_.c_str(), "", &options);
  if ( ret != 0 ) {
    LOG(ERROR) << "init channel error at: " << route_;
    return false;
  }
  connected_.store(true, std::memory_order_relaxed);
  return true;
}


bool MasterClient::list_file(std::vector<std::string> &ret) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "not connect to: " << route_ << " yet!";
    return false;
  }
  brpc::Controller cntl;
  ListFilesArgs args;
  ListFilesReply reply;

  args.set_prefix(""); // TODO: impl
  MasterServer_Stub stub(&channel_);

  stub.ListFiles(&cntl, &args, &reply, nullptr);

  if ( cntl.Failed() ) {
    LOG(ERROR) << "list_file failed at: " << route_;
    if ( cntl.IsCloseConnection() ) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }

  if ( reply.state() != state_ok ) {
    LOG(INFO) << "master server: " << route_ << " err: " << debug_string(reply.state());
    return false;
  }
  // 响应结果
  ret.clear();
  for (auto file : reply.filenames() ) {
    ret.push_back(file);
  }
  return true;
}


bool MasterClient::file_info_at(const std::string &filename, uint64_t chunk_index,
                                std::vector<std::string>& routes, uint64_t* chunk_handle) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "not connect to: " << route_ << " yet!";
    return false;
  }

  brpc::Controller cntl;
  FileRouteInfoArgs args;
  FileRouteInfoReply reply;

  args.set_filename(filename);
  args.set_chunk_index(chunk_index);

  MasterServer_Stub stub(&channel_);

  stub.FileRouteInfo(&cntl, &args, &reply, nullptr);
  if ( cntl.Failed() ) {
    LOG(ERROR) << "file route info failed at: " << route_;
    if ( cntl.IsCloseConnection() ) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }

  if ( reply.state() != state_ok ) {
    LOG(INFO) << "master server: " << route_ << " err: " << debug_string(reply.state());
    return false;
  }
  routes.clear();
  for (auto it: reply.route()) {
    routes.push_back(it);
  }
  *chunk_handle = reply.chunk_handle();
  return true;
}


// ******************* DEBUG *********************
#include <cassert>
using std::cout;
using std::endl;
void MasterClient::print_files() {
  std::vector<std::string> files;
  bool ret = list_file(files);

  assert ( ret == true );
  cout << "files: " << endl;
  for (auto it : files ) {
    cout << it << endl;
  }
}

};

