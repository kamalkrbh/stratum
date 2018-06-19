/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef STRATUM_LIB_CHANNEL_CHANNEL_H_
#define STRATUM_LIB_CHANNEL_CHANNEL_H_

#include <deque>
#include <memory>
#include <vector>

#include "stratum/lib/macros.h"
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"

namespace stratum {

// Channels provide intra-process typed message-passing, designed to
// enable thread-safe data transfer within the Hercules switch-stack.
//
// A Channel<T> synchronizes communication between multiple ChannelWriter<T>s
// and ChannelReader<T>s. Messages are handled in FIFO order.
//
// Channel Objects:
//
//   Channel<T>: This is the main Channel object which encapsulates an internal
//   queue and the necessary synchronization primitives. A Channel<T> is created
//   via Channel<T>::Create() which returns a unique pointer to a new Channel<T>
//   object. Communication through a Channel<T> instance is done via
//   ChannelReader<T> and ChannelWriter<T> instances.
//
//   ChannelWriter<T>, ChannelReader<T>: The ChannelWriter and ChannelReader
//   objects are used to access the Write and Read functionalities respectively
//   of a Channel. ChannelWriter and ChannelReader instances share ownership of
//   a Channel instance via shared pointers.
//
//   T: Message type. T must be move-assignable.
//
// Example Setup and Cleanup:
//
//   int max_depth = 128;
//   // Create new Channel with internal buffer size 128 * sizeof(T) bytes.
//   std::shared_ptr<Channel<T>> channel = Channel<T>::Create(max_depth);
//
//   // Create the ChannelReader and ChannelWriter.
//   auto reader = ChannelReader<T>::Create(channel);
//   auto writer = ChannelWriter<T>::Create(channel);
//
//   // Pass the ChannelReader and ChannelWriter to other threads.
//   reader_args_t reader_args(std::move(reader));
//   pthread_create(..., &reader_args);
//   writer_args_t writer_args(std::move(writer));
//   pthread_create(..., &writer_args);
//
//   // Relinquish control of the Channel object once all required
//   ChannelReaders and
//   // ChannelWriters have been created. The object will only be destroyed once
//   all
//   // related ChannelReaders and ChannelWriters have been destroyed.
//   channel.reset();
//
//   // ALTERNATIVE: There may be conditions where it is known that the Channel
//   // will no longer be required, such as shutdown scenarios. In such cases,
//   // the original reference can be retained, and the following may be done:
//   channel->close();
//   // This notifies all blocked ChannelReaders or ChannelWriters that the
//   Channel is closed.
//   // Subsequent Read() or Write() calls immediately return.
//
// Example ChannelReader Function:
//
//   bool exit = false;  // can be set by another thread.
//   void* ThreadFunc(void* args) {
//     auto reader = std::move(reinterpret_cast<reader_args_t*>(args)->reader);
//     absl::Duration timeout = absl::Seconds(5);
//     T buf;
//     ::util::Status retval;
//     // Keep reading as long as the Channel is still open.
//     do {
//       retval = reader.Read(&buf, timeout);
//       if (retval.CanonicalCode() == ::util::error::CANCELLED) break;
//       // At most every 5 seconds, check whether or not to exit.
//       if (exit) break;
//       // If the queue was empty, block on Read again.
//       if (retval.CanonicalCode() == ::util::error::NOT_FOUND) continue;
//       // Operate on data.
//       ...
//     } while (1);
//     // If the ChannelReader holds the last reference to the Channel, the
//     // Channel will also be destroyed.
//     return nullptr;
//   }
//
// Notes on Usage:
//
// 1. ChannelReader<T>/ChannelWriter<T> instances are the only way to access the
//    core functionalities of a Channel<T> instance.
//
// 2. The Channel remains open so long as Close() has not been called. As long
//    as a valid shared_ptr managing the original Channel instance remains in
//    scope, more ChannelReaders or ChannelWriters may be added to the Channel.
//
// 3. It is recommended to only read from a given Channel from a single thread.
//    Reading necessarily consumes data which will not be available to other
//    threads. Additionally, Reading from multiple threads can easily cause
//    out-of-sender-order processing of messages.

template <typename T>
class ChannelReader;
template <typename T>
class ChannelWriter;

// TODO: add support for optional en/dequeue timestamping.
template <typename T>
class Channel {
  // Check the type requirements documented at the top of this file.
  static_assert(std::is_move_assignable<T>::value,
                "Channel<T> requires T to be MoveAssignable.");

