#include "benchmarks/common.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
#include "include/UTL/log.hpp" // log::println()

// Libraries to benchmarks against
#include "benchmarks/thirdparty/ankerl/unordered_dense.h" // ankerl::unordered_dense::set<>

// Standard headers
#include <cstdint>         // uint64_t
#include <list>            // list<>, pmr::list<>, size_t
#include <memory_resource> // pmr::polymorphic_allocator, pmr::monotonic_buffer_resource
#include <set>             // set<>, pmr::set<>
#include <type_traits>     // is_same<>
#include <unordered_set>   // unoredered_set<>, pmr::unoredered_set<>
#include <vector>          // vector<>, pmr::vector<>

// ____________________ IMPLEMENTATION ____________________

// What are we trying to measure:
//    - Performance overhead of trivial 'std::pmr::polymorphic_allocator'
//    - Speedup from using various memory resources for various containers

template <class Container, class PmrContainer, std::size_t count, class Func>
void benchmark_push_back(Func&& inserter) {

    static_assert(std::is_same_v<typename Container::value_type, typename PmrContainer::value_type>);

    using value_type = typename Container::value_type;

    benchmark("std::allocator<>", [&] {
        Container container;
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::polymorphic_allocator<T>", [&] {
        PmrContainer container;
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::polymorphic_allocator<std::byte>", [&] {
        PmrContainer container(std::pmr::polymorphic_allocator<std::byte>{});
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::polymorphic_allocator<void*>", [&] {
        PmrContainer container(std::pmr::polymorphic_allocator<void*>{});
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::monotonic_buffer_resource", [&] {
        std::pmr::monotonic_buffer_resource         memory_resourse;
        std::pmr::polymorphic_allocator<value_type> allocator{&memory_resourse};

        PmrContainer container{allocator};
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::monotonic_buffer_resource (pre-allocated stack)", [&] {
        constexpr std::size_t buffer_size = count * sizeof(value_type) * 8;

        std::array<std::byte, buffer_size>          buffer;
        std::pmr::monotonic_buffer_resource         memory_resourse{buffer.data(), buffer.size()};
        std::pmr::polymorphic_allocator<value_type> allocator{&memory_resourse};

        PmrContainer container{allocator};
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::monotonic_buffer_resource (pre-allocated heap)", [&] {
        constexpr std::size_t buffer_size = count * sizeof(value_type) * 8;

        std::vector<std::byte>                      buffer(buffer_size);
        std::pmr::monotonic_buffer_resource         memory_resourse{buffer.data(), buffer.size()};
        std::pmr::polymorphic_allocator<value_type> allocator{&memory_resourse};

        PmrContainer container{allocator};
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::synchronized_pool_resource", [&] {
        std::pmr::synchronized_pool_resource        memory_resourse;
        std::pmr::polymorphic_allocator<value_type> allocator{&memory_resourse};

        PmrContainer container{allocator};
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });

    benchmark("std::pmr::unsynchronized_pool_resource", [&] {
        std::pmr::unsynchronized_pool_resource      memory_resourse;
        std::pmr::polymorphic_allocator<value_type> allocator{&memory_resourse};

        PmrContainer container{allocator};
        for (std::size_t i = 0; i < count; ++i) inserter(container, i);
        DO_NOT_OPTIMIZE_AWAY(container);
    });
}

template <class Container, class PmrContainer, std::size_t count, class Func>
void benchmark_push_back_wrapper(std::string_view container, std::string_view pmr_container, Func&& inserter) {
    static_assert(std::is_same_v<typename Container::value_type, typename PmrContainer::value_type>);

    using value_type = typename Container::value_type;

    println("\n====== BENCHMARKING container growth ======\n");
    println("STD container -> ", container);
    println("PMR container -> ", pmr_container);
    println("Count         -> ", count);
    println("Memory usage  -> ", count * sizeof(value_type) / 1e6, " MB");
    println();

    const std::string title = std::string(container) + " / " + std::to_string(count) + " elements";

    bench.title(title).minEpochTime(5ms).warmup(1000).relative(true); // global options

    benchmark_push_back<Container, PmrContainer, count>(std::forward<Func>(inserter));
}

