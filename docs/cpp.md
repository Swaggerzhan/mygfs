<font face="Monaco">

# 本项目中关于gfs的实现(cpp版本)

本项目是一个关于google file system的实现，一个“玩具”实现，并不考虑工程问题，更多的是为了深入理解gfs的运作方式而写，项目中CHUNK_SIZE并不是和论文中64MB那样，之所以使用64KB是为了更好的调试(64MB实在太占空间了)。

依赖：

[brpc v0.9.7](https://github.com/apache/incubator-brpc/tree/0.9.7)

[protobuf v3.19.3](https://github.com/protocolbuffers/protobuf/tree/3.19.x)

## 0x00 目录结构

src中是主要源码，src/chunk下为chunk server的源码，src/master为master server的源码，src/gfs是整个项目客户端的封装，test里则是测试代码。

```shell
cpp
├── CMakeLists.txt
├── proto
│   ├── gfs.proto
│   └── out
├── script
│   ├── kill_chunk_server.sh
│   ├── run.sh
│   └── start_chunk_server.sh
├── src
│   ├── chunk
│   │   ├── chunk_client.cc
│   │   ├── chunk_client.h
│   │   ├── chunk_server.cc
│   │   ├── chunk_server.h
│   │   ├── disk_manager.cc
│   │   ├── disk_manager.h
│   │   ├── page.cc
│   │   └── page.h
│   ├── gfs
│   │   ├── client.cc
│   │   ├── client.h
│   │   ├── context.cc
│   │   ├── context.h
│   │   ├── file_context.cc
│   │   └── file_context.h
│   ├── master
│   │   ├── chunk_route.cc
│   │   ├── chunk_route.h
│   │   ├── file_info.cc
│   │   ├── file_info.h
│   │   ├── master_client.cc
│   │   ├── master_client.h
│   │   ├── master_server.cc
│   │   └── master_server.h
│   └── util
│       ├── conf.h
│       ├── lock.cc
│       ├── lock.h
│       ├── lru.hpp
│       ├── state_code.cc
│       ├── state_code.h
│       ├── time.cc
│       └── time.h
└── test
    ├── chunk_client_test
    │   ├── CMakeLists.txt
    │   └── chunk_client_test.cc
    ├── chunk_server_test
    │   ├── CMakeLists.txt
    │   └── chunk_server_test.cc
    ├── gfs_test
    │   ├── CMakeLists.txt
    │   └── gfs_test.cc
    ├── init
    │   ├── CMakeLists.txt
    │   └── init.cc
    ├── master_client_test
    │   ├── CMakeLists.txt
    │   └── master_client_test.cc
    ├── master_server_test
    │   ├── CMakeLists.txt
    │   └── master_server_test.cc
    └── page_test
        ├── CMakeLists.txt
        └── page_test.cc
```

## 0x00 Master Server

### 内部

class MasterServerImpl是Master Server的实现，内部有一个比较重要的map，其内容为：

```cpp
class MasterServerImpl {
    // filename -> FileInfoPtr
    std::map<std::string, FileInfoPtr> files_;
};
```

而这个FileInfoPtr则是一个文件的抽象，其内容也是由一个重要的map构成：

```cpp
class FileInfo {
    // chunk_index -> chunk info
    std::map<uint32_t, ChunkInfoPtr> chunks_;
};
struct ChunkInfo {
    uint64_t chunk_handle; // 保存在chunk server上的UUID
    int primary; // 主副本，无则为-1
    std::vector<int> secondaries; // 其他副本
    uint64_t expired; // 如果有主副本，那这个就是lease过期的绝对时间
};
```

### 开放的RPC

MasterServerImpl开放了很多rpc接口，但基本都是关于文件meta信息获取的，特别是关于chunk server的路由信息等。

下面一个个介绍：

### 1. ListFiles

```protobuf
service MasterServer {
    rpc ListFiles (ListFilesArgs) returns (ListFilesReply);
};

message ListFilesArgs {
    required string prefix = 1;
};

message ListFilesReply {
    required int32 state = 1;
    repeated string filenames = 2;
};
```

ListFiles接口非常简单，直接响应当前MasterServer中全部的文件名信息。

TODO: impl目录

### 2. FileRouteInfo

```protobuf
service MasterServer {
    rpc FileRouteInfo (FileRouteInfoArgs) returns (FileRouteInfoReply);
};

message FileRouteInfoArgs {
  required string filename = 1;
  required uint32 chunk_index = 2;
};

message FileRouteInfoReply {
  required int32 state = 1;
  optional uint64 chunk_handle = 2;
  repeated string route = 3; // 路由信息
};
```

FileRouteInfo给read提供了文件的meta信息，通过filename和chunk_index可以获取对应的路由信息，每个filename下的chunk_index都有一个独一无二的UUID标示号， __在gfs中称为chunk_handle，chunk_server对这些文件名并不感兴趣，只对chunk_handle感兴趣而已(或者说，chunk_server对这些文件名透明)__ 。

### 3. FindLeaseHolder

```protobuf
service MasterServer {
    rpc FindLeaseHolder (FindLeaseHolderArgs) returns 
    (FindLeaseHolderReply);
};
message FindLeaseHolderArgs {
  required string filename = 1;
  optional uint32 chunk_index = 2;
  required bool last = 3; // 直接找到最后一块chunk
};

message FindLeaseHolderReply {
  required int32 state = 1;
  optional uint64 chunk_handle = 2;
  optional uint32 version = 3;
  optional string primary = 4;
  repeated string secondaries = 5;
  optional uint32 chunk_index = 6;
};
```

FindLeaseHolder和之前的FileRouteInfo有一个不一样的点，它给write/append提供了meta信息，需要做的更多，在Master内部，需要找到或者确定一个primary副本，也就是分发lease，然后将对应的路由信息响应至客户端。

在rpc的请求args中，有一个last标记，用来表明是否无视chunk_index，从而直接寻找最后的一个chunk。

之所以这样做是因为在这类文件服务器中，客户端更多的会去做“Append”操作，也就是追加操作，随机写入的操作是很少发生的。

### 4. CreateFile

```protobuf
service MasterServer {
    rpc CreateFile (CreateFileArgs) returns (CreateFileReply);
};
message CreateFileArgs {
  optional string prefix = 1; // TODO: impl
  required string filename = 2;
};

message CreateFileReply {
  required int32 state = 1;
  optional uint64 chunk_handle = 2;
  repeated string routes = 3;
  optional string primary_route = 4;
};
```

CreateFile用于创建一个新的文件，在Master内部会进行文件的初始化，包括创建FileInfo来管理一个新的文件，然后生成第一个chunk，也就是chunk_index = 0的chunk块，然后通知各个chunk server来去初始化这个块，随后响应给客户端，同时，Master会分发lease。

由于客户端可以知道哪个chunk server拥有lease，所以在调用CreateFile的rpc后，客户端也可以直接进行append操作。

### 5. TODO

## 0x01 Chunk Server

### 内部

class ChunkServerImpl是Chunk Server的实现，其主要依赖于一个磁盘管理的抽象类，DiskManager，内部有几个比较重要的map：

```cpp
class DiskManager {
    // chunk_handle -> chunk信息
    std::map<uint64_t, ChunkHandlePtr> chunks_;
    // chunk_handle -> chunk内容
    LRU<uint64_t, PagePtr> cache_;
    // 客户端id+时间 -> 临时数据
    std::map<store_mark_t, PagePtr> tmp_cache_;
};
```

class Page是一个内存中的数据，通过RAII的方式进行内存释放，它映射内存到真正的磁盘数据中，一个Page刚刚好是一个CHUNK_SIZE(本实现中为64KB)的大小。

chunks_是关于chunk_handle至chunk信息的keyvalue数据结构，而cache_在里面是一个look-aside形式的缓存，采用LRU的方式。

tmp_cache_则是临时数据，用于写入操作临时缓存用。

### 开放的RPC

同样的，ChunkServer开放了很多RPC，有给Master调的，也有给客户端的。

### 1. ReadChunk

```protobuf
service ChunkServer {
    rpc ReadChunk (ReadChunkArgs) returns (ReadChunkReply);
};

message ReadChunkArgs {
  // UUID
  required uint64 chunk_handle = 1;
  optional uint32 chunk_version = 2;
  // chunk读取起始offset
  required int64 offset_start = 3;
  // 需要读取的总长度
  required int64 length = 4;
};

message ReadChunkReply {
  required int32 state = 1;
  optional bytes data = 2;
  // 实际上读取的字节数
  optional int64 bytes_read = 3;
};
```

ReadChunk是开放给客户端读取的，通过chunk_handle和offset以及length可以让chunk server知道客户端想要读取哪个chunk的信息，从哪里开始读取，读多长。

### 2. PutData

```protobuf
service ChunkServer {
    rpc PutData (PutDataArgs) returns (PutDataReply);
};
message PutDataArgs {
  optional int64 client_id = 1; // TODO:impl
  required uint64 timestamp = 2; // timestamp millsecond
  required bytes data = 3;
};

message PutDataReply {
  required int32 state = 1;
  optional int64 uuid = 2; // 保留
};
```

PutData同样是给客户端调用的，通过这个rpc，客户端可以提前的将数据放到chunk server上，为之后的write/append做准备，它以客户端id和时间戳作为缓存名字。

### 3. AppendChunk

```protobuf
service ChunkServer {
    rpc AppendChunk (AppendChunkArgs) returns (AppendChunkReply);
};
message AppendChunkArgs {
  optional int64 client_id = 1;
  required uint64 timestamp = 2;
  optional int64 tmp_data_offset = 3; // 临时缓存中起步位置
  repeated string secondaries = 4;
  required uint64 chunk_handle = 5;
};

message AppendChunkReply {
  required int32 state = 1;
  required int64 bytes_written = 2;
};
```

AppendChunk通过客户端id和时间戳来找到临时缓存，在rpc请求中，携带了副本的路由信息，只有primary副本的chunk server会接受这个rpc请求，然后它通过其中携带的路由信息去找到其他chunk server，进而进行提交操作。

ps: 这里并没有实现像gfs论文中的那种链式复制方式，而是由客户端直接向多个chunk server发起PutData rpc，所以本质上流量压力还是在客户端身上。


</font>