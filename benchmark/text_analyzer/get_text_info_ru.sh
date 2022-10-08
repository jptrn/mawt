# Get Stats for RU

cd ..
./bench.sh ${BIN_DIR}/text_analyzer \
    --file /scratch/tarnowski/data/ru.wo_punct.wb \
    --file-bits 32 \
    --prefix 2GiB \
    --no-highlight