#define BENCHMARK_GROWTH(container_, pmr_container_, count_, inserter_)                                                \
    benchmark_push_back_wrapper<container_, pmr_container_, count_>(#container_, #pmr_container_, inserter_)

void print_separator() {
    utl::log::println();
    utl::log::println();
    utl::log::println("-----------------------------------------------------------------" | utl::log::color::yellow);
    utl::log::println("-----------------------------------------------------------------" | utl::log::color::yellow);
    utl::log::println();
}

int main() {
    using uint64 = std::uint64_t;

    constexpr std::size_t small  = 50;
    constexpr std::size_t medium = 800;
    constexpr std::size_t large  = 10'000;

    using ankerl_set     = ankerl::unordered_dense::set<uint64>;
    using ankerl_pmr_set = ankerl::unordered_dense::set<uint64, ankerl::unordered_dense::hash<uint64>,
                                                        std::equal_to<uint64>, std::pmr::polymorphic_allocator<uint64>>;

    // Vector
    print_separator();

    BENCHMARK_GROWTH(std::vector<uint64>, std::pmr::vector<uint64>, small,
                     [](auto&& container, std::size_t i) { container.push_back(i); });

    BENCHMARK_GROWTH(std::vector<uint64>, std::pmr::vector<uint64>, medium,
                     [](auto&& container, std::size_t i) { container.push_back(i); });

    BENCHMARK_GROWTH(std::vector<uint64>, std::pmr::vector<uint64>, large,
                     [](auto&& container, std::size_t i) { container.push_back(i); });

    // List
    print_separator();

    BENCHMARK_GROWTH(std::list<uint64>, std::pmr::list<uint64>, small,
                     [](auto&& container, std::size_t i) { container.push_back(i); });

    BENCHMARK_GROWTH(std::list<uint64>, std::pmr::list<uint64>, medium,
                     [](auto&& container, std::size_t i) { container.push_back(i); });

    BENCHMARK_GROWTH(std::list<uint64>, std::pmr::list<uint64>, large,
                     [](auto&& container, std::size_t i) { container.push_back(i); });

    // Set
    print_separator();

    BENCHMARK_GROWTH(std::set<uint64>, std::pmr::set<uint64>, small,
                     [](auto&& container, std::size_t i) { container.emplace(i); });

    BENCHMARK_GROWTH(std::set<uint64>, std::pmr::set<uint64>, medium,
                     [](auto&& container, std::size_t i) { container.emplace(i); });

    BENCHMARK_GROWTH(std::set<uint64>, std::pmr::set<uint64>, large,
                     [](auto&& container, std::size_t i) { container.emplace(i); });

    // Unordered set
    print_separator();

    BENCHMARK_GROWTH(std::unordered_set<uint64>, std::pmr::unordered_set<uint64>, small,
                     [](auto&& container, std::size_t i) { container.emplace(i); });

    BENCHMARK_GROWTH(std::unordered_set<uint64>, std::pmr::unordered_set<uint64>, medium,
                     [](auto&& container, std::size_t i) { container.emplace(i); });

    BENCHMARK_GROWTH(std::unordered_set<uint64>, std::pmr::unordered_set<uint64>, large,
                     [](auto&& container, std::size_t i) { container.emplace(i); });

    // Unordered set (ankerl)
    print_separator();

    BENCHMARK_GROWTH(ankerl_set, ankerl_pmr_set, small, [](auto&& container, std::size_t i) { container.emplace(i); });

    BENCHMARK_GROWTH(ankerl_set, ankerl_pmr_set, medium, [](auto&& container, std::size_t i) { container.emplace(i); });

    BENCHMARK_GROWTH(ankerl_set, ankerl_pmr_set, large, [](auto&& container, std::size_t i) { container.emplace(i); });
}