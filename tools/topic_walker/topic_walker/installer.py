"""Installer helpers for Topic Walker."""
from __future__ import annotations

import json
import os
import shutil
from pathlib import Path
from typing import Optional

from PyQt6 import QtCore, QtGui, QtWidgets


DEFAULT_STORAGE = Path.home() / ".cppTopicWalker"
POINTER_FILE = Path.home() / ".cppTopicWalker_launcher.json"
CONFIG_NAME = "config.json"


class StoragePaths(QtCore.QObject):
    """Manages persistence of the storage directory."""

    storage_ready = QtCore.pyqtSignal(Path)

    def __init__(self, parent: Optional[QtCore.QObject] = None) -> None:
        super().__init__(parent)
        self._base_path: Optional[Path] = None

    @property
    def base_path(self) -> Path:
        if self._base_path is None:
            raise RuntimeError("Storage path has not been initialized")
        return self._base_path

    def ensure(self, parent: Optional[QtWidgets.QWidget] = None) -> Path:
        """Ensure the storage path is configured."""

        saved = self._load_saved_path()
        if saved:
            self._base_path = saved
            return saved

        dialog = StorageSetupDialog(default_path=DEFAULT_STORAGE, parent=parent)
        if dialog.exec() == QtWidgets.QDialog.DialogCode.Accepted:
            chosen = dialog.selected_path
            self._base_path = chosen
            self._persist_path(chosen)
            return chosen
        raise RuntimeError("Storage directory is required to run the application")

    def _load_saved_path(self) -> Optional[Path]:
        candidates = []
        # Pointer file lets us find storage even if user selected custom path
        if POINTER_FILE.exists():
            candidates.append(POINTER_FILE)

        # Default config path if user never moved things around
        default_config = DEFAULT_STORAGE / "settings" / CONFIG_NAME
        if default_config.exists():
            candidates.append(default_config)

        for candidate in candidates:
            try:
                with candidate.open("r", encoding="utf-8") as handle:
                    data = json.load(handle)
            except (OSError, json.JSONDecodeError):
                continue
            storage_str = data.get("storage_path")
            if not storage_str:
                continue
            path = Path(storage_str).expanduser()
            if path.exists():
                return path
        return None

    def _persist_path(self, path: Path) -> None:
        settings_dir = path / "settings"
        settings_dir.mkdir(parents=True, exist_ok=True)
        config_path = settings_dir / CONFIG_NAME
        payload = {"storage_path": str(path)}
        with config_path.open("w", encoding="utf-8") as handle:
            json.dump(payload, handle, indent=2)

        POINTER_FILE.parent.mkdir(parents=True, exist_ok=True)
        with POINTER_FILE.open("w", encoding="utf-8") as handle:
            json.dump(payload, handle, indent=2)

    def uninstall_everything(self) -> None:
        if self._base_path is None:
            self._base_path = self._load_saved_path()
        if self._base_path:
            shutil.rmtree(self._base_path, ignore_errors=True)
        if POINTER_FILE.exists():
            try:
                POINTER_FILE.unlink()
            except FileNotFoundError:
                pass


class StorageSetupDialog(QtWidgets.QDialog):
    """Installs the storage directory for first time setup."""

    def __init__(self, default_path: Path, parent: Optional[QtWidgets.QWidget] = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Choose Storage Location")
        self.selected_path = default_path

        self._path_edit = QtWidgets.QLineEdit(str(default_path))
        self._browse_btn = QtWidgets.QPushButton("Browse…")
        self._browse_btn.clicked.connect(self._pick_directory)

        self._error_label = QtWidgets.QLabel()
        self._error_label.setStyleSheet("color: #b00020;")

        buttons = QtWidgets.QDialogButtonBox(
            QtWidgets.QDialogButtonBox.StandardButton.Ok | QtWidgets.QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self._handle_accept)
        buttons.rejected.connect(self.reject)

        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(QtWidgets.QLabel("Select a writable directory for Topic Walker data."))
        path_layout = QtWidgets.QHBoxLayout()
        path_layout.addWidget(self._path_edit)
        path_layout.addWidget(self._browse_btn)
        layout.addLayout(path_layout)
        layout.addWidget(self._error_label)
        layout.addWidget(buttons)
        self.resize(520, 140)

    def _pick_directory(self) -> None:
        directory = QtWidgets.QFileDialog.getExistingDirectory(self, "Select Directory", self._path_edit.text())
        if directory:
            self._path_edit.setText(directory)

    def _handle_accept(self) -> None:
        candidate = Path(self._path_edit.text()).expanduser()
        if not candidate.is_absolute():
            self._error_label.setText("Please provide an absolute path.")
            return
        try:
            (candidate / "database").mkdir(parents=True, exist_ok=True)
            (candidate / "settings").mkdir(parents=True, exist_ok=True)
        except PermissionError:
            self._show_error_dialog(
                "Topic Walker cannot write to this location due to permissions."
            )
            return
        except OSError as exc:  # unexpected issues
            self._error_label.setText(str(exc))
            return

        self.selected_path = candidate
        self.accept()

    def _show_error_dialog(self, message: str) -> None:
        dialog = TimedMessageDialog(message, parent=self)
        dialog.exec()


class TimedMessageDialog(QtWidgets.QDialog):
    """Displays an error message that times out automatically."""

    def __init__(self, message: str, parent: Optional[QtWidgets.QWidget] = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Permission Error")
        self._seconds_remaining = 60

        self._message_label = QtWidgets.QLabel(message)
        self._countdown_label = QtWidgets.QLabel(self._format_countdown())

        close_button = QtWidgets.QPushButton("Close")
        close_button.clicked.connect(self.close)

        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(self._message_label)
        layout.addWidget(self._countdown_label)
        layout.addWidget(close_button)
        self.resize(420, 150)

        self._timer = QtCore.QTimer(self)
        self._timer.timeout.connect(self._tick)
        self._timer.start(1000)

    def _format_countdown(self) -> str:
        return f"Closing in {self._seconds_remaining} seconds"

    def _tick(self) -> None:
        self._seconds_remaining -= 1
        if self._seconds_remaining <= 0:
            self._timer.stop()
            self.close()
            return
        self._countdown_label.setText(self._format_countdown())


def remove_application_artifacts(app_paths: StoragePaths) -> None:
    """Helper that removes the application data and quits."""

    app_paths.uninstall_everything()
    # Remove potential platform-specific locations if present
    mac_app = Path("/Applications/cppTopicWalker.app")
    if mac_app.exists() and mac_app.is_dir():
        try:
            shutil.rmtree(mac_app)
        except PermissionError:
            pass
