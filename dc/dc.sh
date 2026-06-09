#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
cd "$ROOT"

require_file() {
  if [ ! -f "$1" ]; then
    echo "dc.sh: missing required file: $1" >&2
    exit 1
  fi
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "dc.sh: required tool not found in PATH: $1" >&2
    echo "Build gdC.elf first (cd dc && make) and ensure KOS tools are on PATH." >&2
    exit 1
  fi
}

require_file gdC.elf
require_file cd/gba_bios.bin

if [ ! -d cd/gbaDC ]; then
  echo "dc.sh: missing ROM directory: cd/gbaDC" >&2
  exit 1
fi

"$ROOT/../scripts/sync-game-config.sh"

require_cmd sh-elf-objcopy
require_cmd mkdcdisc

run_scramble() {
  if command -v scramble >/dev/null 2>&1; then
    scramble "$@"
  elif [ -n "${KOS_BASE:-}" ] && [ -x "$KOS_BASE/utils/scramble/scramble" ]; then
    "$KOS_BASE/utils/scramble/scramble" "$@"
  else
    echo "dc.sh: required tool not found: scramble" >&2
    echo "Add KOS utils to PATH or set KOS_BASE." >&2
    exit 1
  fi
}

sh-elf-objcopy -R .stack -O binary gdC.elf gdC.bin
rm -f cd/1ST_READ.BIN
run_scramble gdC.bin 1ST_READ.BIN
cp 1ST_READ.BIN cd/
mkdcdisc -n gbapspDC -N -f cd/gba_bios.bin -d cd/gbaDC -e gdC.elf -o gbapspDC.cdi
