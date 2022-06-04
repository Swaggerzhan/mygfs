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





// ******************************* DEBUG *********************************


void ChunkServerImpl::start_debug() {
  disk_.padding_debug_chunk();
}




};