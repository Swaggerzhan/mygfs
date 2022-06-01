//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_FILE_CONTEXT_H
#define MYGFS_FILE_CONTEXT_H

#include <string>
#include <map>
#include <vector>

namespace gfs {

class MasterClient;

class FileContext {
public:

  FileContext(const std::string& filename, MasterClient* master);
  ~FileContext();

  std::string name();

  /*
   * 获取对应chunk的路由信息
   * @param begin_chunk_index : 开始的chunk
   * @param end_chunk_index: 结束chunk
   */
  bool chunk_info_fetch(uint32_t begin_chunk_index, uint32_t end_chunk_index,
                        std::map<uint64_t, std::vector<std::string>>& ret_routes);

  /*
   * 通过chunk_handle，也就是UUID来获取对应的路由信息
   */
  bool routes(uint64_t chunk_handle, std::vector<std::string> routes);



private:
  friend class Client;

  // not belong this class
  MasterClient* client_;

  std::string filename_;

  int64_t offset_;

  // chunk_index -> chunk_handle: UUID
  std::map<uint64_t, uint64_t> chunks_;

  // chunk_handle:UUID -> chunk_server route
  std::map<uint64_t, std::vector<std::string>> chunk_routes_;


};





};

#endif //MYGFS_FILE_CONTEXT_H
