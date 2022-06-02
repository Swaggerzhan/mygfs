//
// Created by swagger on 2022/6/2.
//

#ifndef MYGFS_CHUNK_ROUTE_H
#define MYGFS_CHUNK_ROUTE_H

namespace gfs {

class ChunkClient;
typedef std::shared_ptr<ChunkClient> ChunkClientPtr;

int add_chunk_server(const std::string& route);

bool fetch_chunk_server(int id, ChunkClientPtr& ptr);

bool fetch_chunk_server_route(int id, std::string& route);




}; // namespace gfs





#endif //MYGFS_CHUNK_ROUTE_H
