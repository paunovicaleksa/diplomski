#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <parboil.h>
#include <Kokkos_Core.hpp>
#include <iostream>
#include <fstream>
#include <string>

KOKKOS_INLINE_FUNCTION void divisor_count_and_sum(unsigned int n, unsigned int* pcount,
                           unsigned int* psum) {
    unsigned int divisor_count = 1;
    unsigned int divisor_sum = 1;
    unsigned int power = 2;
    for (; (n & 1) == 0; power <<= 1, n >>= 1) {
        ++divisor_count;
        divisor_sum += power;
    }
    for (unsigned int p = 3; p * p <= n; p += 2) {
        unsigned int count = 1, sum = 1;
        for (power = p; n % p == 0; power *= p, n /= p) {
            ++count;
            sum += power;
        }
        divisor_count *= count;
        divisor_sum *= sum;
    }
    if (n > 1) {
        divisor_count *= 2;
        divisor_sum *= n + 1;
    }
    *pcount = divisor_count;
    *psum = divisor_sum;
}

template<class Scheduler>
struct TaskFunctor {
    // to ret
    using value_type = Kokkos::pair<int, int>;
    int  start, end;
    TaskFunctor( int sstart, int eend): start(sstart), end(eend) { }

    template <class TeamMember> 
    KOKKOS_INLINE_FUNCTION
    void operator() (TeamMember& team_member, Kokkos::pair<int, int>& result) {
        int arithmetic_count = 0;
        int composite_count = 0;
        for(int n = start; n < end; n++){
            unsigned int divisor_count;
            unsigned int divisor_sum;
            divisor_count_and_sum(n, &divisor_count, &divisor_sum);
            if (divisor_sum % divisor_count != 0)
                continue;
            ++arithmetic_count;
            if (divisor_count > 2) {
                ++composite_count;
            }
        }
        result.first = arithmetic_count;
        result.second = composite_count;
    }
};

int main(int argc, char** argv) {
    Kokkos::initialize(argc, argv);
    {
        std::string help = "./arithmetic num output_file";
        if(argc != 3) {
            std::cerr << help << std::endl;
            return 1;
        }

        int num = atoi(argv[1]);
        std::string file_name = argv[2];
        unsigned int arithmetic_count = 0;
        unsigned int composite_count = 0;
        unsigned int n;

        int factor = 1000;
        // does not make sense on a GPU
        int concurrency = Kokkos::DefaultExecutionSpace::concurrency();
        using scheduler_type = Kokkos::TaskScheduler<Kokkos::DefaultExecutionSpace>;
        using memory_space = scheduler_type::memory_space ;
        using memory_pool_type = scheduler_type::memory_pool;
        size_t memoy_size = concurrency * factor * sizeof(TaskFunctor<scheduler_type>);
        auto mpool = memory_pool_type(memory_space{}, memoy_size);
        auto scheduler = scheduler_type(mpool);

        pb_TimerSet timers;
        pb_InitializeTimerSet(&timers);

        int start = 1;

        pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

        while(arithmetic_count <= num) {
            std::vector<Kokkos::BasicFuture<Kokkos::pair<int, int>, scheduler_type>> futures(concurrency);
            // spawn the tasks
            for(auto& fut : futures) {
                fut = Kokkos::host_spawn(
                    Kokkos::TaskSingle(scheduler),
                    TaskFunctor<scheduler_type>(start, start + factor)
                );
                start += factor;
            }

            // wait on the scheduler
            Kokkos::wait(scheduler);
            int current_arithmetic_count = 0, current_composite_count = 0;

            // collect all results
            for(auto& fut : futures) {
                current_arithmetic_count += fut.get().first;
                current_composite_count += fut.get().second;
            }

            if(arithmetic_count + current_arithmetic_count >= num) {
                start -= concurrency * factor;
                break;
            }

            arithmetic_count += current_arithmetic_count;
            composite_count += current_composite_count;
         }

        for (n = start; arithmetic_count <= num; ++n) {
            unsigned int divisor_count;
            unsigned int divisor_sum;
            divisor_count_and_sum(n, &divisor_count, &divisor_sum);
            if (divisor_sum % divisor_count != 0)
                continue;
            ++arithmetic_count;
            if (divisor_count > 2) {
                ++composite_count;
            }
        }

        pb_SwitchToTimer(&timers, pb_TimerID_IO);
        std::fstream f(file_name, std::fstream::out);
        if(!f.good()) {
            std::cerr << "Cannot open file: " << file_name << std::endl;
            return 1;
        }


        f << arithmetic_count << "th arithmetic number is " << n << std::endl;
        f << "Number of composite arithmetic numbers <= " << n  << ":"  << composite_count << std::endl;

        pb_SwitchToTimer ( &timers, pb_TimerID_NONE );
        pb_PrintTimerSet ( &timers );
    }

    Kokkos::finalize();
    return 0;
}