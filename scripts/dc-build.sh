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

exec docker run --rm -v "$ROOT:/src" -w /src/dc "$IMAGE" make "$@"
