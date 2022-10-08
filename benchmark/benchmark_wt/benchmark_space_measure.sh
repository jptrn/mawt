# Benchmark for Space Measurement

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 1 \
    --min 64MiB \
    --max 256MiB \
    --alpha-min 4 \
    --alpha-max 16 \
    --alpha-step 4 \
    --ext get_huffman_codes \
    --ext make_tree \
    --ext ctor \
    --ext init \
    --ext split \
    --ext get_borders \
    --ext merge \
    --no-highlight
