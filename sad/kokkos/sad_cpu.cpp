/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <stdlib.h>
#include "sad.h"


void sad4_cpu ( Kokkos::View<short*> blk_sad, Kokkos::View<short*> frame, Kokkos::View<short*> ref, int mb_width, int mb_height, int img_size ) {
	/* For each block */
    Kokkos::parallel_for("sad4_cpu", 
        Kokkos::MDRangePolicy({0, 0, 0, 0}, {mb_height,  mb_width, 2 * SEARCH_RANGE + 1, 2 * SEARCH_RANGE + 1}), 
        KOKKOS_LAMBDA (int mb_y, int mb_x, int position_y,  int position_x) {
            int macroblock_sad_start_index = SAD_TYPE_7_IX ( mb_width * mb_height ) + 
                                                ( mb_y * mb_width + mb_x ) * ( SAD_TYPE_7_CT * MAX_POS_PADDED );
            int current_frame_start_index =  mb_y * 256 * mb_width + mb_x * 16 ;
            auto macroblock_sad = Kokkos::subview(blk_sad, Kokkos::pair(macroblock_sad_start_index, img_size));
            auto current_frame = Kokkos::subview(frame, Kokkos::pair(current_frame_start_index, img_size));
            /* Go to the starting offset in blk_sad */
            /* Each 4x4 block in the macroblock */
            for ( int block_y = 0; block_y < 4; ++block_y ) {
                for ( int block_x = 0; block_x < 4; ++block_x ) {
                    int frame_y	= mb_y * 16;
                    int	frame_x = mb_x * 16;

                    int width  = mb_width * 16;
                    int height = mb_height * 16;

                    /* search position */
                    int position = ( position_y ) * ( 2 * SEARCH_RANGE + 1 ) + ( position_x );

                    unsigned short sad = 0;

                    /* Each pixel */
                    for ( int y = 0; y < 4; ++y ) {
                        for ( int x = 0; x < 4; ++x ) {
                            /* Get reference pixel coordinate, clipped to image boundary */
                            int reference_x = frame_x + ( position_x - SEARCH_RANGE) + ( block_x * 4 ) + x;
                            int reference_y = frame_y + ( position_y - SEARCH_RANGE) + ( block_y * 4 ) + y;

                            if ( reference_x < 0 ) {
                                reference_x = 0;
                            } else if ( reference_x >= width ) {
                                reference_x = width - 1;
                            }

                            if ( reference_y < 0 ) {
                                reference_y = 0;
                            } else if ( reference_y >= height ) {
                                reference_y = height - 1;
                            }

                            unsigned int b = ref(reference_y * width + reference_x);
                            unsigned int a = current_frame(( block_y * 4 + y ) * width + ( block_x * 4 + x ));

                            sad += abs ( static_cast<int> (a - b) );
                        }
                    }

                    /* Save the SAD */
                    macroblock_sad(MAX_POS_PADDED*( 4 * block_y + block_x ) + position) = sad;
                }
            }
        });

    Kokkos::fence();
}


inline void add_subviews(Kokkos::View<short*> x, Kokkos::View<short*> y, Kokkos::View<short*> z) {
	for ( int count = 0; count < (MAX_POS); ++count) {
		z(count) = x(count) + y(count);
	}
}

