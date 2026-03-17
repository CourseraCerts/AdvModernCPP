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
  --name "$APP_NAME" \
  --osx-bundle-identifier "com.topicwalker.app" \
  topic_walker/app.py

echo "macOS .app bundle available under dist/${APP_NAME}.app"
