// collatz_superfast.cpp

#include <thread>
#include <iostream>
#include <chrono>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

constexpr int CACHE_LIMIT = 10000000;
unsigned int collatz_cache[CACHE_LIMIT] = {0};

// Fast branchless Collatz step
__attribute__((always_inline)) inline unsigned long long collatz_step(unsigned long long n) {
    return (n & 1) ? (n * 3 + 1) : (n >> 1);
}

// Highly optimized, loop-unrolled Collatz sequence length
__attribute__((always_inline)) inline unsigned int collatz_length(unsigned long long n) {
    unsigned long long original = n;
    unsigned int len = 1;

    while (n != 1) {
        if (n < CACHE_LIMIT && collatz_cache[n]) {
            len += collatz_cache[n] - 1;
            break;
        }

        // Loop unrolling by 2 steps
        n = collatz_step(n);
        len++;
        if (n == 1) break;

        n = collatz_step(n);
        len++;
    }

    if (original < CACHE_LIMIT)
        collatz_cache[original] = len;

    return len;
}

unsigned long long parse_input(const string& s) {
    size_t pos = s.find("**");
    if (pos != string::npos) {
        unsigned long long base = stoull(s.substr(0, pos));
        unsigned long long exp = stoull(s.substr(pos + 2));
        unsigned long long result = 1;
        while (exp--) result *= base;
        return result;
    }
    return stoull(s);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <start> <end>" << endl;
        return 1;
    }

    unsigned long long start = parse_input(argv[1]);
    unsigned long long end = parse_input(argv[2]);

    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    auto t1 = high_resolution_clock::now();

    vector<thread> threads(num_threads);
    vector<unsigned long long> max_n(num_threads);
    vector<unsigned int> max_len(num_threads);

    unsigned long long chunk = (end - start + 1) / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        unsigned long long sub_start = start + t * chunk;
        unsigned long long sub_end = (t == num_threads - 1) ? end : sub_start + chunk - 1;

        threads[t] = thread([t, sub_start, sub_end, &max_n, &max_len]() {
            unsigned long long local_max_n = 0;
            unsigned int local_max_len = 0;

            for (unsigned long long i = sub_start; i <= sub_end; ++i) {
                unsigned int len = collatz_length(i);
                if (len > local_max_len) {
                    local_max_len = len;
                    local_max_n = i;
                }
            }

            max_n[t] = local_max_n;
            max_len[t] = local_max_len;
        });
    }

    for (auto& th : threads) th.join();

    unsigned long long final_n = 0;
    unsigned int final_len = 0;
    for (int t = 0; t < num_threads; ++t) {
        if (max_len[t] > final_len) {
            final_len = max_len[t];
            final_n = max_n[t];
        }
    }

    auto t2 = high_resolution_clock::now();
    duration<double> elapsed = t2 - t1;

    cout << "Collatz " << start << " to " << end << " completed in " << elapsed.count() << " seconds\n";
    cout << "Max length: " << final_len << " at n = " << final_n << endl;

    return 0;
}
