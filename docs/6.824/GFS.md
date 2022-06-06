<font face="Monaco">

# GFS 

## 0x00 why hard?

在分布式存储中，为了提高性能，我们可以通过shard来打散各个文件的小块，但如果仅仅这样做，那么容错的能力将大大降低，这也就需要一种容错的机制，比如复制，但复制又引出了更麻烦的东西，因为一不小心，就会出现不一致的情况，而如果想要更好的一致性，那么必然会导致性能下降，那么就陷入了死循环了，因为性能就是我们一开始想要的东西。具体见CAP。


## 0x01 read

### 从client视角查看read操作

客户端先向Master发起rpc请求，为FileRouteInfo请求，其rpc定义：

```protobuf
service MasterServer {
    rpc FileRouteInfo (FileRouteInfoArgs) returns (FileRouteInfoReply);
};

message FileRouteInfoArgs {
    required string filename = 1; // 请求的文件名，如"/tmp/target.txt"
    required uint64 chunk_index = 2; // 客户端需通过offset自行计算
};

message FileRouteInfoReply {
    required int32 state = 1;
    optional uint64 chunk_handle = 2; // chunk UUID
    repeated string route = 3; // 路由信息
};
```

Master返回对应的响应，比如chunk的UUID，各个副本所在的信息，然后客户端再根据路由信息向这些持有对应的chunk server发起请求，具体rpc为：

```protobuf
service ChunkServer {
    rpc ReadChunk (ReadChunkArgs) returns (ReadChunkReply);
};

message ReadChunkArgs {
    required uint64 chunk_handle = 1; // chunk UUID
    optional uint32 chunk_version = 2; // version
    required int64 offset = 3;
    required int64 length = 4;
};

message ReadChunkReply {
    required int32 state = 1;
    optional bytes data = 2;
    optional int64 bytes_read = 3; // 实际读取的字节数
};
```

客户端随机的向所有chunk server发起请求，只要有一个chunk server成功响应了，那么即可返回成功。

### 从Master视角查看read操作

Master得到一个read请求后，通过比对自己身上的路由表来进行响应，无需确定primary副本。

## 0x02 write

### 从客户端角度来查看write操作

首先客户端需要向Master请求路由信息，几乎和FileRouteInfo的rpc一致，但还是有些许不同点这点将体现在Master服务内部，不过还是先来看看这个write请求路由信息的操作：

```protobuf
service MasterServer {
    rpc FindLeaseHolder (FindLeaseHolderArgs) 
        returns (FindLeaseHolderReply);
};

// 几乎和FileRotueInfo一致
message FindLeaseHolderArgs {
    required string filename = 1;
    required uint64 chunk_index = 2;
};

message FindLeaseHolderReply {
    required int32 state = 1;
    required uint64 chunk_handle = 2;
    optional string primary_route = 3;
    repeated string secondaries_route = 4;
};
```

相比之下，多出了primary副本和secondary副本，得到这些东西后，客户端就需要向chunk server发起真正的写入操作了。

chunk server在这方面提供了几个rpc：

```protobuf
service ChunkServer {
    rpc PutData (PutDataArgs) returns (PutDataReply);
    
    rpc WriteChunk (WriteChunkArgs) returns (WriteChunkReply);
};

message PutDataArgs {
    required uint64 client_id = 1;
    required uint64 second = 2;
    required uint64 nano_sec = 3;
    required bytes data = 4;
};

message PutDataReply {
    required int32 state = 1;
};

message WriteChunkArgs {
    required uint64 client_id = 1;
    required uint64 second = 2;
    required uint64 nano_sec = 3;
    required uint64 chunk_handle = 4; // 写入的chunk UUID
    required uint64 offset = 5; // 在chunk中的offset位置进行写入
    repeated string secondaries_route = 6; // 其他副本
    optional uint64 tmp_data_offset = 7; // 保留
};

message WriteChunkReply {
    required int32 state = 1;
    optional uint64 bytes_written = 2;
};
```

首先是chunk需要先通过PutData将数据推到所有的副本服务器上，然后访问具有Lease(primary)的chunk server，PutData结束后，所有服务器都具有了“提交”的能力，随后客户端对primary的服务器发起WriteChunk请求进行提交，提交操作需要直到所有副本成功才会对副本响应成功，否则失败(这里具有Lease的chunk server就有点2PC中coordinate那种感觉)。

### 从Master角度来看write操作

Master做的比较多的是在内部，比如FindLeaseHolder这个rpc操作，如果已经有确定的Lease，那肯定最好，直接响应即可，如果没有确定Lease在哪个chunk server上，那么Master就需要去生成一个，或者说，客户端想要的这个chunk_index压根就不存在，那Master就需要去生成对应的新chunk了。

> 1. Master上已经确定好primary chunk server了

直接响应客户端即可。

> 2. Master上有对应的chunk server列表，但不知道谁才是最新的版本

Master挨个访问chunk server，找到最新的版本作为新的primary，分发Lease，然后将所有的副本版本号加一，响应客户端。

## 0x03 lease





</font>