 public:
  virtual ~Channel() {}

  // Creates shared Channel object with given maximum queue depth.
  static std::unique_ptr<Channel<T>> Create(size_t max_depth) {
    return std::unique_ptr<Channel<T>>(new Channel<T>(max_depth));
  }

  // Closes the Channel. Any blocked Read() or Write() operations immediately
  // return ERR_CANCELLED. Returns false if the Channel is already closed.
  virtual bool Close() LOCKS_EXCLUDED(queue_lock_);

  // Returns true if the Channel has been closed.
  virtual bool IsClosed() LOCKS_EXCLUDED(queue_lock_);

  // Disallow copy and assign.
  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;

 protected:
  // Protected constructor which intializes the Channel to the given maximum
  // queue depth.
  explicit Channel(size_t max_depth) : closed_(false), max_depth_(max_depth) {}

  // Writes a copy of t into the Channel. Returns ERR_SUCCESS on successful
  // enqueue. Blocks if the queue is full until the timeout, then returns
  // ERR_NO_RESOURCE. Returns ERR_CANCELLED if the Channel is closed.
  //
  // Note: If timeout = absl::InfiniteDuration(), Write() blocks indefinitely.
  virtual ::util::Status Write(const T& t, absl::Duration timeout)
      LOCKS_EXCLUDED(queue_lock_);
  virtual ::util::Status Write(T&& t, absl::Duration timeout)
      LOCKS_EXCLUDED(queue_lock_);

  // Returns ERR_NO_RESOURCE immediately if the queue is full.
  virtual ::util::Status TryWrite(const T& t) LOCKS_EXCLUDED(queue_lock_);
  virtual ::util::Status TryWrite(T&& t) LOCKS_EXCLUDED(queue_lock_);

  // Reads and pops the first element of the queue into t. Returns ERR_SUCCESS
  // on successful dequeue. Blocks if the queue is empty until the timeout, then
  // returns ERR_ENTRY_NOT_FOUND. Returns ERR_CANCELED if Channel is closed and
  // the queue is empty.
  //
  // Note: If timeout = absl::InfiniteDuration(), Read() blocks indefinitely.
  virtual ::util::Status Read(T* t, absl::Duration timeout)
      LOCKS_EXCLUDED(queue_lock_);

  // Returns ERR_ENTRY_NOT_FOUND immediately if the queue is empty.
  virtual ::util::Status TryRead(T* t) LOCKS_EXCLUDED(queue_lock_);

  // Reads all of the elements of the queue into ts. Returns ERR_CANCELED if the
  // Channel is closed, otherwise ERR_SUCCESS.
  virtual ::util::Status ReadAll(std::vector<T>* t_s)
      LOCKS_EXCLUDED(queue_lock_);

 private:
  // Helper function used by both variants of Write(). Checks if Channel state
  // is closed and blocks if the internal queue is full. Returns OK or the error
  // statuses described above.
  ::util::Status CheckWriteStateAndBlock(absl::Duration timeout)
      EXCLUSIVE_LOCKS_REQUIRED(queue_lock_);

  // Helper function used by both variants of TryWrite(). Checks Channel state
  // for closure and queue occupancy. Returns OK or the error statuses described
  // above.
  ::util::Status CheckWriteState() EXCLUSIVE_LOCKS_REQUIRED(queue_lock_);

  // Mutex to protect internal queue of the Channel and state.
  mutable absl::Mutex queue_lock_;
  std::deque<T> queue_ GUARDED_BY(queue_lock_);
  bool closed_ GUARDED_BY(queue_lock_);

  // Maximum queue depth.
  const size_t max_depth_;

  // Condition variable for ChannelReaders waiting on empty queue_.
  mutable absl::CondVar cond_not_empty_;

  // Condition variable for ChannelWriters waiting on full queue_.
  mutable absl::CondVar cond_not_full_;

  friend class ChannelReader<T>;
  friend class ChannelWriter<T>;
};

template <typename T>
class ChannelReader {
 public:
  virtual ~ChannelReader() {}

