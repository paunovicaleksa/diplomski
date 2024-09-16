/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#ifndef __ARGS_H__
#define __ARGS_H__

#include <string>
#include <vector>

struct options {
  std::string data_name;
  std::string output_name;
  std::vector<std::string> random_names;
  int random_count;
  int npoints;
};

void usage(char *name);
void parse_args(int argc, char **argv, options& args);

#endif
