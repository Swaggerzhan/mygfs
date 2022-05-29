//
// Created by swagger on 2022/5/29.
//
#include "lock.h"

Lock::Lock() {

}

Lock::~Lock() {

}

void Lock::lock() {}
void Lock::unlock() {}
void Lock::write_unlock() {}
void Lock::write_lock() {}
void Lock::read_lock() {}
void Lock::read_unlock() {}


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

