//
// Created by swagger on 2022/6/2.
//

#ifndef MYGFS_TIME_H
#define MYGFS_TIME_H

#include <cinttypes>

namespace gfs {

uint64_t get_lease_time();

bool is_expired(uint64_t lease);

uint64_t now();


}; // namespace gfs
#endif //MYGFS_TIME_H
