//
// Created by swagger on 2022/5/29.
//
#include "chunk_client.h"
#include "src/util/state_code.h"


namespace gfs {


ChunkClient::ChunkClient(const std::string &route, int chunk_client_id)
: route_(route)
, connected_(false)
, chunk_client_id_(chunk_client_id)
, chunk_server_id_(-1)
{}


ChunkClient::~ChunkClient() {

}

bool ChunkClient::init() {
  brpc::ChannelOptions options;
  options.timeout_ms = 2000;
  int ret = channel_.Init(route_.c_str(), "", &options);
  if ( ret != 0 ) {
    LOG(ERROR) << "channel init fail at route: " << route_;
    return false;
  }
  connected_.store(true, std::memory_order_relaxed);
  chunk_server_id_ = heartbeat();
  LOG(INFO) << "client_id: " << chunk_client_id_ << " connect to chunk server: "
  << chunk_server_id_;
  return true;
}

std::string ChunkClient::name() {
  return route_;
}

std::string ChunkClient::route() {
  return route_;
}

int ChunkClient::id() {
  if ( chunk_server_id_ == - 1) {
    chunk_server_id_ = heartbeat();
  }
  return chunk_server_id_;
}

int ChunkClient::self_id() {
  return chunk_client_id_;
}

bool ChunkClient::connected() {
  return connected_.load(std::memory_order_relaxed);
}

int64_t ChunkClient::read_chunk(uint64_t chunk_handle, uint32_t version,
                          char *buf, int64_t offset, int64_t length) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "channel disconnected at route: " << route_;
    return -1;
  }
  LOG(INFO) << 1;
  brpc::Controller cntl;
  ReadChunkArgs args;
  ReadChunkReply reply;

  // param set
  args.set_chunk_handle(chunk_handle);
  args.set_chunk_version(version);
  args.set_offset_start(offset);
  args.set_length(length);
  LOG(INFO) << 2 ;
  ChunkServer_Stub stub(&channel_);
  stub.ReadChunk(&cntl, &args, &reply, nullptr);
  LOG(INFO) << 3;
  if ( cntl.Failed() ) {
    LOG(ERROR) << "read failed at route: " << route_ << " because: " << cntl.ErrorText();
    if (cntl.IsCloseConnection()) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return -1;
  }
  LOG(INFO) << 4;
  if ( reply.state() != state_ok ) {
    LOG(INFO) << "server error at: " << route_ << " reply: " << debug_string(reply.state());
    return -1;
  }

  if ( reply.bytes_read() == 0 ) {
    return 0;
  }
  LOG(INFO) << "copy....";
  memcpy(buf, reply.mutable_data()->c_str(), reply.bytes_read());
  LOG(INFO) << "copy ok";
  return reply.bytes_read();
}

bool ChunkClient::put_data(uint64_t timestamp, const std::string &data) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "channel disconnected at route: " << route_;
    return -1;
  }
  brpc::Controller cntl;
  PutDataArgs args;
  PutDataReply reply;
  args.set_client_id(chunk_client_id_);
  args.set_timestamp(timestamp);
  args.set_data(data);

  ChunkServer_Stub stub(&channel_);
  stub.PutData(&cntl, &args, &reply, nullptr);

  if ( cntl.Failed() ) {
    LOG(ERROR) << "put data failed at route: " << route_ << " because: " << cntl.ErrorText();
    if (cntl.IsCloseConnection()) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }
  if ( reply.state() != state_ok ) {
    return false;
  }
  return true;
}

int64_t ChunkClient::write_chunk_commit(uint64_t timestamp,
                                        uint32_t version,
                                        uint64_t chunk_handle,
                                        int64_t offset) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "channel disconnected at route: " << route_;
    return -1;
  }
  brpc::Controller cntl;
  WriteChunkArgs args;
  WriteChunkReply reply;

  args.set_client_id(chunk_client_id_);
  args.set_timestamp(timestamp);
  args.set_version(version);
  args.set_chunk_handle(chunk_handle);
  args.set_offset(offset);

  ChunkServer_Stub stub(&channel_);
  stub.WriteChunk(&cntl, &args, &reply, nullptr);

  if ( cntl.Failed() ) {
    LOG(ERROR) << "write chunk failed at route: " << route_ << " because: " << cntl.ErrorText();
    if (cntl.IsCloseConnection()) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }
  if ( reply.state() == state_file_not_found ) {
    LOG(ERROR) << "chunk_handle or tmp data not found";
    return -1;
  }
  if ( reply.state() == state_length_err ) {
    LOG(ERROR) << "chunk_handle don't has enough length to do write commit";
    return -1;
  }
  return 1; // TODO: return length
}

int64_t ChunkClient::append_commit(uint64_t timestamp,
                                   uint32_t version,
                                   uint64_t chunk_handle,
                                   int64_t tmp_data_offset) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "channel disconnected at route: " << route_;
    return -1;
  }

  brpc::Controller cntl;
  AppendChunkArgs args;
  AppendChunkReply reply;
  args.set_client_id(chunk_client_id_);
  args.set_chunk_handle(chunk_handle);
  args.set_tmp_data_offset(tmp_data_offset);
  args.set_timestamp(timestamp);

  ChunkServer_Stub stub(&channel_);
  stub.AppendChunk(&cntl, &args, &reply, nullptr);

  if ( cntl.Failed() ) {
    LOG(ERROR) << "init chunk failed at route: " << route_ << " because: " << cntl.ErrorText();
    if (cntl.IsCloseConnection()) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }
  if ( reply.state() != state_ok ) {
    return -1;
  }
  return reply.bytes_written();
}

// ************************** Master call rpc *****************************
int ChunkClient::heartbeat() {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "channel disconnected at route: " << route_;
    return -1;
  }
  brpc::Controller cntl;
  HeartBeatArgs args;
  HeartBeatReply reply;
  args.set_echo(1);

  ChunkServer_Stub stub(&channel_);
  stub.HeartBeat(&cntl, &args, &reply, nullptr);
  if ( cntl.Failed() || reply.echo() == -1) {
    LOG(ERROR) << "heartbeat failed at route: " << route_ << " because: " << cntl.ErrorText();
    connected_.store(false, std::memory_order_relaxed);
    return -1;
  }
  return reply.id();
}

bool ChunkClient::init_chunk(uint64_t chunk_handle) {
  if ( !connected_.load(std::memory_order_relaxed) ) {
    LOG(ERROR) << "channel disconnected at route: " << route_;
    return -1;
  }

  brpc::Controller cntl;
  InitChunkArgs args;
  InitChunkReply reply;
  args.set_chunk_handle(chunk_handle);

  ChunkServer_Stub stub(&channel_);
  stub.InitChunk(&cntl, &args, &reply, nullptr);

  if ( cntl.Failed() ) {
    LOG(ERROR) << "init chunk failed at route: " << route_ << " because: " << cntl.ErrorText();
    if (cntl.IsCloseConnection()) {
      connected_.store(false, std::memory_order_relaxed);
    }
    return false;
  }
  if ( reply.state() != state_ok ) {
    LOG(INFO) << "server error at: " << route_ << " reply: " << debug_string(reply.state());
    return false;
  }
  return true;
}


};

