//
// Created by swagger on 2022/5/29.
//

#ifndef MYGFS_LOCK_H
#define MYGFS_LOCK_H

#include <pthread.h>

class Lock {
public:
  Lock();
  ~Lock();
  virtual void lock();
  virtual void unlock();
  virtual void read_lock();
  virtual void read_unlock();
  virtual void write_lock();
  virtual void write_unlock();
};


class RWLOCK : public Lock {
public:

  RWLOCK() = default;
  ~RWLOCK() = default;

  void read_lock() override;
  void read_unlock() override;

  void write_lock() override;
  void write_unlock() override;


private:
  pthread_rwlock_t rw_lock_;
};


class LockGuard {
public:
  LockGuard(Lock* l);
  ~LockGuard();

  Lock* lock_;
};

class ReadLockGuard {
public:
  explicit ReadLockGuard(Lock* l);
  ~ReadLockGuard();

  Lock* lock_;
};

class WriteLockGuard {
public:
  explicit WriteLockGuard(Lock* l);
  ~WriteLockGuard();

  Lock* lock_;
};



#endif //MYGFS_LOCK_H