  // Creates and returns ChannelReader for the Channel. Returns nullptr if
  // Channel is nullptr or closed.
  static std::unique_ptr<ChannelReader<T>> Create(
      std::shared_ptr<Channel<T>> channel) {
    if (!channel || channel->IsClosed()) return nullptr;
    return std::unique_ptr<ChannelReader<T>>(
        new ChannelReader(std::move(channel)));
  }

  // The following functions are wrappers around the corresponding Channel
  // functionality.
  virtual ::util::Status Read(T* t, absl::Duration timeout) {
    return channel_->Read(t, timeout);
  }
  virtual ::util::Status TryRead(T* t) { return channel_->TryRead(t); }
  virtual ::util::Status ReadAll(std::vector<T>* t_s) {
    return channel_->ReadAll(t_s);
  }
  virtual bool IsClosed() { return channel_->IsClosed(); }

  // Disallow copy and assign.
  ChannelReader(const ChannelReader&) = delete;
  ChannelReader& operator=(const ChannelReader&) = delete;

 protected:
  // Constructor for mock ChannelReaders.
  ChannelReader() {}

 private:
  // Private constructor which initializes a ChannelReader from the given
  // Channel.
  explicit ChannelReader(std::shared_ptr<Channel<T>> channel)
      //FIXME ABSL_DIE_IF_NULL not available in absl
      : channel_(/*FIXME ABSL_DIE_IF_NULL(*/std::move(channel)/*)*/) {}

 std::shared_ptr<Channel<T>> channel_;
};

template <typename T>
class ChannelWriter {
 public:
  virtual ~ChannelWriter() {}

  // Creates and returns ChannelWriter for the Channel. Returns nullptr if
  // Channel is nullptr or closed.
  static std::unique_ptr<ChannelWriter<T>> Create(
      std::shared_ptr<Channel<T>> channel) {
    if (!channel || channel->IsClosed()) return nullptr;
    return std::unique_ptr<ChannelWriter<T>>(
        new ChannelWriter(std::move(channel)));
  }

  // The following functions are wrappers around the corresponding Channel
  // functionality.
  virtual ::util::Status Write(const T& t, absl::Duration timeout) {
    return channel_->Write(t, timeout);
  }
  virtual ::util::Status Write(T&& t, absl::Duration timeout) {
    return channel_->Write(std::move(t), timeout);
  }
  virtual ::util::Status TryWrite(const T& t) { return channel_->TryWrite(t); }
  virtual ::util::Status TryWrite(T&& t) {
    return channel_->TryWrite(std::move(t));
  }
  virtual bool IsClosed() { return channel_->IsClosed(); }

  // Disallow copy and assign.
  ChannelWriter(const ChannelWriter&) = delete;
  ChannelWriter& operator=(const ChannelWriter&) = delete;

 protected:
  // Constructor for mock ChannelWriters.
  ChannelWriter() {}

 private:
  // Private constructor which initializes a ChannelWriter to the given Channel.
  explicit ChannelWriter(std::shared_ptr<Channel<T>> channel)
      //FIXME ABSL_DIE_IF_NULL not available in absl
      : channel_(/*ABSL_DIE_IF_NULL(*/std::move(channel)/*)*/) {}

