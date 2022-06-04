//
// Created by swagger on 2022/6/2.
//
#include "time.h"
#include <sys/time.h>

static uint64_t kSecond2MillSecond = 1000;

uint64_t get_lease_time() {
  timeval t{};
  gettimeofday(&t, nullptr);
  return t.tv_sec * kSecond2MillSecond + t.tv_usec;
}

bool is_expired(uint64_t lease) {
  timeval t{};
  gettimeofday(&t, nullptr);
  uint64_t now = t.tv_sec * kSecond2MillSecond + t.tv_usec;
  return now >= lease;
}

