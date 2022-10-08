# Benchmark for Suffix Arrays

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 5 \
    --min 64MiB \
    --max 256MiB \
    --suffix-array \
    --ext get_huffman_codes \
    --ext make_tree \
    --ext ctor \
    --ext init \
    --ext split \
    --ext get_borders \
    --ext merge \
    --no-highlight
