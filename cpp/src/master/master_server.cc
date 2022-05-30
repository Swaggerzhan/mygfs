//
// Created by swagger on 2022/5/29.
//
#include "master_server.h"
#include "brpc/server.h"
#include "src/util/state_code.h"
#include "src/util/conf.h"

namespace gfs {


MasterServerImpl::MasterServerImpl()
: files_()
, files_rw_lock_()
, chunk_info_rw_lock_()
, chunk_route_info_()
, chunk_servers_()
, lease_info_()
{
}

MasterServerImpl::~MasterServerImpl() noexcept {

}

void MasterServerImpl::start() {



}

void MasterServerImpl::ListFiles(google::protobuf::RpcController *cntl,
                                 const ListFilesArgs *args,
                                 ListFilesReply *reply,
                                 google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  // TODO: use prefix


  ReadLockGuard read_guard(&files_rw_lock_);

  for (const auto it : files_ ) {
    std::string* target = reply->add_filenames();
    *target = it.first;
  }
  reply->set_state(state_ok);

}


void MasterServerImpl::FileRouteInfo(google::protobuf::RpcController *cntl,
                                     const FileRouteInfoArgs *args,
                                     FileRouteInfoReply *reply,
                                     google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  bool has_file = false;
  bool chunk_index_ok = false; // 是否超出预期值
  uint64_t chunk_handle;
  {
    ReadLockGuard read_guard(&files_rw_lock_);
    auto it = files_.find(args->filename());
    if ( it != files_.end() ) {
      has_file = true;
      if ( args->chunk_index() < it->second.size() ) {
        chunk_index_ok = true;
        chunk_handle = it->second[args->chunk_index()];
      }
    }
  }
  // 文件不存在
  if ( !has_file ) {
    reply->set_state(state_file_not_found);
    return;
  }
  // 文件index错误
  if ( !chunk_index_ok ) {
    reply->set_state(state_file_chunk_index_err);
    return;
  }

  ReadLockGuard read_guard(&chunk_info_rw_lock_);
  auto it = chunk_route_info_.find(chunk_handle);
  if ( it == chunk_route_info_.end() ) {
    reply->set_state(state_err);
    return;
  }

  // TODO: fetch route info from ChunkClientPtr by it


  reply->set_state(state_ok);
}



// ******************* DEBUG *************************
void MasterServerImpl::start_debug() {
  // 连接至所有的chunk server
  ChunkClient* client1 = new ChunkClient(GFS_CHUNK_SERVER_1_ROUTE);
  ChunkClient* client2 = new ChunkClient(GFS_CHUNK_SERVER_2_ROUTE);
  ChunkClient* client3 = new ChunkClient(GFS_CHUNK_SERVER_3_ROUTE);

  if (client1->init() && client1->heartbeat()) {
    chunk_servers_[GFS_CHUNK_SERVER_1_ROUTE] = client1;
  }else {
    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_1_ROUTE << " failed";
  }

  if (client2->init() && client2->heartbeat()) {
    chunk_servers_[GFS_CHUNK_SERVER_2_ROUTE] = client2;
  }else {
    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_2_ROUTE << " failed";
  }

  if (client3->init() && client3->heartbeat()) {
    chunk_servers_[GFS_CHUNK_SERVER_3_ROUTE] = client3;
  }else {
    LOG(ERROR) << "connect to route: " << GFS_CHUNK_SERVER_3_ROUTE << " failed";
  }

  padding_test_file();

}

void MasterServerImpl::padding_test_file() {
  for (auto it : chunk_servers_) {
    bool ret = it.second->init_chunk(TEST_FILE_CHUNK_HANDLE_1);
    if ( !ret ) {
      LOG(ERROR) << "some error happen";
    }
    ret = it.second->init_chunk(TEST_FILE_CHUNK_HANDLE_2);
    if ( !ret ) {
      LOG(ERROR) << "some error happen";
    }
  }
  std::vector<uint64_t> chunks = {TEST_FILE_CHUNK_HANDLE_1, TEST_FILE_CHUNK_HANDLE_2};
  files_.emplace(TEST_FILE_NAME, chunks);
  std::vector<std::string> routes = {GFS_CHUNK_SERVER_1_ROUTE, GFS_CHUNK_SERVER_2_ROUTE,
                                     GFS_CHUNK_SERVER_3_ROUTE};
  chunk_route_info_[TEST_FILE_CHUNK_HANDLE_1] = routes;
  chunk_route_info_[TEST_FILE_CHUNK_HANDLE_2] = routes;

}

}; // namespace gfs

