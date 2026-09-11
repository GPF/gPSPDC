#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/game_config.txt"
DEST="$ROOT/dc/cd/gbaDC/game_config.txt"

if [ ! -f "$SRC" ]; then
  echo "missing game_config.txt at $SRC" >&2
  exit 1
fi

cp "$SRC" "$DEST"