void larger_sads ( Kokkos::View<short*> sads, int mbs, int img_size) {
	/* x and y are inputs to vector addition */
	/* z is the output of vector addition */

	const int size = 41 * MAX_POS_PADDED * mbs;
	Kokkos::parallel_for("larger_sads", mbs,  
        KOKKOS_LAMBDA (int macroblock) {
            /* Block type 6 */
            for ( int block_y = 0; block_y < 2; ++block_y ) {
                for ( int block_x = 0; block_x < 4; ++block_x ) {
                    int x_index = ( SAD_TYPE_7_IX ( mbs ) + macroblock * SAD_TYPE_7_CT * MAX_POS_PADDED + ( 8 * block_y + block_x ) * MAX_POS_PADDED); 
                    int y_index = x_index + 4 * MAX_POS_PADDED;
                    int z_index = ( SAD_TYPE_6_IX ( mbs ) + macroblock * SAD_TYPE_6_CT * MAX_POS_PADDED + ( 4 * block_y + block_x ) * MAX_POS_PADDED);

                    auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                    auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                    auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                    add_subviews ( x, y, z );
                }
            }

            /* Block type 5 */
            for ( int block_y = 0; block_y < 4; ++block_y ) {
                for ( int block_x = 0; block_x < 2; ++block_x ) {
                    int x_index = ( SAD_TYPE_7_IX ( mbs ) + macroblock * SAD_TYPE_7_CT * MAX_POS_PADDED + ( 4 * block_y + 2 * block_x ) * MAX_POS_PADDED);
                    int y_index = x_index + MAX_POS_PADDED;
                    int z_index = ( SAD_TYPE_5_IX ( mbs ) + macroblock * SAD_TYPE_6_CT * MAX_POS_PADDED + ( 2 * block_y + block_x ) * MAX_POS_PADDED);

                    auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                    auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                    auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                    add_subviews ( x, y, z );
                }
            }

            /* Block type 4 */
            for ( int block_y = 0; block_y < 2; ++block_y ) {
                for ( int block_x = 0; block_x < 2; ++block_x ) {
                    int x_index = ( SAD_TYPE_5_IX ( mbs ) + macroblock * SAD_TYPE_5_CT * MAX_POS_PADDED + ( 4 * block_y + block_x ) * MAX_POS_PADDED);
                    int y_index = x_index + 2 * MAX_POS_PADDED;
                    int z_index = ( SAD_TYPE_4_IX ( mbs ) + macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED + ( 2 * block_y + block_x ) * MAX_POS_PADDED);

                    auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                    auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                    auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                    add_subviews ( x, y, z );
                }
            }

            /* Block type 3 */
            {
                int x_index = ( SAD_TYPE_4_IX ( mbs ) + macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED);
                int y_index = x_index + 2 * MAX_POS_PADDED;
                int z_index = (SAD_TYPE_3_IX ( mbs ) + macroblock * SAD_TYPE_3_CT * MAX_POS_PADDED);

                auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                add_subviews ( x, y, z );
            }

            {
                int x_index = ( SAD_TYPE_4_IX ( mbs ) + macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED + MAX_POS_PADDED);
                int y_index = x_index + 2 * MAX_POS_PADDED;
                int z_index = ( SAD_TYPE_3_IX ( mbs ) + macroblock * SAD_TYPE_3_CT * MAX_POS_PADDED + MAX_POS_PADDED);

                auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                add_subviews ( x, y, z );
            }

            /* Block type 2 */
            {
                int x_index = ( SAD_TYPE_4_IX ( mbs ) + macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED);
                int y_index = x_index + MAX_POS_PADDED;
                int z_index = ( SAD_TYPE_2_IX ( mbs ) + macroblock * SAD_TYPE_2_CT * MAX_POS_PADDED);

                auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                add_subviews ( x, y, z );
            }

            {
                int x_index = ( SAD_TYPE_4_IX ( mbs ) + macroblock * SAD_TYPE_4_CT * MAX_POS_PADDED + 2 * MAX_POS_PADDED);
                int y_index = x_index + MAX_POS_PADDED;
                int z_index = ( SAD_TYPE_2_IX ( mbs ) + macroblock * SAD_TYPE_2_CT * MAX_POS_PADDED + MAX_POS_PADDED);

                auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                add_subviews ( x, y, z );
            }

            /* Block type 1 */
            {
                int x_index = ( SAD_TYPE_2_IX ( mbs ) + macroblock * SAD_TYPE_2_CT * MAX_POS_PADDED);
                int y_index = x_index + MAX_POS_PADDED;
                int z_index = ( SAD_TYPE_1_IX ( mbs ) + macroblock * SAD_TYPE_1_CT * MAX_POS_PADDED);

                auto x = Kokkos::subview(sads, Kokkos::pair(x_index, img_size));
                auto y = Kokkos::subview(sads, Kokkos::pair(y_index, img_size));
                auto z = Kokkos::subview(sads, Kokkos::pair(z_index, img_size));

                add_subviews ( x, y, z );
            }
        });
}
