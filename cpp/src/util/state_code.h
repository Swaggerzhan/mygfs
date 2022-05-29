//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_STATE_CODE_H
#define MYGFS_STATE_CODE_H

#include <iostream>


enum state_code {
  state_ok = 0,
  state_err = 1,
  state_file_not_found = 2,
  state_file_chunk_index_err = 3,
};


std::string debug_string(state_code c);
void debug_print(state_code c);


#endif //MYGFS_STATE_CODE_H
