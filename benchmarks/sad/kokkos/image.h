/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#ifndef __IMAGE_H
#define __IMAGE_H

#include <Kokkos_Core.hpp>

struct image_i16 {
  int width;
  int height;
//   behaves like a pointer i guess
  Kokkos::View<short*> data;
};


bool load_image ( const char *filename, image_i16& img );
void free_image ( image_i16 *image );

#endif

