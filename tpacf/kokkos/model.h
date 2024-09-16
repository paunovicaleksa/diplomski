/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#ifndef __MODEL_H__
#define __MODEL_H__


#define D2R M_PI/180.0
#define R2D 180.0/M_PI
#define R2AM 60.0*180.0/M_PI

#define bins_per_dec 5
#define min_arcmin 1.0
#define max_arcmin 10000.0

#define NUM_BINS 20

#include <Kokkos_Core.hpp>

typedef unsigned long hist_t;

struct spherical 
{
  float ra, dec;  // latitude, longitude pair
};
 
struct cartesian 
{
  float x, y, z;  // cartesian coodrinates
};

int readdatafile(const char *fname, Kokkos::View<cartesian*> data, int npoints);

int doCompute(Kokkos::View<cartesian*> data1, int n1, Kokkos::View<cartesian*> data2, 
	      int n2, int doSelf, Kokkos::View<long long*> data_bins, 
	      int nbins, Kokkos::View<float*> binb);

#endif
