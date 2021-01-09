#!/bin/sh
#SBATCH --job-name=measure # Job name
#SBATCH --ntasks=64 # Run on a eight CPU
#SBATCH --time=00:050:00 # Time limit hrs:min:sec
#SBATCH --output=measure%j.out # Standard output and error log
#SBATCH --partition=cl1_all_64C

hostname;

cd ~/hpca-assignment-2020-2021/PartA;
g++ main.cpp -o3 -o diag_mult -I ./header -lpthread;

#collect running time for multithreaded version
./diag_mult data/input_4096.in;
./diag_mult data/input_8192.in;
./diag_mult data/input_16384.in;
./diag_mult data/input_32768.in;

#recompile with debug flag
g++ main.cpp -g -o3 -o diag_mult -I ./header -lpthread;

perf record -o ~/perf_data_4096.out -F 4000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_4096.in s;
perf report -i ~/perf_data_4096.out --source > ~/perf_data_4096.txt

perf record -o ~/perf_data_8192.out -F 4000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_8192.in s;
perf report -i ~/perf_data_8192.out --source > ~/perf_data_8192.txt

perf record -o ~/perf_data_16384.out -F 4000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_16384.in s;
perf report -i ~/perf_data_16384.out --source > ~/perf_data_16384.txt

perf record -o ~/perf_data_32768.out -F 4000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_32768.in s;
perf report -i ~/perf_data_32768.out --source > ~/perf_data_32768.txt
