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

int doCompute(struct cartesian *data1, int n1, struct cartesian *data2, 
	      int n2, int doSelf, long long *data_bins, 
	      int nbins, float *binb) {
    int i, j, k;
    if (doSelf) {
        n2 = n1;
        data2 = data1;
    }
    // #pragma omp parallel for 
    for (i = 0; i < ((doSelf) ? n1-1 : n1); i++) {
        #pragma omp parallel for
        for (j = ((doSelf) ? i+1 : 0); j < n2; j++) {
            register float dot = data1[i].x * data2[j].x + data1[i].y * data2[j].y + 
                                 data1[i].z * data2[j].z;
            
            // run binary search
            register int min = 0;
            register int max = nbins;
            register int k, indx;
            
            while (max > min+1) {
                k = (min + max) / 2;
                if (dot >= binb[k]) 
                    max = k;
                else 
                    min = k;
            }

            // #pragma omp critical	  
            if (dot >= binb[min]) {
            #pragma omp atmoic 
                data_bins[min] += 1; /*k = min;*/ 
            }
            else if (dot < binb[max]) { 
            #pragma omp atomic 
                data_bins[max+1] += 1; /*k = max+1;*/ 
            }
            else { 
            #pragma omp atomic 
                data_bins[max] += 1; /*k = max;*/ 
            }
        }
    }
    
    return 0;
}

