Simple C++ threadpool
=====================

This is an extremely simple (no optimizations) threadpool using C++11
threads and synchronization primitives.

API
---

To use this threadpool implementation, include `threadpool.h` in your
project. The threadpool is a single class, `threadpool::Threadpool`,
whose methods are described below.

    Threadpool::Threadpool(int n)

Creates a new threadpool with `n` workers. The number of workers is
fixed throughout the life of the threadpool.

    bool Threadpool::Enqueue(std::function<void()>&& f)

Enqueues the function `f` to execute on the threadpool, and returns
immediately. The storage (closure) for `f` will be freed once it
returns. Returns `true` if `f` is executed or `false` if the
threadpool is shutting down.

    bool Threadpool::EnqueueAndWait(std::function<void()>&& f)

Enqueues the function `f` to execute on the threadpool, and waits
until `f` completes. Returns `true` if `f` is executed or `false` if
the threadpool is shutting down.

    void Threadpool::Shutdown()

Shuts down the threadpool. All worker threads exit and are joined
before this method returns. The threadpool first completes any
enqueued work; however, new work is rejected while shutdown is in
progress (returning `false` from enqueueing methods).

Usage notes
-----------

The threadpool accepts rvalue-references (`std::function<void()>&&`)
for work items. The API is commonly used either by passing a lambda
directly (`pool.Enqueue([]() { /* ... */ };`) or by moving a runnable
object into the argument (`pool.Enqueue(std::move(runnable));`).


Copyright and License
=====================

This threadpool implementation is copyright (c) 2017 by Chris Fallin
&lt;cfallin@c1f.net&gt;. It is released under the terms of the GNU
Lesser General Public License, version 3 or later, included as the
file `LICENSE.txt` in this distribution.