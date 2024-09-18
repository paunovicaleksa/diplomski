#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <parboil.h>
#include <iostream>
#include <fstream>
#include <string>

inline void divisor_count_and_sum(unsigned int n, unsigned int* pcount,
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
    std::string help = "./arithmetic num output_file";
    if(argc != 3) {
        std::cerr << help << std::endl;
        return 1;
    }

    int num = atoi(argv[1]);
    std::string file_name = argv[2];
    unsigned int arithmetic_count = 0;
    unsigned int composite_count = 0;
    unsigned int n = 1;
    int factor = 1000;

	pb_TimerSet timers;
    pb_InitializeTimerSet(&timers);

    int start = 1;
    
    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
    while(arithmetic_count <= num) {
        int current_arithmetic_count = 0, current_composite_count = 0;
        #pragma omp parallel for default(none) shared(start, factor) reduction(+:current_arithmetic_count, current_composite_count)
        for(int i = start; i < start + factor; i++) {
            unsigned int divisor_count;
            unsigned int divisor_sum;
            divisor_count_and_sum(i, &divisor_count, &divisor_sum);
            if(divisor_sum % divisor_count == 0) {
                current_arithmetic_count++;
                if(divisor_count > 2) {
                    current_composite_count++;
                }
            }

        } 

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
    
    return 0;
}