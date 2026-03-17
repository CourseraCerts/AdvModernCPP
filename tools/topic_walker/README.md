# Topic Walker

Topic Walker is a PyQt-based desktop application for macOS and Linux that lets you capture C++ study prompts and the tags that relate to them. It keeps prompts, tags, and relationships in a local SQLite database so you can review them later either by prompt or by tag.

## Features
- Capture prompts with one source tag plus any number of related tags
- Inspect data "by prompt" (editable tags) or "by tag" with drill-down navigation between both views
- Clickable tags/ prompts plus a navigation trail with Back + Clear filter actions
- Drag-select rows in the table to make quick comparisons
- Uses native system theme and supports standard zoom-in/zoom-out shortcuts (`Cmd/Ctrl +` and `Cmd/Ctrl -`)
- First-launch installer asks where to store your database/settings and verifies directory permissions
- Settings dialog shows the storage location and lets you uninstall (removes database/settings/application bundle if present)

## Project layout
```
requirements.md   # Original product requirements
README.md         # This file, includes build/package instructions
scripts/          # Helper scripts for packaging
topic_walker/     # Application sources
```

## Running locally
1. Install dependencies (Python 3.10+ recommended):
   ```bash
   python -m venv .venv
   source .venv/bin/activate
   pip install -r requirements.txt
   ```
2. Launch the UI:
   ```bash
   python -m topic_walker.app
   ```
3. On first launch you will be prompted to choose where Topic Walker should store its database and settings. The default is `~/.cppTopicWalker`. If you select a directory you do not have write access to, the app shows a timed error dialog and returns to the directory picker.

## Packaging
Both packaging flows rely on [PyInstaller](https://pyinstaller.org). Install the tooling once per machine:
```bash
pip install pyinstaller
```

### macOS
1. Ensure you are on macOS and inside your virtual environment.
2. Run the packaging helper:
   ```bash
   ./scripts/package_mac.sh
   ```
3. PyInstaller creates `dist/cppTopicWalker.app`. You can copy this bundle into `/Applications` (administrator permissions may be required). Signed/notarized distribution is outside this sample scope.
4. To create a DMG for distribution, wrap the `.app` using the built-in `hdiutil`:
   ```bash
   hdiutil create -volname cppTopicWalker -srcfolder dist/cppTopicWalker.app dist/cppTopicWalker.dmg
   ```

### Linux (Linux Mint tested)
1. Activate your Python environment on Mint Linux.
2. Run:
   ```bash
   ./scripts/package_linux.sh
   ```
3. PyInstaller generates a GUI-friendly binary at `dist/cppTopicWalker`. You can place it under `/opt/cppTopicWalker/` and add a `.desktop` file pointing to it for full desktop integration.

## Installer & settings
- The installer stores everything under the directory you chose:
  - `database/topicwalker.db` — SQLite database
  - `settings/config.json` plus additional preferences
- If you pick a directory without permissions, the installer shows the error dialog for up to 60 seconds (you can close it sooner) and then returns to the directory prompt.
- Settings dialog (accessible via the *Settings* button) displays the selected storage path and includes an **Uninstall Topic Walker** button. This removes the storage directory, pointer file, and a `/Applications/cppTopicWalker.app` bundle if present, then exits the application.

## Requirements
See `requirements.txt` for the runtime dependencies. PyInstaller is only required for packaging.

