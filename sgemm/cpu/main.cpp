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
#include <parboil.h>

// I/O routines
extern bool readColMajorMatrixFile(const char *fn, int &nr_row, int &nr_col, std::vector<float>&v);
extern bool writeColMajorMatrixFile(const char *fn, int, int, std::vector<float>&);

// main kernel

extern void basicSgemm( char transa, char transb, int m, int n, int k, float alpha, const float *A, int lda, const float *B, int ldb, float beta, float *C, int ldc );

int main (int argc, char *argv[]) {
	pb_TimerSet timers;
    pb_InitializeTimerSet(&timers);
    int32_t iarg = 0;
    std::string output_file = "";
    std::string input_fileA = "";
    std::string input_fileB = "";
    std::string input_fileBT = "";
    std::string error_message = "Usage: ./matmul -o out_file in_file1 in_file2 in_file2Transposed";

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

    std::vector<float> matA;
    int n_rowA, n_colA;
    readColMajorMatrixFile(input_fileA.c_str(), n_rowA, n_colA, matA);



    int n_rowBT, n_colBT;
    std::vector<float> matB;
    readColMajorMatrixFile(input_fileBT.c_str(), n_colBT, n_rowBT, matB);
    std::vector<float> matC(n_rowA * n_colBT, 0);

    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);

    basicSgemm('N', 'T', n_rowA, n_colBT, n_colA, 1.0f, &matA.front(), n_rowA, &matB.front(), n_colBT, 0.0f, &matC.front(), n_rowA);

    pb_SwitchToTimer(&timers, pb_TimerID_IO);

    writeColMajorMatrixFile(output_file.c_str(), n_rowA, n_colBT, matC);

	pb_SwitchToTimer ( &timers, pb_TimerID_NONE );
	pb_PrintTimerSet ( &timers );

    return 0;
}
