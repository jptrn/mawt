# Benchmark for Large Texts 

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 5 \
    --file /scratch/tarnowski/data/ru.wo_punct.wb \
    --file-alpha-bits 32 \
    --compute-effective-alphabet \
    --min 2GiB \
    --ext get_huffman_codes \
    --ext make_tree \
    --ext ctor \
    --ext init \
    --ext split \
    --ext get_borders \
    --ext merge \
    --no-highlight
