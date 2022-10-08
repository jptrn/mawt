# Break down Huffman algorithms by time

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --min 256MiB \
    --max 1GiB \
    --iterations 3 \
    --alpha-min 8 \
    --alpha-max 16 \
    --alpha-step 8 \
    --ext get_huffman_codes \
    --ext make_tree \
    --no-highlight
