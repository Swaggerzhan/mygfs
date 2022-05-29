//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_CONF_H
#define MYGFS_CONF_H



// chunk server
#define CHUNK_SIZE 1024 * 64 // 64K
#define GFS_CHUNK_SERVER_ROOT_DIR "/gfs"
#define GFS_CHUNK_SERVER_START_PROT 30001



// for debug

#define DEBUG_CHUNK_UUID 10000000
#define DEBUG_CHUNK_VERSION_BEGIN 1

#define GFS_CHUNK_SERVER_1_ROUTE "192.168.3.15:30001"



#endif //MYGFS_CONF_H
