#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <parboil.h>
#include <iostream>
#include <fstream>
#include <string>

void divisor_count_and_sum(unsigned int n, unsigned int* pcount,
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
    char *isArithmetic;
    char *isComposite;
    int numThreads;
    int factor = 1000;

	pb_TimerSet timers;
    pb_InitializeTimerSet(&timers);
    
    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

    #pragma omp parallel shared(isArithmetic, isComposite, n, arithmetic_count, composite_count, numThreads, num, factor) 
    {
        #pragma omp single
        {
            numThreads = omp_get_num_threads();
            isArithmetic = (char*) malloc(numThreads * factor);
            isComposite = (char*) malloc(numThreads * factor);
            for(int i = 0; i < numThreads * factor; i++) {
                isArithmetic[i] = 0;
                isComposite[i] = 0;
            }
        }
        while(arithmetic_count < num) {
            int myId = omp_get_thread_num();
            int next_num = n + myId * factor;
            for(int i = 0; i < factor; i++) {
                int index = (next_num - 1) % (numThreads * factor);
                isArithmetic[index] = 0;
                isComposite[index] = 0;
                unsigned int divisor_count;
                unsigned int divisor_sum;
                divisor_count_and_sum(next_num, &divisor_count, &divisor_sum);
                isArithmetic[index] = (divisor_sum % divisor_count == 0);
                isComposite[index] = isArithmetic[index] && divisor_count > 2? 1 : 0;
                next_num = next_num + 1;
            } 

            #pragma omp barrier
            #pragma omp single
            {
                for(int i = 0; i < numThreads * factor; i++) {
                    if(isArithmetic[i]) {
                        arithmetic_count++;
                    }
                    if(isComposite[i]) composite_count++;
                    if(arithmetic_count == num + 1){
                        n = n + i;
                        break;
                    }
                }
            }

            if(arithmetic_count == num + 1) break;

            #pragma omp single
                n += numThreads * factor;
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