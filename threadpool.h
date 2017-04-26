/*
 * Copyright (c) 2017 Chris Fallin <cfallin@c1f.net>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Lesser Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace threadpool {

struct Item {
  std::function<void()> item;
  bool done;
  bool detached;
  std::condition_variable done_condvar;
  std::mutex done_mutex;
};

class Workqueue {
private:
  std::queue<Item *> items_;
  std::mutex items_mutex_;
  std::condition_variable items_condvar_;
  bool shutdown_;

public:
  Workqueue() : shutdown_(false) {}

  bool Enqueue(std::function<void()> &&item) {
    std::unique_ptr<Item> p(new Item{std::move(item), false, true});

    {
      std::unique_lock<std::mutex> guard(items_mutex_);
      if (shutdown_) {
        return false;
      }
      items_.push(p.release());
      items_condvar_.notify_one();
    }
    return true;
  }

  bool EnqueueAndWait(std::function<void()> &&item) {
    Item it{std::move(item), false, false};
    {
      std::unique_lock<std::mutex> guard(items_mutex_);
      if (shutdown_) {
        return false;
      }
      items_.push(&it);
      items_condvar_.notify_one();
    }
    {
      std::unique_lock<std::mutex> guard(it.done_mutex);
      it.done_condvar.wait(guard, [&] { return it.done; });
      return true;
    }
  }

  Item *Dequeue() {
    std::unique_lock<std::mutex> guard(items_mutex_);
    if (!items_.empty()) {
      Item *it = items_.front();
      items_.pop();
      return it;
    }
    if (shutdown_) {
      return nullptr;
    }
    items_condvar_.wait(guard, [&] { return !items_.empty() || shutdown_; });
    if (items_.empty() && shutdown_) {
      return nullptr;
    }
    Item *it = items_.front();
    items_.pop();
    return it;
  }

  void Complete(Item *it) {
    std::unique_lock<std::mutex> guard(it->done_mutex);
    it->done = true;
    it->done_condvar.notify_all();
    if (it->detached) {
      guard.unlock();
      delete it;
    }
  }

  void Shutdown() {
    std::unique_lock<std::mutex> guard(items_mutex_);
    shutdown_ = true;
    items_condvar_.notify_all();
  }
};

class Threadpool {
private:
  std::vector<std::thread> threads_;
  Workqueue queue_;

public:
  Threadpool(int n) {
    for (int i = 0; i < n; i++) {
      threads_.push_back(std::thread([&]() {
        while (1) {
          Item *it = queue_.Dequeue();
          if (!it) {
            break;
          }
          it->item();
          queue_.Complete(it);
        }
      }));
    }
  }

  ~Threadpool() { Shutdown(); }

  void Shutdown() {
    queue_.Shutdown();
    for (auto &t : threads_) {
      t.join();
    }
    threads_.clear();
  }

  bool Enqueue(std::function<void()> &&item) {
    return queue_.Enqueue(std::move(item));
  }

  bool EnqueueAndWait(std::function<void()> &&item) {
    return queue_.EnqueueAndWait(std::move(item));
  }
};

} // namespace threadpool

#endif // __THREADPOOL_H__
