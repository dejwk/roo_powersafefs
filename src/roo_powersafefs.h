#pragma once

#include <memory>

#include "roo_threads/mutex.h"

namespace roo_powersafefs {

/// Filesystem backend that can be mounted and unmounted on demand.
class Device {
 public:
  virtual ~Device() {}
  /// Mounts the underlying filesystem.
  ///
  /// @return true if the filesystem is mounted and ready for use.
  virtual bool mount() = 0;
  /// Unmounts the underlying filesystem.
  virtual void unmount() = 0;
};

class Guard;

/// RAII handle that requests the filesystem to stay mounted.
///
/// The filesystem remains mounted while this object is alive. Depending on
/// guard mode and `forced`, it may remain mounted after destruction.
///
/// Typical usage:
///
/// @code
/// Mount mount(&guard);
/// if (mount.mounted()) {
///   // Perform reads on the filesystem.
/// }
/// @endcode
class Mount {
 public:
  /// Attempts to mount using guard policy.
  ///
  /// @param guard Guard controlling mount policy.
  /// @param forced If true, bypasses restrictive modes when allowed.
  Mount(Guard* guard, bool forced = false);
  Mount(Mount&& other);

  /// Returns true if the mount request succeeded.
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

/// RAII handle that marks an active filesystem write transaction.
///
/// Use this in combination with `Mount` when writes are needed.
class WriteTransaction {
 public:
  /// Attempts to begin a write transaction.
  ///
  /// @param guard Guard controlling write policy.
  /// @param forced If true, bypasses restrictive modes when allowed.
  WriteTransaction(Guard* guard, bool forced = false);
  WriteTransaction(WriteTransaction&& other);

  ~WriteTransaction();

  /// Returns true if the write transaction is active.
  bool active() const { return active_; }

 private:
  WriteTransaction(const WriteTransaction&) = delete;
  WriteTransaction& operator=(const WriteTransaction&) = delete;

  Guard* guard_;
  bool forced_;
  bool active_;
};

/// Coordinates filesystem mount and write access policy.
class Guard {
 public:
  /// Filesystem operating mode.
  enum Mode {
    /// All mount and write requests are granted.
    ///
    /// Filesystem remains mounted indefinitely once mounted.
    FS_NORMAL,

    /// Like `FS_NORMAL`, but unmounts when the last `Mount` is destroyed.
    FS_EAGER_UNMOUNT,

    /// Rejects non-forced requests and unmounts when no mounts remain.
    FS_LAME_DUCK,

    /// Rejects all new requests and unmounts when no mounts remain.
    FS_SHUTDOWN,

    /// Immediately unmounts, even if currently in use.
    FS_DISABLED
  };

  /// Creates a guard for the specified filesystem device.
  Guard(Device* device);

  /// Returns the current guard mode.
  Mode mode() const;
  /// Changes guard mode.
  void setMode(Mode);

  /// Requests a mounted handle.
  Mount mount(bool force = false);
  /// Requests an active write transaction.
  WriteTransaction write(bool force = false);

  /// Returns true if the underlying device is mounted.
  ///
  /// In `FS_DISABLED` mode this returns true. In `FS_NORMAL`, this can return
  /// true even when `getPendingMountsCount()` is zero.
  bool isMounted() const;

  /// Returns the number of active `Mount` handles.
  int getPendingMountsCount() const;

  /// Returns the number of active write transactions.
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