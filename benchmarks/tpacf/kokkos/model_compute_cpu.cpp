/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <stdio.h> 

#include "model.h"
#include <Kokkos_Core.hpp>

int doCompute(Kokkos::View<cartesian*> data1, int n1, Kokkos::View<cartesian*> data2, 
	      int n2, int doSelf, Kokkos::View<long long*> data_bins, 
	      int nbins, Kokkos::View<float*> binb) {
    if (doSelf) {
        n2 = n1;
        data2 = data1;
    }

    for (int i = 0; i < ((doSelf) ? n1-1 : n1); i++) {
            const register float xi = data1(i).x;
            const register float yi = data1(i).y;
            const register float zi = data1(i).z;

            Kokkos::parallel_for("j_kernel", Kokkos::RangePolicy((doSelf) ? i+1 : 0, n2),
                KOKKOS_LAMBDA (int j) {
                    register float dot = xi * data2(j).x + yi * data2(j).y + 
                                        zi * data2(j).z;
                    
                    // run binary search
                    register int min = 0;
                    register int max = nbins;
                    register int k, indx;
                    
                    while (max > min+1) {
                        k = (min + max) / 2;
                        if (dot >= binb(k)) 
                            max = k;
                        else 
                            min = k;
                    }
                    
                    if (dot >= binb(min)) {
                        Kokkos::atomic_add(&data_bins(min), 1); /*k = min;*/ 
                    }
                    else if (dot < binb(max)) { 
                        Kokkos::atomic_add(&data_bins(max+1), 1); /*k = max+1;*/ 
                    }
                    else { 
                        Kokkos::atomic_add(&data_bins(max), 1); /*k = max;*/ 
                    }
            });

            Kokkos::fence();
    }

    
    return 0;
}

