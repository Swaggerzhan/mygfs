//
// Created by swagger on 2022/5/29.
//
#include "chunk_server.h"
#include "brpc/server.h"
#include "src/util/conf.h"

namespace gfs {


ChunkServerImpl::ChunkServerImpl(int port)
: port_(port)
, disk_(port)
{

}

ChunkServerImpl::~ChunkServerImpl() noexcept {

}

void ChunkServerImpl::ReadChunk(google::protobuf::RpcController *cntl,
                                const ReadChunkArgs *args,
                                ReadChunkReply *reply,
                                google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  PagePtr page_ptr;
  if ( !disk_.fetch_chunk(args->chunk_handle(), args->chunk_version(), page_ptr)) {
    reply->set_state(state_file_not_found);
    return;
  }
  const char* mem = page_ptr->read_expose();
  if ( args->offset_start() >= CHUNK_SIZE) { // 已经超出CHUNK_SIZE长度了
    reply->set_state(state_ok);
    reply->set_bytes_read(0);
    return;
  }
  // 计算出需要读取的长度
  // TODO: 保存长度
  int64_t byte_need_read;
  if ( args->offset_start() + args->length() < CHUNK_SIZE ) {
    byte_need_read = args->length();
  }else {
    byte_need_read = CHUNK_SIZE - args->offset_start();
  }
  std::string data(mem + args->offset_start(), byte_need_read);
  reply->set_data(data);
  reply->set_bytes_read(byte_need_read);
  reply->set_state(state_ok);
}

void ChunkServerImpl::PutData(google::protobuf::RpcController *cntl,
                              const PutDataArgs *args,
                              PutDataReply *reply,
                              google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  if (!disk_.store_tmp_data(args->client_id(), args->timestamp(),
                       args->data().c_str(), args->data().length()) ) {
    reply->set_state(state_err);
    return;
  }
  reply->set_state(state_ok);
}

void ChunkServerImpl::AppendChunk(google::protobuf::RpcController *cntl,
                                  const AppendChunkArgs *args,
                                  AppendChunkReply *reply,
                                  google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);
  LOG(INFO)  << "AppendChunk...";
  int64_t bytes_written;
  state_code ret = disk_.append_chunk(args->client_id(),
                                      args->timestamp(),
                                      args->chunk_handle(), 1,
                                      args->tmp_data_offset(),
                                      bytes_written);
  reply->set_state(ret);
  reply->set_bytes_written(bytes_written);
  LOG(INFO) << "append state: " << debug_string(ret) << " bytes written: " << bytes_written;
  // TODO: commit to other chunk
}

void ChunkServerImpl::WriteChunk(google::protobuf::RpcController *cntl,
                                 const WriteChunkArgs *args,
                                 WriteChunkReply *reply,
                                 google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  state_code ret = disk_.write_chunk_commit(args->client_id(),
                           args->timestamp(),
                           args->chunk_handle(),
                           args->version(),
                           args->offset());
  reply->set_state(ret);
}

// ****************************** Master call rpc ************************
void ChunkServerImpl::HeartBeat(google::protobuf::RpcController *cntl,
                                const HeartBeatArgs *args,
                                HeartBeatReply *reply,
                                google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);
  reply->set_echo(args->echo());
  reply->set_id(port_);
}

void ChunkServerImpl::InitChunk(google::protobuf::RpcController *cntl,
                                const InitChunkArgs *args,
                                InitChunkReply *reply,
                                google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);
  bool ret = disk_.create_chunk(args->chunk_handle(), 1);
  if ( !ret ) {
    reply->set_state(state_err);
  }else {
    reply->set_state(state_ok);
  }
}

void ChunkServerImpl::MarkPrimaryChunk(google::protobuf::RpcController *cntl,
                                       const MarkPrimaryChunkArgs *args,
                                       MarkPrimaryChunkReply *reply,
                                       google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);

  if ( !disk_.mark_as_primary(args->chunk_handle(), 1, args->expired()) ) {
    reply->set_state(state_err);
    return;
  }
  reply->set_state(state_ok);
}





// ******************************* DEBUG *********************************


void ChunkServerImpl::start_debug() {
  disk_.padding_debug_chunk();
}




};