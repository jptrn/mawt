# Benchmark for Large Texts 

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 5 \
    --file /scratch/data/large/cc.txt \
    --file-alpha-bits 8 \
    --file /scratch/data/large/dna.txt \
    --file-alpha-bits 8 \
    --file /scratch/data/large/wiki.txt \
    --file-alpha-bits 8 \
    --compute-effective-alphabet \
    --min 16GiB \
    --ext get_huffman_codes \
    --ext make_tree \
    --ext ctor \
    --ext init \
    --ext split \
    --ext get_borders \
    --ext merge \
    --no-highlight
