/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#ifndef __FILE_H
#define __FILE_H

unsigned short read16u ( FILE *file );
short read16i ( FILE *file );
void write32u ( FILE *file, unsigned int value );
void write16u ( FILE *file, unsigned short value );
void write16i ( FILE *file, short value );

#endif
