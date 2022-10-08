# Benchmark for PCC Texts

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 5 \
    --file /scratch/data/pcc/dblp.xml \
    --file-alpha-bits 8 \
    --file /scratch/data/pcc/dna \
    --file-alpha-bits 8 \
    --file /scratch/data/pcc/english \
    --file-alpha-bits 8 \
    --file /scratch/data/pcc/pitches \
    --file-alpha-bits 8 \
    --file /scratch/data/pcc/proteins \
    --file-alpha-bits 8 \
    --file /scratch/data/pcc/sources \
    --file-alpha-bits 8 \
    --compute-effective-alphabet \
    --ext get_huffman_codes \
    --ext make_tree \
    --ext ctor \
    --ext init \
    --ext split \
    --ext get_borders \
    --ext merge \
    --no-highlight
