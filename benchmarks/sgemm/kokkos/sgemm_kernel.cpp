/***************************************************************************
 *cr
 *cr            (C) Copyright 2010 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* 
 * Base C implementation of MM
 * Kernel left unchanged.
 */

#include <iostream>
#include <Kokkos_Core.hpp>

void basicSgemm( char transa, char transb, int m, int n, int k, float alpha, Kokkos::View<float**> A , int lda,
                 Kokkos::View <float**> B, int ldb, float beta, 
                 Kokkos::View<float**> C, int ldc ) {
    if ((transa != 'N') && (transa != 'n')) {
        std::cerr << "unsupported value of 'transa' in regtileSgemm()" << std::endl;
        return;
    }
    
    if ((transb != 'T') && (transb != 't')) {
        std::cerr << "unsupported value of 'transb' in regtileSgemm()" << std::endl;
        return;
    }
    
    Kokkos::parallel_for(Kokkos::MDRangePolicy({0, 0}, {m, n}),
        KOKKOS_LAMBDA (int mm, int nn) {
            float c = 0.0f;
            for (int i = 0; i < k; ++i) {
                float a = A(i, mm); 
                float b = B(i, nn);
                c += a * b;
            }

            C(nn, mm) = C(nn, mm) * beta + alpha * c;
        });

    Kokkos::fence();
}
