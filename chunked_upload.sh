#!/bin/bash

FILE_PATH="$1"
URL="${2:-http://localhost:8080/POST}"

if [[ ! -f "$FILE_PATH" ]]; then
  echo "File not found: $FILE_PATH"
  exit 1
fi

echo "Uploading $FILE_PATH to $URL using chunked encoding..."

curl -v -X POST "$URL" \
  -H "Transfer-Encoding: chunked" \
  --data-binary @"$FILE_PATH"

