#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include "omp.h"

template <typename T>
long long serial_sum(std::vector<T> &vec)
{
    long long sum = 0;

    for (std::size_t i = 0; i != vec.size(); ++i)
    {
        sum += vec[i];
    }

    return sum;
}

template <typename T>
long long adder(std::vector<T> vec, std::size_t begin, std::size_t end)
{
    long long sum = 0;

    for (std::size_t i = begin; i != end; ++i)
    {
        sum += vec[i];
    }

    return sum;
}

template <typename T>
long long parallel_sum(std::vector<T> &vec)
{
    std::size_t n_threads = std::thread::hardware_concurrency();
    std::size_t length = std::distance(vec.begin(), vec.end());
    std::size_t elements_per_thread = length / n_threads;
    std::size_t begin = 0;
    std::size_t end = elements_per_thread;
    long long final_sum = 0;

    std::vector<long long> sums(n_threads);
    std::vector<std::thread> threads(n_threads - 1);

    for (std::size_t i = 0; i != threads.size(); ++i)
    {
        threads[i] = std::thread([&](long long sum) { sum = adder(vec, begin, end); }, std::ref(sums[i]));

        begin = end;
        end = end + elements_per_thread;
    }

    sums[n_threads - 1] = adder(vec, begin, end);

    for (std::size_t i = 0; i != threads.size(); ++i)
    {
        threads[i].join();
    }

    final_sum = adder(sums, 0, sums.size());
    return final_sum;
}

int main()
{
    // SETUP
    std::random_device gen;
    std::uniform_int_distribution<int> dist(-10, 10);
    std::vector<int> vector_a(10'000'000, 0);
    std::generate(vector_a.begin(), vector_a.end(), [&]() { return dist(gen); });

    // SERIELL
    auto start = std::chrono::high_resolution_clock::now();
    long long sum_vector = serial_sum(vector_a);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end - start;
    std::cout << std::endl << "Serial time in ms: " << ms.count();
    std::cout << std::endl << "Serial Sum: " << sum_vector << std::endl;

    // PARALLEL
    start = std::chrono::high_resolution_clock::now();
    sum_vector = parallel_sum(vector_a);
    end = std::chrono::high_resolution_clock::now();
    ms = end - start;
    std::cout << std::endl << "Parallel time in ms: " << ms.count();
    std::cout << std::endl << "Parallel Sum: " << sum_vector << std::endl;

    // REDUCE
    start = std::chrono::high_resolution_clock::now();
    sum_vector = std::reduce(vector_a.begin(), vector_a.end());
    end = std::chrono::high_resolution_clock::now();
    ms = end - start;
    std::cout << std::endl << "Reduce time in ms: " << ms.count();
    std::cout << std::endl << "Reduce Sum: " << sum_vector << std::endl;

    // ACCUMULATE
    start = std::chrono::high_resolution_clock::now();
    sum_vector = std::accumulate(vector_a.begin(), vector_a.end(), 0, std::plus<int>());
    end = std::chrono::high_resolution_clock::now();
    ms = end - start;
    std::cout << std::endl << "Accumulate time in ms: " << ms.count();
    std::cout << std::endl << "Accumulate Sum: " << sum_vector << std::endl;

    return 0;
}