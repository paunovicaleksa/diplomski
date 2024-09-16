/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#ifndef __IMAGE_H
#define __IMAGE_H

struct image_i16 {
  int width;
  int height;
  short *data;
};


image_i16 * load_image ( const char *filename );
void free_image ( image_i16 *image );

#endif

