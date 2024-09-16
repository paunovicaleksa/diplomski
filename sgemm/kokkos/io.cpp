/***************************************************************************
 *cr
 *cr            (C) Copyright 2010 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/* I/O routines for reading and writing matrices in column-major
 * layout
 */

#include<fstream>
#include<iostream>
#include<vector>
#include <Kokkos_Core.hpp>

bool readColMajorMatrixFile(const char *fn, int &nr_row, int &nr_col, Kokkos::View<float**>& v_data, std::string view_name) {
    std::cerr << "Opening file:"<< fn << std::endl;
    std::fstream f(fn, std::fstream::in);
    if ( !f.good() ) {
        return false;
    }

    // Read # of rows and cols
    f >> nr_row;
    f >> nr_col;

    float data;
    std::cerr << "Matrix dimension: "<<nr_row<<"x"<<nr_col<<std::endl;
    Kokkos::View<float**> v_load(view_name, nr_col, nr_row); // default on host is LayoutRight
    int i = 0, j = 0;
    for(i = 0; i < nr_col & f.good(); i++) {
        for(j = 0; j < nr_row && f.good(); j++) {
            f >> data;
            v_load(i, j) = data;
        }
    }

    if(i != nr_col) return false;

    v_data = v_load;

    return true;
}

bool writeColMajorMatrixFile(const char *fn, int nr_row, int nr_col, Kokkos::View<float**> v_data) {
    std::cerr << "Opening file:"<< fn << " for write." << std::endl;
    std::fstream f(fn, std::fstream::out);
    if ( !f.good() ) {
        return false;
    }

    // Read # of rows and cols
    f << nr_row << " "<<nr_col<<" ";

    float data;
    std::cerr << "Matrix dimension: "<<nr_row<<"x"<<nr_col<<std::endl;
    for(int i = 0; i < nr_col; i++) {
        for(int j = 0; j < nr_row; j++) {
            data = v_data(i, j);
            f << data << ' ';
        }
    }

    f << "\n";
    return true;
}
