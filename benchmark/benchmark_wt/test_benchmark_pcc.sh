# Evaluate on PCC plots

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --iterations 3 \
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
    --no-highlight
