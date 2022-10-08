# Benchmark for Lightweight Texts 

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 5 \
    --file /scratch/data/lightweight/chr22.dna \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/etext99 \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/gcc-3.0.tar \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/howto \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/jdk13c \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/linux-2.4.5.tar \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/rctail96 \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/rfc \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/sprot34.dat \
    --file-alpha-bits 8 \
    --file /scratch/data/lightweight/w3c2 \
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
