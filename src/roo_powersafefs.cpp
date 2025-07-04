#include "roo_powersafefs.h"

namespace roo_powersafefs {

Mount::Mount(Guard* guard, bool forced)
    : guard_(guard), forced_(forced), mounted_(guard_->tryMount(forced)) {}

Mount::~Mount() {
  if (mounted_) {
    guard_->unmount(forced_);
  }
}

Mount::Mount(Mount&& other)
    : guard_(other.guard_), forced_(other.forced_), mounted_(other.mounted_) {
  other.mounted_ = false;
}

WriteTransaction::WriteTransaction(Guard* guard, bool forced)
    : guard_(guard),
      forced_(forced),
      active_(guard_->tryBeginWriteTransaction(forced)) {}

WriteTransaction::WriteTransaction(WriteTransaction&& other)
    : guard_(other.guard_), forced_(other.forced_), active_(other.active_) {
  other.active_ = false;
}

WriteTransaction::~WriteTransaction() {
  if (active_) {
    guard_->endWriteTransaction(forced_);
  }
}

Guard::Guard(Device* device)
    : device_(device),
      mode_(FS_NORMAL),
      mounted_(false),
      mount_count_(0),
      forced_mount_count_(0),
      write_transaction_count_(0),
      forced_write_transaction_count_(0) {}

Guard::Mode Guard::mode() const {
  roo::lock_guard<roo::mutex> guard(mutex_);
  return mode_;
}

void Guard::unmountIfPending() {
  if (mounted_ && mount_count_ == 0) {
    device_->unmount();
    mounted_ = false;
  }
}

void Guard::setMode(Guard::Mode mode) {
  roo::lock_guard<roo::mutex> guard(mutex_);
  if (mode == mode_) return;
  switch (mode_) {
    case FS_EAGER_UNMOUNT: {
      unmountIfPending();
      // No break.
    }
    case FS_NORMAL: {
      if (!mounted_ && mount_count_ > 0) {
        // Need to re-mount.
        mounted_ = device_->mount();
      }
      break;
    }
    case FS_LAME_DUCK: {
      if (!mounted_ && forced_mount_count_ > 0) {
        // Need to re-mount.
        mounted_ = device_->mount();
      }
      unmountIfPending();
      break;
    }
    case FS_SHUTDOWN: {
      unmountIfPending();
      break;
    }
    case FS_DISABLED: {
      if (mounted_) {
        // Force unmount.
        device_->unmount();
        mounted_ = false;
      }
      break;
    }
  }
  mode_ = mode;
}

bool Guard::isMounted() const {
  roo::lock_guard<roo::mutex> guard(mutex_);
  return mounted_;
}

Mount Guard::mount(bool forced) {
  return Mount(this, forced);
}

WriteTransaction Guard::write(bool forced) {
  return WriteTransaction(this, forced);
}

int Guard::getPendingMountsCount() const {
  roo::lock_guard<roo::mutex> guard(mutex_);
  return mount_count_;
}

int Guard::getPendingWriteTransactionsCount() const {
  roo::lock_guard<roo::mutex> guard(mutex_);
  return write_transaction_count_;
}

bool Guard::tryMount(bool forced) {
  roo::lock_guard<roo::mutex> guard(mutex_);
  switch (mode_) {
    case FS_DISABLED:
    case FS_SHUTDOWN: {
      return false;
    }
    case FS_LAME_DUCK: {
      if (!forced) return false;
    }
    case FS_NORMAL:
    case FS_EAGER_UNMOUNT: {
    }
  }
  if (!mounted_) {
    mounted_ = device_->mount();
  }
  if (mounted_) {
    ++mount_count_;
    if (forced) ++forced_mount_count_;
  }
  return mounted_;
}

void Guard::unmount(bool forced) {
  roo::lock_guard<roo::mutex> guard(mutex_);
  --mount_count_;
  if (forced) --forced_mount_count_;
  if (mounted_ && mount_count_ == 0 && mode_ != FS_NORMAL) {
    device_->unmount();
    mounted_ = false;
  }
}

bool Guard::tryBeginWriteTransaction(bool forced) {
  roo::lock_guard<roo::mutex> guard(mutex_);
  if (!mounted_) return false;
  switch (mode_) {
    case FS_DISABLED:
    case FS_SHUTDOWN: {
      return false;
    }
    case FS_LAME_DUCK: {
      if (!forced) return false;
    }
    case FS_NORMAL:
    case FS_EAGER_UNMOUNT: {
    }
  }
  ++write_transaction_count_;
  if (forced) ++forced_write_transaction_count_;
  return true;
}

void Guard::endWriteTransaction(bool forced) {
  roo::lock_guard<roo::mutex> guard(mutex_);
  --write_transaction_count_;
  if (forced) --forced_write_transaction_count_;
}

}  // namespace roo_powersafefs