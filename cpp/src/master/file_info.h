//
// Created by swagger on 2022/6/2.
//

#ifndef MYGFS_FILE_INFO_H
#define MYGFS_FILE_INFO_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "src/util/lock.h"
#include "proto/out/gfs.pb.h"


namespace gfs {

struct ChunkInfo;
typedef std::shared_ptr<ChunkInfo> ChunkInfoPtr;

class FileInfo {
public:

  FileInfo(const std::string &filename);

  ~FileInfo() = default;

  /**
   * @brief 创建一个新的chunk，在文件初始化的时候调用
   * @param reply
   * @return: true 为成功
   */
  bool init(CreateFileReply* reply);

  /**
   *  @brief 通过chunk_index来获取当前文件的信息
   *  @param chunk_index: 文件对应的chunk_index
   *  @param reply: RPC响应
   *  @return: true 为成功
   */
  bool read_info(uint32_t chunk_index, FileRouteInfoReply* reply);

  /**
   * @brief 同样通过chunk_index来获取文件信息，但需要找到一个primary
   * 如果没有，那就生成一个
   * @param chunk_index: 文件对应的chunk_index
   * @param reply: RPC响应
   * @return: true 为成功
   */
  bool write_info(uint32_t chunk_index, FindLeaseHolderReply* reply);

private:

  /*
   * 从给定的数组chunk server中选出一个chunk server 作为primary
   * @return: 找出的primary id
   * -1为失败
   */
  int pick_primary(const std::vector<int>& secondaries);


private:

  std::string filename_;

  std::mutex chunks_mutex_;
  // chunk_index -> chunk info
  std::map<uint32_t, ChunkInfoPtr> chunks_;


};

}; // namespace gfs

#endif //MYGFS_FILE_INFO_H
