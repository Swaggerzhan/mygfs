//
// Created by swagger on 2022/5/29.
//
#include "lock.h"


void RWLOCK::read_lock() {
  pthread_rwlock_rdlock(&rw_lock_);
}

void RWLOCK::read_unlock() {
  pthread_rwlock_unlock(&rw_lock_);
}

void RWLOCK::write_lock() {
  pthread_rwlock_wrlock(&rw_lock_);
}

void RWLOCK::write_unlock() {
  pthread_rwlock_unlock(&rw_lock_);
}




LockGuard::LockGuard(Lock* l)
: lock_(l)
{
  l->lock();
}

LockGuard::~LockGuard() {
  lock_->unlock();
}


ReadLockGuard::ReadLockGuard(Lock *l)
: lock_(l)
{
  l->read_lock();
}

ReadLockGuard::~ReadLockGuard() {
  lock_->read_unlock();
}

WriteLockGuard::WriteLockGuard(Lock *l)
: lock_(l)
{
  l->write_lock();
}

WriteLockGuard::~WriteLockGuard() {
  lock_->write_unlock();
}

