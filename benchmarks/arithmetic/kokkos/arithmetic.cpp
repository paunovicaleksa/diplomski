#include <stdio.h>
#include <stdlib.h>
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
        // should be changed for different platforms
        int factor = 1000;
        int concurrency = Kokkos::DefaultExecutionSpace::concurrency();

        pb_TimerSet timers;
        pb_InitializeTimerSet(&timers);

        int start = 1;

        pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
        while(arithmetic_count <= num) {
            int current_arithmetic_count = 0, current_composite_count = 0;
            Kokkos::parallel_reduce("reduce_kernel", Kokkos::RangePolicy(start, start + factor),
                KOKKOS_LAMBDA (int i, int& is_arithmetic, int& is_composite) {
                    unsigned int divisor_count;
                    unsigned int divisor_sum;
                    divisor_count_and_sum(i, &divisor_count, &divisor_sum);
                    if(divisor_sum % divisor_count == 0) {
                        is_arithmetic++;
                        if(divisor_count > 2) {
                            is_composite++;
                        }
                    }

            }, current_arithmetic_count, current_composite_count);

            Kokkos::fence();

            if(arithmetic_count + current_arithmetic_count >= num) {
                break;
            }

            arithmetic_count += current_arithmetic_count;
            composite_count += current_composite_count;

            start += factor;    
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