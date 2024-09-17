/***************************************************************************
 *cr
 *cr            (C) Copyright 2010 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* 
 * Main entry of dense matrix-matrix multiplication kernel
 */

#include <iostream>
#include <vector>
#include <getopt.h>
#include <Kokkos_Core.hpp>
#include <parboil.h>

// I/O routines
bool readColMajorMatrixFile(const char *fn, int &nr_row, int &nr_col, Kokkos::View<float**>& v_data, std::string view_name);
extern bool writeColMajorMatrixFile(const char *fn, int nr_row, int nr_col, Kokkos::View<float**> v_data);

// main kernel

extern void basicSgemm( char transa, char transb, int m, int n, int k, float alpha, Kokkos::View<float**> A , int lda,
                 Kokkos::View <float**> B, int ldb, float beta, 
                 Kokkos::View<float**> C, int ldc );

int main (int argc, char *argv[]) {
    Kokkos::initialize(argc, argv);
    {
        pb_TimerSet timers;
        pb_InitializeTimerSet(&timers);
        int32_t iarg = 0;
        std::string output_file = "";
        std::string input_fileA = "";
        std::string input_fileB = "";
        std::string input_fileBT = "";
        std::string error_message = "Usage: ./sgemm -o out_file in_file1 in_file2 in_file2Transposed";

        if(argc < 6) {
            std::cout << error_message << std::endl;
            return 1;
        }

        while((iarg = getopt(argc, argv, "o:")) != -1) {
            switch(iarg) {
                case 'o':
                    output_file = optarg;
                    break;
                default:
                    std::cerr << error_message << std::endl;
                    return 1;
            }
        }
        
        if(optind != (argc - 3)) {
            std::cerr << error_message << std::endl;
            return 1;
        }

        input_fileA = argv[optind++];
        input_fileB = argv[optind++];
        input_fileBT = argv[optind];

        pb_SwitchToTimer(&timers, pb_TimerID_IO);

        std::cout << input_fileA << " " << input_fileB << " " << input_fileBT << std::endl;

        int n_rowA, n_colA;
        Kokkos::View<float**> matA;
        if(!readColMajorMatrixFile(input_fileA.c_str(), n_rowA, n_colA, matA, "matA")) {
            std::cerr << "Error reading matrix A\n";
            return 1;
        }

        int n_rowBT, n_colBT;
        Kokkos::View<float**> matB;

        if(!readColMajorMatrixFile(input_fileBT.c_str(), n_colBT, n_rowBT, matB, "matB")){
            std::cerr << "Error reading matrix B\n";
            return 1;
        } 

        Kokkos::View<float**> matC("matC", n_colBT, n_rowA);

        pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

        basicSgemm('N', 'T', n_rowA, n_colBT, n_colA, 1.0f, matA, n_rowA, matB, n_colBT, 0.0f, matC, n_rowA);

        pb_SwitchToTimer(&timers, pb_TimerID_IO);

        writeColMajorMatrixFile(output_file.c_str(), n_rowA, n_colBT, matC);

        pb_SwitchToTimer ( &timers, pb_TimerID_NONE );
        pb_PrintTimerSet ( &timers );
    }
    Kokkos::finalize();

    return 0;
}
