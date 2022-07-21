//
// Created by swagger on 2022/6/2.
//

#ifndef MYGFS_CHUNK_ROUTE_H
#define MYGFS_CHUNK_ROUTE_H

#include <vector>
namespace gfs {

class ChunkClient;
typedef std::shared_ptr<ChunkClient> ChunkClientPtr;

/**
 * @brief 通过route添加chunk client
 * @param route: 路由
 * @return: chunk server id
 */
int add_chunk_server(const std::string& route);

/**
 * @brief 通过chunk server id来获取相应的chunk client指针
 * @param id : chunk server id
 * @param ptr: 需要得到的指针
 * @return: true为成功
 */
bool fetch_chunk_server(int id, ChunkClientPtr& ptr);

/**
 * @brief 通过route来获取对应的chunk client指针
 * 如果没有，就尝试添加
 * @param route: 路由地址
 * @param ptr: 需要得到的指针
 * @return: true为成功
 */
bool fetch_chunk_server(const std::string& route, ChunkClientPtr& ptr);

/**
 * @brief 通过chunk server id来获取相应的路由信息
 * @param id : chunk server id
 * @param route: 路由返回
 * @return: true为成功
 */
bool fetch_chunk_server_route(int id, std::string& route);

/**
 * @return true 表示路由表为空
 */
bool route_empty();

/**
 * @brief 随机返回一个主副本chunk server和几个备份副本的chunk server
 * @param primary: 主副本chunk client ptr
 * @param secondaries: 备用副本 chunk client ptrs
 * @return true为成功
 */
bool pick_primary_and_secondaries(ChunkClientPtr& primary,
                                  std::vector<ChunkClientPtr>& secondaries);


// ************* DEBUG ********************

/*
 * 直接链接至固定的chunk server
 */
void debug_start_connect();




}; // namespace gfs





#endif //MYGFS_CHUNK_ROUTE_H
