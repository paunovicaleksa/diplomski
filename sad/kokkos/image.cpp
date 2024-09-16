/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "image.h"

bool load_image ( const char *filename, image_i16& img ) {
	FILE *file = fopen(filename, "r");

	if ( !file ) {
		fprintf ( stderr, "Cannot find file '%s'\n", filename );
        return false;
	}

	/* Read image dimensions */
	int width  = read16u ( file );
	int height = read16u ( file );

	/* Read image contents */
	// short *data = ( short* ) malloc ( width * height * sizeof ( short ) );
    Kokkos::View<short*> data (std::string(filename), width * height);

	fread ( data.data(), sizeof ( short ), width * height, file );

	fclose ( file );

	/* Create the return data structure */
	// image_i16 *result = ( image_i16* ) malloc ( sizeof ( image_i16 ) );

	img.width  = width;
	img.height = height;
//  copy constructed, only pointer is copied
	img.data   = data;

	return true;
}

void free_image ( image_i16 *image ) {
	free ( image );
}
