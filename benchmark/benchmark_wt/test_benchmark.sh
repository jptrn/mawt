# Pilot benchmark setup, used for quickly evaluating what works and what doesn't

cd ..
./bench.sh ${BIN_DIR}/benchmark_wt \
    --min 256MiB \
    --max 1GiB \
    --iterations 3 \
    --alpha-min 8 \
    --alpha-max 16 \
    --alpha-step 8 \
    --no-highlight
