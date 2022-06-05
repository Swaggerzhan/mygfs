//
// Created by swagger on 2022/6/4.
//
#ifndef MYGFS_PAGE_H
#define MYGFS_PAGE_H
#include <memory>
#include <string>

namespace gfs {

/*
 * 用作缓存磁盘数据至内存中
 * 当Page析构函数被调用时，如果有对Page进行写入
 * 那么析构函数会尝试更新这些内容
 * not thread safe
 */
class Page;
typedef std::shared_ptr<Page> PagePtr;

class ChunkHandle;
typedef std::weak_ptr<ChunkHandle> ChunkHandleWeakPtr;


class Page {
public:

  Page();
  ~Page();

  static void padding_garbage(int fd);

  bool init(const std::string& path);

  /*
   * 将Page中的信息以可写内存暴露
   */
  char* write_expose();

  /*
   * 将Page中的信息以只读方式暴露
   */
  const char* read_expose();

  /*
   * 将Page中内存信息刷入磁盘
   */
  void flush();

public:

  ChunkHandleWeakPtr chunk_info;

  // used for tmp cache
  int64_t length;

private:
  bool valid_;
  bool has_modify_;

  static int kProt_; // read | write
  static int kFlags_; // private

  std::string path_;

  void* mem_;

};


}; // namespace gfs
#endif //MYGFS_PAGE_H