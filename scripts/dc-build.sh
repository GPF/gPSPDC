#!/bin/sh
set -eu

IMAGE="${DC_BUILD_IMAGE:-einsteinx2/dcdev-kos-toolchain:gcc-9__v2.0.0}"
ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"

if ! command -v docker >/dev/null 2>&1; then
  echo "docker is required to run the Dreamcast cross-compile." >&2
  echo "Install Docker, or build manually with KOS: cd dc && make" >&2
  exit 1
fi

docker pull "$IMAGE" >/dev/null 2>&1 || true

# Default to the real ELF target. Plain `make` resolves to the KOS `subdirs`
# goal from Makefile.rules ("Nothing to be done for 'subdirs'") and builds
# nothing, so an explicit target is required for CI to actually cross-compile.
if [ "$#" -eq 0 ]; then
  set -- all
fi

exec docker run --rm -v "$ROOT:/src" -w /src/dc "$IMAGE" make "$@"
