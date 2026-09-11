#!/bin/sh
# Preflight validation for the Dreamcast disc layout before building a CDI.
# Catches the common "the disc won't boot cleanly" mistakes without needing the
# KOS toolchain or an emulator. Run standalone, or via dc/dc.sh (which calls it).
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
CD="$ROOT/dc/cd"
GBADC="$CD/gbaDC"
BIOS="$CD/gba_bios.bin"
BIOS_MD5="a860e8c0b6d573d191e4ec7db1b1e4f6"
BIOS_SIZE=16384

fail=0
err() { echo "check-disc: $1" >&2; fail=1; }

# BIOS: required, exact size, and (if a hasher is available) exact MD5.
if [ ! -f "$BIOS" ]; then
  err "missing BIOS: $BIOS (16384 bytes, MD5 $BIOS_MD5)"
else
  size=$(wc -c < "$BIOS" | tr -d ' ')
  [ "$size" = "$BIOS_SIZE" ] || err "BIOS wrong size: $size bytes (expected $BIOS_SIZE)"
  if command -v md5sum >/dev/null 2>&1; then
    got=$(md5sum "$BIOS" | cut -d' ' -f1)
    [ "$got" = "$BIOS_MD5" ] || err "BIOS MD5 mismatch: $got (expected $BIOS_MD5)"
  elif command -v md5 >/dev/null 2>&1; then
    got=$(md5 -q "$BIOS")
    [ "$got" = "$BIOS_MD5" ] || err "BIOS MD5 mismatch: $got (expected $BIOS_MD5)"
  fi
fi

# ROM directory and per-game config must exist.
[ -d "$GBADC" ] || err "missing ROM directory: $GBADC"
[ -f "$GBADC/game_config.txt" ] || err "missing $GBADC/game_config.txt (run scripts/sync-game-config.sh)"

# autoload.txt is optional, but if present it must name a ROM that exists in
# gbaDC/ — otherwise the freshly built disc fails on first boot.
AUTOLOAD="$GBADC/autoload.txt"
if [ -f "$AUTOLOAD" ]; then
  rom=$(head -n 1 "$AUTOLOAD" | tr -d '\r' | sed 's/[[:space:]]*$//')
  if [ -z "$rom" ]; then
    echo "check-disc: autoload.txt is empty; disc will open the ROM browser" >&2
  elif [ ! -f "$GBADC/$rom" ]; then
    err "autoload.txt names '$rom' but $GBADC/$rom does not exist"
  else
    echo "check-disc: autoload ROM ok: $rom"
  fi
fi

if [ "$fail" -ne 0 ]; then
  echo "check-disc: disc layout has problems (see above)" >&2
  exit 1
fi

echo "check-disc: disc layout ok"
