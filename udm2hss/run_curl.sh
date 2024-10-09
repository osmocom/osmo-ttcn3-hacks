#!/bin/sh

URL=$1
DATA=$2

curl \
    --http2 --http2-prior-knowledge \
    -H "Content-Type: application/json" \
    -d "${DATA}" \
    "${URL}"

