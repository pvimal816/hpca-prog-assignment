#!/bin/sh
#SBATCH --job-name=measure # Job name
#SBATCH --ntasks=1 # Run on a eight CPU
#SBATCH --time=00:050:00 # Time limit hrs:min:sec
#SBATCH --output=measure%j.out # Standard output and error log
#SBATCH --partition=q2h_24h-2G

hostname;

cd ~/hpca-prog-assignment/PartB;
nvcc -o diag_mult_server ./main.cu -I ./header/ -std=c++11;

echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_4096.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_8192.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_16384.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_32768.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_65536.in;
echo "==========================================================================";
