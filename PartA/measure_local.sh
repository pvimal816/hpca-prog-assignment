g++ main.cpp -g -o3 -o diag_mult -I ./header -lpthread;

perf record -o analysis/perf_data_4096_local.out -F 20000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_4096.in s;
perf report -i analysis/perf_data_4096_local.out --source > analysis/perf_data_4096_local.txt

perf record -o analysis/perf_data_8192_local.out -F 20000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_8192.in s;
perf report -i analysis/perf_data_8192_local.out --source > analysis/perf_data_8192_local.txt

perf record -o analysis/perf_data_16384_local.out -F 20000 -s -e L1-dcache-load-misses,LLC-load-misses,LLC-store-misses,branch-misses ./diag_mult data/input_16384.in s;
perf report -i analysis/perf_data_16384_local.out --source > analysis/perf_data_16384_local.txt
