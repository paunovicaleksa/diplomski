#include <stdio.h>
#include <stdlib.h>
#include <parboil.h>
#include <omp.h>
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

	pb_TimerSet timers;
    pb_InitializeTimerSet(&timers);
    
    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

    #pragma omp parallel shared(arithmetic_count, composite_count, n)
    {
        #pragma omp single
        {   
            int numThreads = omp_get_num_threads();
            int factor = 1000;
            char *isArithmetic = (char*) malloc(numThreads * factor);
            char *isComplex = (char*) malloc(numThreads * factor);
            for(int i = 0; i < numThreads * factor; i++) {
                isArithmetic[i] = 0;
                isComplex[i] = 0;
            }

            while(arithmetic_count <= num) {
                for(int i = 0; i < numThreads; i++) {
                    int next_num = n + i * factor;
                    #pragma omp task firstprivate(next_num)
                    {  
                        for(int j = 0; j < factor; j++) {
                            int index = (next_num - 1) % (numThreads * factor);
                            isArithmetic[index] = 0;
                            isComplex[index] = 0;
                            unsigned int divisor_count;
                            unsigned int divisor_sum;
                            divisor_count_and_sum(next_num, &divisor_count, &divisor_sum);
                            isArithmetic[index] = (divisor_sum % divisor_count == 0);
                            isComplex[index] = isArithmetic[index] && divisor_count > 2? 1 : 0;
                            next_num = next_num + 1;
                        } 
                    }
                }

                #pragma omp taskwait
                for(int i = 0; i < numThreads * factor; i++) {
                    if(isArithmetic[i]) {
                        arithmetic_count++;
                    }
                    if(isComplex[i]) composite_count++;
                    if(arithmetic_count == num + 1){
                        n = n + i;
                        break;
                    }

                }

                if(arithmetic_count == num + 1) break;

                n += numThreads * factor;
            }
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