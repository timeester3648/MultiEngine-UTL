# utl::parallel

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**parallel** module is a lightweight threading library providing an API very similar to [Intel TBB](https://github.com/uxlfoundation/oneTBB).

It implements classic building blocks of concurrent algorithms such as tasks, parallel for, reductions and etc. and provides a sane thread pool implementation for custom concurrency needs.

> [!Important]
> Due to rather extensive API, seeing [usage examples](#examples) first might be helpful.

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Definitions

```cpp
// Thread pool
class ThreadPool {
    // Construction
    ThreadPool() = default;
    explicit ThreadPool(std::size_t thread_count);
    ~ThreadPool();
    
    // Threads
    std::size_t get_thread_count() const;
    void        set_thread_count(std::size_t thread_count);
    
    // Task queue
    template <class Func, class... Args>
    void add_task(Func&& func, Args&&... args);
    
    template <class Func, class... Args>
    std::future<FuncReturnType> add_task_with_future(Func&& func, Args&&... args);
    
    void wait_for_tasks();
    void clear_task_queue();
    
    // Pausing
    void     pause();
    void   unpause();
    bool is_paused() const;
};

// Static thread pool operations
ThreadPool& static_thread_pool();

std::size_t get_thread_count();
void        set_thread_count(std::size_t thread_count);

// Ranges
template <class Iter>
struct Range {
    Range() = delete;
    Range(Iter begin, Iter end);
    Range(Iter begin, Iter end, std::size_t grain_size);
    
    template <class Container> Range(const Container& container);
    template <class Container> Range(      Container& container);
}

template <class Idx>
struct IndexRange {
    IndexRange() = delete;
    IndexRange(Idx first, Idx last);
    IndexRange(Idx first, Idx last, std::size_t grain_size);
}

// Task API
template <class Func, class... Args> void task(Func&& func, Args&&... args);

template <class Func, class... Args>
std::future<FuncReturnType>   task_with_future(Func&& func, Args&&... args);

void wait_for_tasks();

// Parallel-for API
template <class Iter,      class Func> void for_loop(     Range<Iter> range,     Func&& func);
template <class Container, class Func> void for_loop(const Container& container, Func&& func);
template <class Container, class Func> void for_loop(      Container& container, Func&& func);
template <class Idx,       class Func> void for_loop( IndexRange<Idx> range,     Func&& func);

// Reduction API
template <class Iter,      class BinaryOp> auto reduce(     Range<Iter> range,     BinaryOp&& op)
    -> typename Iter::value_type;
template <class Container, class BinaryOp> auto reduce(const Container& container, BinaryOp&& op)
    -> typename Container::value_type;
template <class Container, class BinaryOp> auto reduce(      Container& container, BinaryOp&& op)
    -> typename Container::value_type;

// Pre-defined binary operations
template <class T> struct  sum { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct prod { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  min { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  max { constexpr T operator()(const T& lhs, const T& rhs) const; }
```

> [!Important]
> In most application there is no need to ever work with `ThreadPool` directly, all of the work will be automatically done by `parallel::get_thread_count()`, `parallel::set_thread_count()`, `parallel::task()`, `parallel::for_loop()`, `parallel::reduce()` and etc.

## Methods

### Thread pool

#### Construction

```cpp
ThreadPool() = default;
```

```cpp
explicit ThreadPool(std::size_t thread_count);
```

Creates thread pool with `thread_count` worker threads.

```cpp
~ThreadPool();
```

Finishes all tasks left in the queue then shuts down worker threads.

If the pool was paused, it will automatically resume to finish the work.

#### Threads

```cpp
std::size_t ThreadPool::get_thread_count();
```

Returns current number of worker threads in the thread pool.

```cpp
void ThreadPool::set_thread_count(std::size_t thread_count);
```

Changes the number of worker threads managed by the thread pool to `thread_count`.

#### Task queue

```cpp
template <class Func, class... Args>
void add_task(Func&& func, Args&&... args);
```

Adds a task to execute callable `func` with arguments `args...` (`args...` may be empty).

**Note:** Callables include: function pointers, functors, lambdas, [std::function](https://en.cppreference.com/w/cpp/utility/functional/function), [std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task) and etc.

```cpp
template <class Func, class... Args>
std::future<FuncReturnType> add_task_with_future(Func&& func, Args&&... args);
```

Adds a task to execute callable `func` with arguments `args...` (`args...` may be empty) and returns its [std::future](https://en.cppreference.com/w/cpp/thread/future).

**Note:** `FuncReturnType` evaluates to the return type of the callable `func`.

```cpp
void wait_for_tasks();
```

Blocks current thread execution until all queued tasks are finished.

```cpp
void ThreadPool::clear_task_queue();
```

Clears all currently queued tasks. Tasks already in progress continue running until finished.

#### Pausing

```cpp
void ThreadPool::pause();
```

Stops execution of new tasks from the queue. Use `.unpause()` to resume. Tasks already in progress continue running until finished.

```cpp
void ThreadPool::unpause();
```

Resumes execution of queued tasks.

```cpp
bool ThreadPool::is_paused() const;
```

Returns whether the thread pool is paused.

### Static thread pool

```cpp
ThreadPool& static_thread_pool();
```

Returns a global static instance of the threadpool.

In most cases there is no need to manually maintain the thread pool at call-site, a global thread pool instance gets created automatically upon the first call to `parallel::static_thread_pool()` or any of the parallel algorithm functions.

**Note:** In most cases even the global instance doesn't have to be directly accessed thorough this method, all threading logic will be automatically managed by `parallel::get_thread_count()`, `parallel::set_thread_count()`, `parallel::task()`, `parallel::for_loop()`, `parallel::reduce()` and etc.

```cpp
std::size_t get_thread_count();
```

Returns current number of worker threads in the static thread pool.

```cpp
void set_thread_count(std::size_t thread_count);
```

Changes the number of worker threads managed by the static thread pool to `thread_count`.

### Ranges

```cpp
template <class Iter>
struct Range {
    Range() = delete;
    Range(Iter begin, Iter end);
    Range(Iter begin, Iter end, std::size_t grain_size);
    
    template <class Container> Range(const Container& container);
    template <class Container> Range(      Container& container);
}
```

A lightweight wrapper representing an **iterator range**.

Constructor **(2)** create a range spanning `begin` to `end` and selects grain size automatically, which is **recommended in most cases**.

Constructor **(3)** allows manual selection of `grain_size`.

Constructors **(4)** and **(5)** create a range spanning `container.begin()` to `container.end()` for any container that supports standard member types `Container::iterator` and `Container::const_iterator`.

**Note:** In this context, `grain_size` is a maximum size of subranges, in which the main range gets split up for parallel execution. Splitting up workload into smaller grains can be beneficial for tasks with unpredictable or uneven complexity, but increases the overhead of scheduling & synchronization. By default, the workload is split into `parallel::get_thread_count() * 4` grains.

```cpp
template <class Idx>
struct IndexRange {
    IndexRange() = delete;
    IndexRange(Idx first, Idx last);
    IndexRange(Idx first, Idx last, std::size_t grain_size);
}
```

A lightweight wrapper representing an **index range**.

Constructor **(2)** create a range spanning `first` to `last` and selects grain size automatically, which is **recommended in most cases**.

Constructor **(3)** allows manual selection of `grain_size`.

**Note:** Like all the standard ranges, index range is **exclusive** and does not include `last`.

### Task API

```cpp
template <class Func, class... Args> void task(Func&& func, Args&&... args);
```

Launches asynchronous task to execute callable `func` with arguments `args...`.

```cpp
template <class Func, class... Args>
std::future<FuncReturnType>   task_with_future(Func&& func, Args&&... args);
```

Launches asynchronous task to execute callable `func` with arguments `args...` and returns its [std::future](https://en.cppreference.com/w/cpp/thread/future).

```cpp
void wait_for_tasks();
```

Waits for all currently launched tasks to finish.

### Parallel-for API

```cpp
template <class Iter,      class Func> void for_loop(     Range<Iter> range,     Func&& func);
template <class Container, class Func> void for_loop(const Container& container, Func&& func);
template <class Container, class Func> void for_loop(      Container& container, Func&& func);
```

Executes parallel `for` loop over a range `range` where `func` is a callable with a signature `void(Iter low, Iter high)` that defines how to compute a part of the `for` loop. See the [examples](#parallel-for-loop).

Overloads **(2)** and **(3)** construct range spanning `container.begin()` to `container.end()` automatically.

```cpp
template <class Idx,       class Func> void for_loop( IndexRange<Idx> range,     Func&& func);
```

Executes parallel `for` loop over an **index range** `range` where `func` is a callable with a signature `void(Idx low, Idx high)` that defines how to compute a part of the `for` loop.

### Reduction API

```cpp
template <class Iter,      class BinaryOp> auto reduce(     Range<Iter> range,     BinaryOp&& op)
    -> typename Iter::value_type;
template <class Container, class BinaryOp> auto reduce(const Container& container, BinaryOp&& op)
    -> typename Container::value_type;
template <class Container, class BinaryOp> auto reduce(      Container& container, BinaryOp&& op)
    -> typename Container::value_type;
```

Reduces range `range`  over the binary operation `op` in parallel.

Overloads **(2)** and **(3)** construct range spanning `container.begin()` to `container.end()` automatically.

**Note 1:** Binary operation can be anything with a signature `T(const T&, const T&)` or `T(T, T)`.

**Note 2:** Be wary of passing binary operations as function pointers since that makes inlining more difficult. Lambdas and functor-classes don't experience the same issue, see [pre-defined binary operations](#pre-defined-binary-operations).

#### Pre-defined binary operations

```cpp
template <class T> struct  sum { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct prod { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  min { constexpr T operator()(const T& lhs, const T& rhs) const; }
template <class T> struct  max { constexpr T operator()(const T& lhs, const T& rhs) const; }
```

Pre-defined binary operations for `parallel::reduce()`.

## Examples

### Launching async tasks

[ [Run this code]() ]
```cpp
using namespace utl;

const std::string message = "<some hypothetically very large message>";

// Log the message asynchronously
parallel::task([&]{ std::ofstream("log.txt") << message });

// ... do some other work in the meantime ...

// Destructor will automatically wait for the task to finish before exiting 'main()',
// otherwise you can wait manually
parallel::wait_for_tasks();
```

Output:
```
<some hypothetically very large message>
```

### Launching async tasks with future

[ [Run this code]() ]
```cpp
using namespace utl;

double some_heavy_computation(double x) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x + 32;
}

// ...

// Lauch the computation asynchronously and get its future
auto future = parallel::task_with_future(some_heavy_computation, 10);

// ... do some other work in the meantime ...

// Get the value from std::future, if the task isn't finished it will wait for it to finish
const double result = future.get();

assert( result == 42 );
```

### Parallel for loop

[ [Run this code]() ]
```cpp
double f(double x) { return std::exp(std::sin(x)); }

// ...

using namespace utl;

std::vector<double> vals(1'000'000, 0.5);

// (optional) Select the number of threads 
parallel:set_thread_count(8);

// Apply f() to all elements of the vector
parallel::for_loop(vals, [&](auto low, auto high) {
    for (auto it = low; it != high; ++it) *it = f(*it);
});

// Apply f() to the fist half of the vector
parallel::for_loop(parallel::IndexRange{0, vals.size() / 2}, [&](auto low, auto high) {
    for (auto it = low; it != high; ++it) *it = f(*it);
});
```

### Reducing a range over a binary operation

[ [Run this code]() ]
```cpp
using namespace utl;

const std::vector<double> vals(50'000'000, 2);

// Reduce container over a binary operation
const double sum = parallel::reduce(vals, parallel::sum<double>);

assert( sum == 50'000'000 * 2 );

// Reduce range over a binary operation
const double subrange_sum = parallel::reduce(parallel::Range{vals.begin() + 1000, vals.end()}, parallel::sum<double>);

assert( subrange_sum == 50'000'000 * 2 - 1000 );
```

## Benchmarks

While `utl::parallel` does not claim to provide superior performance to complex vendor-optimized libraries such as [OpenMP](https://en.wikipedia.org/wiki/OpenMP), [Intel TBB](https://github.com/uxlfoundation/oneTBB), [MPI](https://www.open-mpi.org) and etc., it provides a significant boost in both speed and convenience relative to the explicit use of [std::async](https://en.cppreference.com/w/cpp/thread/async) and [std::thread](https://en.cppreference.com/w/cpp/thread/thread) due to its ability to reuse threads and automatically distribute workload.

Below are some of the [benchmarks](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/benchmarks/benchmark_parallel.cpp) comparing performance of different approaches on basic tasks:

```
====== BENCHMARKING ON: Parallel vector sum ======

Threads           -> 4
N                 -> 25000000
Data memory usage -> 190.73486328125 MiB

| relative |               ms/op |                op/s |    err% |     total | Parallel vector sum
|---------:|--------------------:|--------------------:|--------:|----------:|:--------------------
|   100.0% |               18.86 |               53.03 |    2.1% |      2.34 | `Serial version`
|   380.0% |                4.96 |              201.53 |    0.1% |      0.61 | `OpenMP reduce`
|   288.2% |                6.54 |              152.83 |    1.1% |      0.88 | `Naive std::async()`
|   378.3% |                4.99 |              200.59 |    0.1% |      0.61 | `parallel::reduce()`

|----------------------------------------|--------------------|
|                                  Method|         Control sum|
|----------------------------------------|--------------------|
|                                  Serial|         2.50000e+07|
|                           OpenMP reduce|         2.50000e+07|
|                        Naive std::async|         2.50000e+07|
|                      parallel::reduce()|         2.50000e+07|


====== BENCHMARKING ON: Repeated matrix multiplication ======

Threads           -> 4
N                 -> 600
repeats           -> 20
Data memory usage -> 8.23974609375 MiB

| relative |               ms/op |                op/s |    err% |     total | Repeated matrix multiplication
|---------:|--------------------:|--------------------:|--------:|----------:|:-------------------------------
|   100.0% |            1,112.58 |                0.90 |    0.0% |     80.11 | `Serial version`
|   292.3% |              380.69 |                2.63 |    0.1% |     27.26 | `OpenMP parallel for`
|   207.7% |              535.73 |                1.87 |   11.0% |     37.04 | `Naive std::async()`
|   427.3% |              260.39 |                3.84 |    0.3% |     18.24 | `parallel::task()`
|   429.3% |              259.14 |                3.86 |    0.1% |     18.68 | `parallel::for_loop()`

|----------------------------------------|--------------------|
|                                  Method|         Control sum|
|----------------------------------------|--------------------|
|                                  Serial|         1.07912e+09|
|                     OpenMP parallel for|         1.07912e+09|
|                      Naive std::async()|         1.07912e+09|
|                        parallel::task()|         1.07912e+09|
|                    parallel::for_loop()|         1.07912e+09|

// Note 1: Notice extremely unstable measurement for `std::async()` (aka large `err%`),
//         creating new threads is a highly unpredictable task due to OS scheduling.
//
// Note 2: Not sure why OpenMP doesn't give as much speedup as expected.
//
// Note 3: Speedup over 4x can happens on small matrices (like here)
//         due to utilization of muliple cache lines in a distributed case.
```
