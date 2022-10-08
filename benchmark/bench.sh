# Pilot benchmark setup, used for quickly evaluating what works and what doesn't

set -e

if [ -z "$*" ]; then
    echo "No benchmark command given!"
    exit 1
fi

LOGDIR=$(realpath log)
TIMESTAMP=$(date +"%Y-%m-%d--%H-%M-%S")
OUTFILE=${LOGDIR}/temp/results-${TIMESTAMP}.out

echo Logging to ${OUTFILE}...

echo > ${OUTFILE}
echo Running WT Benchmark at ${TIMESTAMP}... >> ${OUTFILE}
echo Git branch: $(git rev-parse --abbrev-ref HEAD) >> ${OUTFILE}
echo Git commit: $(git rev-parse HEAD) >> ${OUTFILE}
echo Git commit \(short\): $(git rev-parse --short HEAD) >> ${OUTFILE}
echo Build Type: ${BUILD_TYPE} >> ${OUTFILE}
echo >> ${OUTFILE}
echo Command: $* >> ${OUTFILE}
echo >> ${OUTFILE}

$* | tee -a ${OUTFILE}
