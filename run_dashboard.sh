#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
if command -v xdg-open >/dev/null 2>&1; then
  xdg-open "$(pwd)/index.html" >/dev/null 2>&1 &
elif command -v firefox >/dev/null 2>&1; then
  firefox "$(pwd)/index.html" >/dev/null 2>&1 &
else
  echo "Open index.html in your Ubuntu browser."
fi
