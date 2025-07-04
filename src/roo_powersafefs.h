#pragma once

#include <memory>

#include "roo_threads/mutex.h"

namespace roo_powersafefs {

class Device {
 public:
  virtual ~Device() {}
  virtual bool mount() = 0;
  virtual void unmount() = 0;
};

class Guard;

// Created in order to request that the filesystem is mounted. The filesystem
// will remain mounted for as long as this object is alive. If the guard
// has been configured for delayed unmount, the filesystem may remain =
// mounted after the object is destroyed.
// If you need write access to the filesystem, also use WriteTransaction
// (see below).
// Typical usage scenario:
//
// Mount mount(&guard);
// if (mount.mounted()) {
//   // Perform reads on the filesystem.
// }
class Mount {
 public:
  Mount(Guard* guard, bool forced = false);
  Mount(Mount&& other);

  bool mounted() const { return mounted_; }

  ~Mount();

 private:
  friend class Guard;

  Mount(const Mount&) = delete;
  Mount& operator=(const Mount&) = delete;

  Guard* guard_;
  bool forced_;
  bool mounted_;
};

// Created in order to signal intent to perform a write operation on the
// file system. The typical usage scenario:
//
// Mount mount(&guard);
// if (mount.mounted()) {
//   WriteTransaction write(&guard);
//   if (write.active()) { ... }
// }
class WriteTransaction {
 public:
  WriteTransaction(Guard* guard, bool forced = false);
  WriteTransaction(WriteTransaction&& other);

  ~WriteTransaction();

  bool active() const { return active_; }

 private:
  WriteTransaction(const WriteTransaction&) = delete;
  WriteTransaction& operator=(const WriteTransaction&) = delete;

  Guard* guard_;
  bool forced_;
  bool active_;
};

class Guard {
 public:
  enum Mode {
    // All mount and write transaction operations are granted.
    // The filesystem gets mounted when requested. Once mounted, the
    // filesystem remains mounted indefinitely.
    FS_NORMAL,

    // Like FS_NORMAL, except that the filesystem gets
    // unmounted as soon as all mount objects are destroyed.
    FS_EAGER_UNMOUNT,

    // New mount and write transaction requests are rejected,
    // unless they are called with the 'force' mode. The filesystem gets
    // unmounted as soon as all mount objects are destroyed.
    FS_LAME_DUCK,

    // New mount and write transaction requests are rejected,
    // even if they are called with the 'force' mode. The filesystem gets
    // unmounted as soon as all mount objects are destroyed.
    FS_SHUTDOWN,

    // The filesystem gets immediately unmounted, even if it is in use.
    FS_DISABLED
  };

  Guard(Device* device);

  Mode mode() const;
  void setMode(Mode);

  Mount mount(bool force = false);
  WriteTransaction write(bool force = false);

  // Returns true if the underlying device is mounted; false otherwise.
  // In FS_DISABLED mode, will always return true. In FS_NORMAL, may return
  // true even if getPendingMountsCount() is zero.
  bool isMounted() const;

  // Returns the number of Mount objects for this guard object.
  int getPendingMountsCount() const;

  // Returns the number of write transactions for this guard object.
  int getPendingWriteTransactionsCount() const;

 private:
  friend class Mount;
  friend class WriteTransaction;

  bool tryMount(bool forced);
  void unmount(bool forced);

  bool tryBeginWriteTransaction(bool forced);
  void endWriteTransaction(bool forced);

  void unmountIfPending();
  
  Device* device_;
  mutable roo::mutex mutex_;

  Mode mode_;
  bool mounted_;
  int mount_count_;
  int forced_mount_count_;
  int write_transaction_count_;
  int forced_write_transaction_count_;
};

}  // namespace roo_powersafefs