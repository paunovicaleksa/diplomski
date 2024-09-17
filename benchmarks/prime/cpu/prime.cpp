#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <parboil.h>
#include <iostream>
#include <fstream>
#include <string>

int prime_number(int n) {
    int i;
    int j;
    int prime;
    int total;

    total = 0;

    for (i = 2; i <= n; i++) {
        prime = 1;
        for (j = 2; j < i; j++) {
        if ((i % j) == 0) {
            prime = 0;
            break;
        }
        }
    
        total = total + prime;
    }
    return total;
}

int test(int n_lo, int n_hi, int n_factor, std::string fn);

int main(int argc, char *argv[]) {
	pb_TimerSet timers;
    pb_InitializeTimerSet(&timers);
    int n_factor;
    int n_hi;
    int n_lo;

    std::string help = "./prime n_lo n_hi n_factor out_file";
    std::string file_name = "";

    printf("\n");
    printf("PRIME TEST\n");

    if (argc != 5){
        std::cerr << help << std::endl;
        return 1;
    } else {
        n_lo = atoi(argv[1]);
        n_hi = atoi(argv[2]);
        n_factor = atoi(argv[3]);
        file_name = argv[4];
    }

    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

    test(n_lo, n_hi, n_factor, file_name);

	pb_SwitchToTimer ( &timers, pb_TimerID_NONE );
	pb_PrintTimerSet ( &timers );

    printf("\n");
    printf("PRIME_TEST\n");
    printf("  Normal end of execution.\n");
    printf("\n");

    return 0;
}

int test(int n_lo, int n_hi, int n_factor, std::string fn) {
    std::fstream f(fn, std::fstream::out);
    if(!f.good()) {
        std::cerr << "Cannot open file: " << fn << std::endl;
        return 1;
    }
    int i;
    int n;
    int primes;
    double ctime;

    n = n_lo;

    while (n <= n_hi) {
        primes = prime_number(n);

        f << primes << " ";
        n = n * n_factor;
    }

    f << '\n';

    return 0;
}
