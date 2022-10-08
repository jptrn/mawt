# Takes a *.tex file as input, processes it with sqlplottools,
# compiles the file, and displays it for preview

set -ex

if [ -z "$*" ]; then
    echo "No file given!"
    exit 1
fi

FILENAME=$(basename -- $1)
FILEBASE="${FILENAME%.*}"
PDFNAME="${FILEBASE}.pdf"
echo Exporting to $PDFNAME
~/sqlplot-tools/sqlplot-tools-sqlite3 $FILENAME
