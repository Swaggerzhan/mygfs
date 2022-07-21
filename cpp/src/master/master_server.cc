//
// Created by swagger on 2022/5/29.
//
#include "master_server.h"
#include "brpc/server.h"
#include "src/util/state_code.h"
#include "src/util/conf.h"
#include "chunk_route.h"

namespace gfs {

MasterServerImpl::MasterServerImpl()
: files_()
, files_mutex_()
{
}

MasterServerImpl::~MasterServerImpl() noexcept {
}

void MasterServerImpl::start() {
}

void MasterServerImpl::CreateFile(google::protobuf::RpcController *cntl,
                                  const CreateFileArgs *args,
                                  CreateFileReply *reply,
                                  google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  std::unique_lock<std::mutex> lock_guard(files_mutex_);
  auto it = files_.find(args->filename()); // TODO: impl file sys
  if ( it != files_.end() ) {
    reply->set_state(state_err);
    return;
  }
  FileInfoPtr ptr(new FileInfo(args->filename()));
  if ( !ptr->init(reply) ) {
    reply->set_state(state_err);
    return;
  }
  reply->set_state(state_ok);
}

void MasterServerImpl::ListFiles(google::protobuf::RpcController *cntl,
                                 const ListFilesArgs *args,
                                 ListFilesReply *reply,
                                 google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  // TODO: use prefix

  std::unique_lock<std::mutex> lock_guard(files_mutex_);

  for (const auto& it : files_ ) {
    reply->add_filenames(it.first);
  }
  reply->set_state(state_ok);
}


void MasterServerImpl::FileRouteInfo(google::protobuf::RpcController *cntl,
                                     const FileRouteInfoArgs *args,
                                     FileRouteInfoReply *reply,
                                     google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  FileInfoPtr file_info;
  {
    std::unique_lock<std::mutex> lock_guard(files_mutex_);
    auto it = files_.find(args->filename());
    if (it == files_.end()) {
      reply->set_state(state_file_not_found);
      return;
    }
    file_info = it->second;
  }

  if ( file_info == nullptr ) {
    // FATAL_ERROR
  }
  if (!file_info->read_info(args->chunk_index(), reply)) {
    reply->set_state(state_err);
    return;
  }
  reply->set_state(state_ok);
}

void MasterServerImpl::FindLeaseHolder(google::protobuf::RpcController *cntl,
                                       const FindLeaseHolderArgs *args,
                                       FindLeaseHolderReply *reply,
                                       google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  FileInfoPtr file_info;
  {
    std::unique_lock<std::mutex> lock_guard(files_mutex_);
    auto it = files_.find(args->filename());
    if (it == files_.end()) {
      reply->set_state(state_file_not_found);
      return;
    }
    file_info = it->second;
  }

  if ( args->last() ) {
    file_info->append_info(args, reply);
    return;
  }

  if (!file_info->write_info(args->chunk_index(), reply)) {
    reply->set_state(state_err);
    return;
  }
  reply->set_state(state_ok);
}



// ******************* DEBUG *************************
void MasterServerImpl::start_debug() {
//  // 连接至所有的chunk server
//  ChunkClient* client1 = new ChunkClient(GFS_CHUNK_SERVER_1_ROUTE);
//  ChunkClient* client2 = new ChunkClient(GFS_CHUNK_SERVER_2_ROUTE);
//  ChunkClient* client3 = new ChunkClient(GFS_CHUNK_SERVER_3_ROUTE);
//
//  if (client1->init() && client1->heartbeat()) {
//    chunk_servers_[GFS_CHUNK_SERVER_1_ROUTE] = client1;
//  }else {
//    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_1_ROUTE << " failed";
//  }
//
//  if (client2->init() && client2->heartbeat()) {
//    chunk_servers_[GFS_CHUNK_SERVER_2_ROUTE] = client2;
//  }else {
//    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_2_ROUTE << " failed";
//  }
//
//  if (client3->init() && client3->heartbeat()) {
//    chunk_servers_[GFS_CHUNK_SERVER_3_ROUTE] = client3;
//  }else {
//    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_3_ROUTE << " failed";
//  }
//
//  padding_test_file();

  // 连接到chunk server
  debug_start_connect();
}

void MasterServerImpl::padding_test_file() {
//  for (auto it : chunk_servers_) {
//    bool ret = it.second->init_chunk(TEST_FILE_CHUNK_HANDLE_1);
//    if ( !ret ) {
//      LOG(ERROR) << "some error happen";
//    }
//    ret = it.second->init_chunk(TEST_FILE_CHUNK_HANDLE_2);
//    if ( !ret ) {
//      LOG(ERROR) << "some error happen";
//    }
//  }
//  std::vector<uint64_t> chunks = {TEST_FILE_CHUNK_HANDLE_1, TEST_FILE_CHUNK_HANDLE_2};
//  files_.emplace(TEST_FILE_NAME, chunks);
//  std::vector<std::string> routes = {GFS_CHUNK_SERVER_1_ROUTE, GFS_CHUNK_SERVER_2_ROUTE,
//                                     GFS_CHUNK_SERVER_3_ROUTE};
//  chunk_route_info_[TEST_FILE_CHUNK_HANDLE_1] = routes;
//  chunk_route_info_[TEST_FILE_CHUNK_HANDLE_2] = routes;

}

}; // namespace gfs

