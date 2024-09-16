#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <parboil.h>
#include <Kokkos_Core.hpp>

int prime_number(int n) {
    int total;

    total = 0;

    Kokkos::parallel_reduce("reduction", Kokkos::RangePolicy(2, n + 1),
        KOKKOS_LAMBDA (const int& i, int& sum) {
            int prime = 1;
            for (int j = 2; j < i; j++) {
                if ((i % j) == 0) {
                    prime = 0;
                    break;
                }
            }

            sum += prime;
        }, total);

    Kokkos::fence();
    return total;
}

void test(int n_lo, int n_hi, int n_factor);

int main(int argc, char *argv[]) {
    Kokkos::initialize(argc, argv);
    {
        pb_TimerSet timers;
        pb_InitializeTimerSet(&timers);
        int n_factor;
        int n_hi;
        int n_lo;

        printf("\n");
        printf("PRIME TEST\n");

        if (argc != 4){
            n_lo = 1;
            n_hi = 131072;
            n_factor = 2;
        } else {
            n_lo = atoi(argv[1]);
            n_hi = atoi(argv[2]);
            n_factor = atoi(argv[3]);
        }

        pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

        test(n_lo, n_hi, n_factor);

        pb_SwitchToTimer ( &timers, pb_TimerID_NONE );
        pb_PrintTimerSet ( &timers );

        printf("\n");
        printf("PRIME_TEST\n");
        printf("  Normal end of execution.\n");
        printf("\n");
    }
    Kokkos::finalize();

    return 0;
}

void test(int n_lo, int n_hi, int n_factor) {
    int i;
    int n;
    int primes;
    double ctime;

    n = n_lo;

    while (n <= n_hi) {
        primes = prime_number(n);

        printf("%8d  \n",  primes);
        n = n * n_factor;
    }

    return;
}
