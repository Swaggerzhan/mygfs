//
// Created by swagger on 2022/5/29.
//
#include "chunk_server.h"
#include "brpc/server.h"

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

  uint64_t chunk_size;
  char* mem = disk_.fetch_chunk(args->chunk_handle(),
                                args->chunk_version(), &chunk_size);
  if ( mem == nullptr ) {
    reply->set_state(state_err);
    return;
  }

  // 超出长度，没有任何东西可以读取
  if ( args->offset_start() >= chunk_size ) {
    reply->set_state(state_ok);
    reply->set_bytes_read(0);
    return;
  }
  // 未读取的剩下部分
  uint64_t remain_length = chunk_size - args->offset_start();
  if ( remain_length - args->length() > 0 ) { // 读取完还有剩余
    std::string data(mem + args->offset_start(), args->length());
    reply->set_data(data);
    reply->set_bytes_read(args->length());
  }else { // 全部读取
    std::string data(mem + args->offset_start(), remain_length);
    reply->set_data(data);
    reply->set_bytes_read(remain_length);
  }
  reply->set_state(state_ok);
  // TODO: delete mem
}

// ****************************** Master call rpc ************************
void ChunkServerImpl::HeartBeat(google::protobuf::RpcController *cntl,
                                const HeartBeatArgs *args,
                                HeartBeatReply *reply,
                                google::protobuf::Closure *done) {
  brpc::ClosureGuard guard(done);
  reply->set_echo(args->echo());
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