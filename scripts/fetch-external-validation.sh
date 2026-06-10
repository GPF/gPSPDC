#!/usr/bin/env sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
DEST_DIR="${EXTERNAL_VALIDATION_DIR:-"$ROOT_DIR/external/gba-validation"}"

clone_or_update() {
  name="$1"
  url="$2"
  dest="$DEST_DIR/$name"

  if [ "${DRY_RUN:-0}" = "1" ]; then
    echo "Would fetch $name from $url into $dest"
    return 0
  fi

  if [ -d "$dest/.git" ]; then
    echo "Updating $name"
    git -C "$dest" fetch --depth=1 origin
    git -C "$dest" checkout --detach FETCH_HEAD
  else
    echo "Cloning $name"
    git clone --depth=1 "$url" "$dest"
  fi
}

if [ "${DRY_RUN:-0}" != "1" ]; then
  mkdir -p "$DEST_DIR"
fi

clone_or_update "jsmolka-gba-tests" \
  "https://github.com/jsmolka/gba-tests.git"
clone_or_update "SingleStepTests-ARM7TDMI" \
  "https://github.com/SingleStepTests/ARM7TDMI.git"
clone_or_update "destoer-gba_tests" \
  "https://github.com/destoer/gba_tests.git"
clone_or_update "ladystarbreeze-GBA-Test-Collection" \
  "https://github.com/ladystarbreeze/GBA-Test-Collection.git"
clone_or_update "rustboyadvance-ng" \
  "https://github.com/michelhe/rustboyadvance-ng.git"
clone_or_update "gba-kit" \
  "https://github.com/macabeus/gba-kit.git"
clone_or_update "miniz" \
  "https://github.com/richgel999/miniz.git"
clone_or_update "stb" \
  "https://github.com/nothings/stb.git"

echo "External validation repos are in $DEST_DIR"
