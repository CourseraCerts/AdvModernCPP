#!/usr/bin/env bash
set -euo pipefail
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"
APP_NAME="cppTopicWalker"
PYINSTALLER_BIN=${PYINSTALLER:-pyinstaller}

$PYINSTALLER_BIN \
  --clean \
  --noconfirm \
  --windowed \
  --onefile \
  --name "$APP_NAME" \
  topic_walker/app.py

echo "Linux AppImage-style binary located at dist/${APP_NAME}"