  std::shared_ptr<Channel<T>> channel_;
};

template <typename T>
bool Channel<T>::Close() {
  absl::MutexLock l(&queue_lock_);
  if (closed_) return false;
  closed_ = true;
  // Signal all blocked ChannelWriters.
  cond_not_full_.SignalAll();
  // Signal all blocked ChannelReaders.
  cond_not_empty_.SignalAll();
  return true;
}

template <typename T>
bool Channel<T>::IsClosed() {
  absl::MutexLock l(&queue_lock_);
  return closed_;
}

template <typename T>
::util::Status Channel<T>::Write(const T& t, absl::Duration timeout) {
  absl::MutexLock l(&queue_lock_);
  // Check internal state, blocking with timeout if queue is full.
  RETURN_IF_ERROR(CheckWriteStateAndBlock(timeout));
  // Enqueue message.
  queue_.push_back(t);
  // Signal next blocked ChannelReader.
  cond_not_empty_.Signal();
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::Write(T&& t, absl::Duration timeout) {
  absl::MutexLock l(&queue_lock_);
  // Check internal state, blocking with timeout if queue is full.
  RETURN_IF_ERROR(CheckWriteStateAndBlock(timeout));
  // Enqueue message.
  queue_.push_back(std::move(t));
  // Signal next blocked ChannelReader.
  cond_not_empty_.Signal();
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::CheckWriteStateAndBlock(absl::Duration timeout) {
  // Check Channel closure. If closed, there will be no signal.
  if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
  // Wait with timeout for non-full internal buffer. While is required as
  // signals may be delivered without an actual call to Signal() or
  // SignallAll().
  while (queue_.size() == max_depth_) {
    bool expired = cond_not_full_.WaitWithTimeout(&queue_lock_, timeout);
    // Could have been signalled because Channel is now closed.
    if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
    // Could have been signalled even if timeout has expired.
    if (expired && (queue_.size() == max_depth_)) {
      return MAKE_ERROR(ERR_NO_RESOURCE)
             << "Write did not succeed within timeout due to full Channel.";
    }
  }
  // Queue size should never exceed maximum queue depth.
  if (queue_.size() > max_depth_) {
    // TODO: Change to CRITICAL error once that is an option.
    return MAKE_ERROR(ERR_INTERNAL)
           << "Channel load " << queue_.size() << " exceeds max queue depth "
           << max_depth_ << ".";
  }
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::TryWrite(const T& t) {
  absl::MutexLock l(&queue_lock_);
  // Check internal state.
  RETURN_IF_ERROR(CheckWriteState());
  // Enqueue message.
  queue_.push_back(t);
  // Signal next blocked ChannelReader.
  cond_not_empty_.Signal();
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::TryWrite(T&& t) {
  absl::MutexLock l(&queue_lock_);
  // Check internal state.
  RETURN_IF_ERROR(CheckWriteState());
  // Enqueue message.
  queue_.push_back(std::move(t));
  // Signal next blocked ChannelReader.
  cond_not_empty_.Signal();
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::CheckWriteState() {
  // Check for Channel closure.
  if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
  // Check for full internal buffer.
  if (queue_.size() == max_depth_) {
    return MAKE_ERROR(ERR_NO_RESOURCE) << "Channel is full.";
  }
  // Queue size should never exceed maximum queue depth.
  if (queue_.size() > max_depth_) {
    // TODO: Change to CRITICAL error once that is an option.
    return MAKE_ERROR(ERR_INTERNAL)
           << "Channel load " << queue_.size() << " exceeds max queue depth "
           << max_depth_ << ".";
  }
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::Read(T* t, absl::Duration timeout) {
  absl::MutexLock l(&queue_lock_);
  // Check Channel closure. If closed, will not be signaled during wait.
  if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
  // Wait with timeout for non-empty internal buffer.
  while (queue_.empty()) {
    bool expired = cond_not_empty_.WaitWithTimeout(&queue_lock_, timeout);
    // Could have been signalled because Channel is now closed.
    if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
    // Could have been signalled even if timeout has expired.
    if (expired && queue_.empty()) {
      return MAKE_ERROR(ERR_ENTRY_NOT_FOUND)
             << "Read did not succeed within timeout due to empty Channel.";
    }
  }
  // Dequeue message.
  *t = std::move(queue_.front());
  queue_.pop_front();
  // Signal next blocked ChannelWriter.
  cond_not_full_.Signal();
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::TryRead(T* t) {
  absl::MutexLock l(&queue_lock_);
  // Check for Channel closure.
  if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
  // Check for empty internal buffer.
  if (queue_.empty()) {
    return MAKE_ERROR(ERR_ENTRY_NOT_FOUND) << "Channel is empty.";
  }
  // Dequeue message.
  *t = std::move(queue_.front());
  queue_.pop_front();
  // Signal next blocked ChannelWriter.
  cond_not_full_.Signal();
  return ::util::OkStatus();
}

template <typename T>
::util::Status Channel<T>::ReadAll(std::vector<T>* t_s) {
  absl::MutexLock l(&queue_lock_);
  // Check for Channel closure.
  if (closed_) return MAKE_ERROR(ERR_CANCELLED) << "Channel is closed.";
  // Resize vector for element_s to be moved.
  t_s->resize(queue_.size());
  std::move(queue_.begin(), queue_.end(), t_s->begin());
  // Clear internal buffer.
  queue_.erase(queue_.begin(), queue_.end());
  // Signal all blocked ChannelWriters.
  cond_not_full_.SignalAll();
  return ::util::OkStatus();
}

}  // namespace stratum

#endif  // STRATUM_LIB_CHANNEL_CHANNEL_H_
