# Get Text Info

cd ..
./bench.sh ${BIN_DIR}/text_analyzer \
    --file /scratch/data/pcc/dblp.xml \
    --file-bits 8 \
    --file /scratch/data/pcc/dna \
    --file-bits 8 \
    --file /scratch/data/pcc/english \
    --file-bits 8 \
    --file /scratch/data/pcc/pitches \
    --file-bits 8 \
    --file /scratch/data/pcc/proteins \
    --file-bits 8 \
    --file /scratch/data/pcc/sources \
    --file-bits 8 \
    --file /scratch/data/lightweight/chr22.dna \
    --file-bits 8 \
    --file /scratch/data/lightweight/etext99 \
    --file-bits 8 \
    --file /scratch/data/lightweight/gcc-3.0.tar \
    --file-bits 8 \
    --file /scratch/data/lightweight/howto \
    --file-bits 8 \
    --file /scratch/data/lightweight/jdk13c \
    --file-bits 8 \
    --file /scratch/data/lightweight/linux-2.4.5.tar \
    --file-bits 8 \
    --file /scratch/data/lightweight/rctail96 \
    --file-bits 8 \
    --file /scratch/data/lightweight/rfc \
    --file-bits 8 \
    --file /scratch/data/lightweight/sprot34.dat \
    --file-bits 8 \
    --file /scratch/data/lightweight/w3c2 \
    --file-bits 8 \
    --file /scratch/data/large/cc.txt \
    --file-bits 8 \
    --file /scratch/data/large/dna.txt \
    --file-bits 8 \
    --file /scratch/data/large/wiki.txt \
    --file-bits 8 \
    --prefix 16GiB \
    --no-highlight
