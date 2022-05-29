//
// Created by swagger on 2022/5/29.
//
#include "state_code.h"


static std::string storage_state_code[4] = {
        "state_ok",
        "state_err",
        "state_file_not_found",
        "state_file_chunk_index_err",
};


void debug_print(state_code c) {
  std::cout << storage_state_code[c] << std::endl;
}

std::string debug_string(state_code c) {
  return storage_state_code[c];
}

std::string debug_string(int32_t c) {
  return storage_state_code[c];
}


