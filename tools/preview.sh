#!/usr/bin/env bash
# Render every module to frames and build a visual gallery (montage.png +
# index.html flipbook) plus a zip. Dependency-free (uses ad-harness + python3).
#
# Usage: tools/preview.sh [out_dir]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/build/ad-harness"
OUT="${1:-$ROOT/preview-out}"
FRAMES="$OUT/frames"
W=480; H=300; FRAMES_N=144; STRIDE=18  # 8 frames/module

[ -x "$BIN" ] || { echo "build first: cmake --build $ROOT/build -j"; exit 1; }
rm -rf "$OUT"; mkdir -p "$FRAMES"

for m in $("$BIN" --list | tail -n +2 | tr -d '\r'); do
  "$BIN" --module "$m" --render "$FRAMES/$m" --width $W --height $H \
         --frames $FRAMES_N --stride $STRIDE >/dev/null
done

python3 "$ROOT/tools/make_gallery.py" "$FRAMES" "$OUT"
rm -rf "$FRAMES"  # keep the gallery, drop the bulky PPMs

( cd "$OUT" && zip -qr "$OUT/afterdark-previews.zip" . )
echo "done: $OUT/montage.png  $OUT/index.html  $OUT/afterdark-previews.zip"
