# Break down DD algorithms by time

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --min 256MiB \
    --max 1GiB \
    --iterations 3 \
    --alpha-min 8 \
    --alpha-max 16 \
    --alpha-step 8 \
    --ext ctor \
    --ext init \
    --ext split \
    --ext get_borders \
    --ext merge \
    --no-highlight
