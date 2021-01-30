#!/bin/sh
#SBATCH --job-name=measure # Job name
#SBATCH --ntasks=1 # Run on a eight CPU
#SBATCH --time=00:050:00 # Time limit hrs:min:sec
#SBATCH --output=measure%j.out # Standard output and error log
#SBATCH --partition=q1m_2h-1G

hostname;

cd ~/hpca-assignment-2020-2021/PartB;
nvcc -o diag_mult_server ./main.cu -I ./header/;

echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_4096.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_8192.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_16384.in;
echo "==========================================================================";
nvprof ./diag_mult_server ./data/input_32768.in;
echo "==========================================================================";