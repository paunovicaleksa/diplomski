/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "args.h"
#include <getopt.h>

extern char *optarg;

void usage(char *name) {
    std::cout << "Usage: ./tpacf < -p num -n num > [ -o file_name ] < data_file > < random_file_1 ... random_file_n >" << std::endl;
    std::cout << "Options:\n";
    std::cout << "-p num       Specify the number of points each data and random file contains" << std::endl;
    std::cout << "-n num       Specify the number of random_files" << std::endl;
    std::cout << "-o file_name Specify the file name to write to, otherwise stdout" << std::endl;

    exit(1);
}

void parse_args(int argc, char **argv, options& args) {
    int c;

    args.data_name = "";
    
    args.random_count = 0;
    args.npoints = 0;
    args.output_name = "";
    bool p = false, n = false;
    
    while ((c = getopt(argc, argv, "n:p:o:")) != EOF) {
        switch (c) {
            case 'n':
                args.random_count = std::stoi(optarg);
                p = true;
                break;
            case 'o':
                args.output_name = optarg;
                break;
            case 'p':
                args.npoints = std::stoi(optarg);
                n = true;
                break;
            default:
                usage(argv[0]);
        }
    }

    if(!p || !n || optind != (argc - (args.random_count + 1))) usage(argv[0]);

    args.data_name = argv[optind++];

    while(optind < argc) {
        args.random_names.push_back(argv[optind++]);
    }
}
