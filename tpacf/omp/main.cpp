/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include <parboil.h>

#include "args.h"
#include "model.h"

int main( int argc, char **argv ) {
    pb_TimerSet timers;

    int rf, k, nbins, npd, npr;

    float w;
    size_t memsize;
    FILE *outfile;

    pb_InitializeTimerSet(&timers);

    options args;
    parse_args( argc, argv, args );

    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
        
    nbins = (int)floor(bins_per_dec * (log10(max_arcmin) - 
                        log10(min_arcmin)));
    memsize = (nbins+2)*sizeof(long long);
        
    // memory for bin boundaries
    std::vector<float> binb(nbins + 1);
    for (k = 0; k < nbins+1; k++) {
        binb[k] = cos(pow(10, log10(min_arcmin) + 
                k*1.0/bins_per_dec) / 60.0*D2R);
    }
        
    // memory for DD
    std::vector<long long> DD(memsize, 0);
        
    // memory for RR
    std::vector<long long> RRS(memsize, 0);
        
    // memory for DR
    std::vector<long long> DRS(memsize, 0);
        
    // memory for input data
    std::vector<cartesian> data(args.npoints);
    std::vector<cartesian> random (args.npoints);

    printf("Min distance: %f arcmin\n", min_arcmin);
    printf("Max distance: %f arcmin\n", max_arcmin);
    printf("Bins per dec: %i\n", bins_per_dec);
    printf("Total bins  : %i\n", nbins);
    pb_SwitchToTimer(&timers, pb_TimerID_IO);
    // read data file
    npd = readdatafile(args.data_name.c_str(), &data.front(), args.npoints);
    if (npd != args.npoints) {
        fprintf(stderr, 
            "Error: read %i data points out of %i\n", 
            npd, args.npoints
        );
        return(1);
    }

    pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
    // compute DD
    doCompute(&data.front(), npd, NULL, 0, 1, &DD.front(), nbins, &binb.front());

    // loop through random data files
    for (rf = 0; rf < args.random_count; rf++) {
        // read random file
        pb_SwitchToTimer(&timers, pb_TimerID_IO);
        npr = readdatafile(args.random_names[rf].c_str(), &random.front(), args.npoints);
        if (npr!= args.npoints) {
            fprintf(stderr, 
                "Error: read %i data points out of %i\n", 
                npr, args.npoints
            );
            return(1);
        }

        pb_SwitchToTimer(&timers, pb_TimerID_COMPUTE);
        // compute RR
        doCompute(&random.front(), npr, NULL, 0, 1, &RRS.front(), nbins, &binb.front());

        // compute DR
        doCompute(&data.front(), npd, &random.front(), npr, 0, &DRS.front(), nbins, &binb.front());
    }

    // compute and output results
    if ((outfile = fopen(args.output_name.c_str(), "w")) == NULL) {
        fprintf(stderr, 
            "Unable to open output file %s for writing, assuming stdout\n", 
            args.output_name
        );
        outfile = stdout;
    }

    pb_SwitchToTimer(&timers, pb_TimerID_IO);
    for (k = 1; k < nbins+1; k++) {
        fprintf(outfile, "%d\n%d\n%d\n", DD[k], DRS[k], RRS[k]);      
    }

    if(outfile != stdout) fclose(outfile);

    pb_SwitchToTimer(&timers, pb_TimerID_NONE);
    pb_PrintTimerSet(&timers);
    // free memory
    return 0;
}

