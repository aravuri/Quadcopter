#!/usr/bin/env bash

if [ $# -eq 0 ]; then
    echo "Please specify directory path for input files"
    exit 1
fi

INPUT_FILES="${1}"
OUTPUT_DIR="${INPUT_FILES}/JPEGs"
mkdir -p "${OUTPUT_DIR}"

for d in "${INPUT_FILES}"/*
do
    for f in "${d}"/*
    do
        FILENAME=$(basename "$f")
        OUTPUT_FILENAME="${FILENAME%.*}".jpg
        EXTENSION="${FILENAME##*.}"
        if [ "${EXTENSION}" == "tif" ] || [ "${EXTENSION}" == "png" ]; then
            echo processing "${FILENAME}" "-->" "${OUTPUT_DIR}"/"${OUTPUT_FILENAME}"
            convert "$f" -quiet -density 300 -units pixelsperinch "${OUTPUT_DIR}"/"${OUTPUT_FILENAME}"
        fi
    done
done
