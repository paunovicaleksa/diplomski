#!/bin/bash
function show_help {
    echo "./run.sh [ options ] -b benchmark [implementation]..."
    echo "If no implementations are specified, run all implementations available for that benchmark"
    echo "Options: "
    echo "-h            print this help and exit."
    echo "-c            compile benchmark/implementiation(s) only, do not run."
    echo "-l            call make clean for benchmark/implementation(s). Presupposes compile. Override other flags"
    echo "-b benchmark  specify the benchmark. Currently, running only a single benchmark is supported."
    echo "-k dir        Path to kokkos directory. Default is ~/mups/kokkos-4.4.00"
    echo "Example usage:"
    echo "Running the sgemm benchmark with omp and kokkos:"
    echo "./run.sh -b sgemm omp kokkos"
    exit 1
}

function compile_benchmark {
    build_error=0

    for platform in $platforms
    do
        if [ $platform != "datasets/" ]
        then
            pushd $platform >> /dev/null
            echo " "
            echo "BUILDING $platform"
            echo " "
            if [ -z $make_clean ]; then
                make "KOKKOS_PATH=$KOKKOS_PATH" 2> "$project_root_dir/build_error_"$benchmark"_$platform"
            else 
                make "$make_clean" 2> "$project_root_dir/build_error_"$benchmark"_$platform"
            fi
            if [ $? -ne 0 ]; then
                echo " "
                echo "Error builiding $benchmark/$platform, output dumped to build_error_"$benchmark"_$platform"
                build_error=1
            else 
                rm $project_root_dir/build_error_"$benchmark"_$platform
            fi
            popd >> /dev/null
        fi
    done
    
    if [ $build_error -ne 0 ]; then
        echo " "
        echo "Build errors detected, exiting."
        exit 1
    fi
}

function run_benchmark {
    run_error=0

    for platform in $platforms; do
        OLD_IFS=$IFS
        IFS=$'\n'
        pushd $platform >> /dev/null
        echo " " 
        echo "RUNNING $benchmark/$platform"
        echo " "
        i=0
        for input in $(cat ../input_data); do
            echo " " 
            echo "RUNNING INPUT $i" 
            echo " "
            args=$(echo "$input" | sed "s/platform/$platform/g")
            TMP=$IFS
            IFS=$OLD_IFS
            ./$benchmark $args 
            if [ $? -ne 0 ]; then
                echo "ERRORS DETECTED WHILE RUNNING $benchmark/$platform, INPUT $i"
                echo EXITING
                exit 1
            fi
            IFS=$TMP
            let i++
        done
        IFS=$OLD_IFS
        popd >> /dev/null
    done
}

function test_benchmark {
    OLD_IFS=$IFS
    IFS=$'\n'
    test_error=0
    for output in $(cat output_data); do
        pushd $platform >> /dev/null
        gold_version=$(echo "$output" | sed "s/platform/out/g" )
        TMP=$IFS
        IFS=$OLD_IFS
        for platform in $platforms; do
            platform_version=$(echo $output | sed "s/platform/$platform/g" )
            diff $platform_version $gold_version 2> $project_root_dir/diff_"$benchmark"_"platform"
            if [ $? -ne 0 ]; then
                echo "TEST FAILED for $benchmark/$platform, check $project_root_dir/diff_"$benchmark"_"platform""
            else 
                echo "TEST PASSED for $benchmark/$platform"
                rm -f $project_root_dir/diff_"$benchmark"_"platform"
            fi
        done
        IFS=$TMP
        popd >> /dev/null
    done
    IFS=$OLD_IFS
}

benchmark=
compile=0
make_clean=
KOKKOS_PATH=~/mups/kokkos-4.4.00

project_root_dir=$(pwd)

while getopts "hlk:cb:" option; do
    case $option in
        h) 
            show_help
            ;;
        k) 
            KOKKOS_PATH=$OPTARG
            ;;
        l) 
            compile=1
            make_clean="clean"
            ;;
        b)  
            benchmark=$OPTARG
            ;;  
        c)
            compile=1
            ;;
        *)
            show_help
            ;;
    esac
done


if [ -z $benchmark ] || [ ! -d benchmarks/$benchmark ]; then
    echo "No such directory: $benchmark"
    show_help
fi

pushd benchmarks/$benchmark > /dev/null

shift $((OPTIND-1))
platforms=$@
if [ -z "$platforms" ]; then
    platforms=$(ls -d */ | sed -E "s:(.*)/:\1:g" | grep -v datasets)
fi

# building 
compile_benchmark

echo " "
echo "BUILDING FINISHED for $benchmark"
echo " " 

if [ $compile -eq 1 ]; then
    exit 0
fi

# run benchmark for specified platforms
run_benchmark

# run tests, simple diff will suffice for now.
test_benchmark

popd > /dev/null