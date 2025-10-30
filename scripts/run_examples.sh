#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# Prefer common build dirs in order
for cand in "${root_dir}/build" "${root_dir}/build-linux" "${root_dir}/build-gcc"; do
  if [[ -d "$cand" ]]; then
    build_dir="$cand"
    break
  fi
done

if [[ -z "${build_dir:-}" || ! -d "$build_dir" ]]; then
  echo "Build directory not found at $build_dir. Run cmake configure+build first." >&2
  exit 1
fi

run() {
  local exe="$1"
  if [[ -x "$exe" ]]; then
    echo "Running $(basename "$exe")"
    "$exe"
    echo
  else
    echo "Missing executable: $exe" >&2
  fi
}

run "$build_dir/example"
run "$build_dir/example_lru"
run "$build_dir/example_lfu"
run "$build_dir/example_arc"
run "$build_dir/example_concurrency"
run "$build_dir/example_mixed"
