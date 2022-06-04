//
// Created by swagger on 2022/6/3.
//
#include "src/util/lru.hpp"
#include <iostream>

using namespace std;

int main() {

  gfs::LRU<int, int> lru;

  for (int i=0; i<1024; ++i) {
    lru.put(i, i);
  }

  int value = -1;
  bool ret = lru.get(0, value);
  if ( !ret ) {
    cout << "error" << endl;
    return 0;
  }
  cout << "got: " << value << endl;
  lru.put(1024, 1024);

  ret = lru.get(1, value);
  if ( ret ) {
    cout << "error" << endl;
    return 0;
  }
  cout << "evict ok" << endl;

  ret = lru.get(2, value);
  cout << "got: " << value << endl;


  return 0;
}

