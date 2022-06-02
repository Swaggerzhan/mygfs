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

typedef std::unique_ptr<ChunkInfo> ChunkInfoPtr;

class FileInfo {
public:

  FileInfo(const std::string &filename);

  /*
   *  通过chunk_index来获取当前文件的信息
   */
  bool read_info(uint32_t chunk_index, FileRouteInfoReply* reply);

  /*
   * 同样通过chunk_index来获取文件信息，但需要找到一个primary
   * 如果没有，那就生成一个
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


  RWLOCK chunks_rw_lock_;
  // chunk_index -> chunk info
  std::map<uint32_t, ChunkInfo> chunks_;


};

}; // namespace gfs

#endif //MYGFS_FILE_INFO_H
