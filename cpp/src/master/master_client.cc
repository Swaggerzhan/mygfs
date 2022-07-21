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
, channel_()
{
}

bool MasterClient::init() {
  brpc::ChannelOptions options;
  options.timeout_ms = 2000; // 2000ms
  int ret = channel_.Init(route_.c_str(), "", nullptr);
  if ( ret != 0 ) {
    LOG(ERROR) << "init channel error at: " << route_;
    return false;
  }
  LOG(INFO) << "init channel to: " << route_ << " ok";
  connected_.store(true, std::memory_order_relaxed);
  return true;
}

bool MasterClient::create_file(const std::string &filename,
                               uint64_t& chunk_handle,
                               std::string &primary_route,
                               std::vector<std::string> &secondaries_routes) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "not connect to: " << route_ << " yet!";
    return false;
  }

  brpc::Controller cntl;
  CreateFileArgs args;
  CreateFileReply reply;
  args.set_filename(filename);

  MasterServer_Stub stub(&channel_);
  stub.CreateFile(&cntl, &args, &reply, nullptr);

  if ( cntl.Failed() ) {
    LOG(ERROR) << "create_file failed at: " << route_;
    if ( cntl.IsCloseConnection() ) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }
  if ( reply.state() != state_ok ) {
    return false;
  }
  chunk_handle = reply.chunk_handle();
  primary_route = reply.primary_route();
  for (auto& r : reply.routes()) {
    secondaries_routes.push_back(r);
  }
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


bool MasterClient::file_info_at(const std::string &filename, uint32_t chunk_index,
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
    LOG(ERROR) << "file route info failed at: " << route_ << " because: " << cntl.ErrorText();
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

bool MasterClient::append_info(const std::string &filename,
                               std::string &primary,
                               std::vector<std::string> &secondaries,
                               uint32_t &chunk_index,
                               uint64_t &chunk_handle) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "not connect to: " << route_ << " yet!";
    return false;
  }
  brpc::Controller cntl;
  FindLeaseHolderArgs args;
  FindLeaseHolderReply reply;
  args.set_last(true);
  args.set_filename(filename);

  MasterServer_Stub stub(&channel_);
  stub.FindLeaseHolder(&cntl, &args, &reply, nullptr);
  if ( cntl.Failed() ) {
    LOG(ERROR) << "append_info failed at: " << route_ << " because: " << cntl.ErrorText();
    if ( cntl.IsCloseConnection() ) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }
  if ( reply.state() != state_ok ) {
    LOG(INFO) << "master server: " << route_ << " err: " << debug_string(reply.state());
    return false;
  }
  primary = reply.primary();
  for (auto& r: reply.secondaries()) {
    secondaries.push_back(r);
  }
  chunk_handle = reply.chunk_handle();
  chunk_index = reply.chunk_index();
  return true;
}

//bool MasterClient::file_info(const std::string &filename, std::map<uint64_t, uint64_t> &chunks,
//                             std::map<uint64_t, std::vector<std::string>> &routes) {
//  if ( !connected_.load(std::memory_order_relaxed) ) {
//    LOG(ERROR) << "not connect to: " << route_ << " yet!";
//    return false;
//  }
//
//  FileRouteInfoArgs args;
//  FileRouteInfoReply reply;
//  uint64_t chunk_index = 0;
//  args.set_filename(filename);
//  args.set_chunk_index(chunk_index);
//  MasterServer_Stub stub(&channel_);
//
//  while ( true ) {
//    brpc::Controller cntl;
//    stub.FileRouteInfo(&cntl, &args, &reply, nullptr);
//    if ( cntl.Failed() ) {
//      LOG(ERROR) << "file route info failed at: " << route_ << " because: " << cntl.ErrorText();
//      if ( cntl.IsCloseConnection() ) {
//        connected_.store(false, std::memory_order_relaxed);
//      }
//      return false;
//    }
//    if ( reply.state() == state_file_chunk_index_err ) {
//      return true;
//    }
//    if ( reply.state() != state_ok ) {
//      return false;
//    }
//    chunks[chunk_index] = reply.chunk_handle();
//    LOG(INFO) << "reply route size: " << reply.route_size();
//    std::vector<std::string> tmp;
//    for (auto it: reply.route()) {
//      tmp.push_back(it);
//    }
//    routes[reply.chunk_handle()] = tmp;
//    chunk_index += 1;
//    args.set_chunk_index(chunk_index);
//  }
//}


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

