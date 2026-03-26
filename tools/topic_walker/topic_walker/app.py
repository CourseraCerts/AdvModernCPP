"""Main UI for the Topic Walker application."""
from __future__ import annotations

import sys
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path
from typing import Iterable, List, Sequence

from PyQt6 import QtCore, QtGui, QtWidgets

from .database import PromptRecord, TagRecord, TopicDatabase
from .installer import StoragePaths, remove_application_artifacts


class ViewMode(Enum):
    BY_PROMPT = auto()
    BY_TAG = auto()

    def label(self) -> str:
        return "by prompt" if self is ViewMode.BY_PROMPT else "by tag"


@dataclass
class ViewContext:
    mode: ViewMode
    filter_value: str | None


class NavigationTrail:
    """Tracks navigation history for drill-in interactions."""

    def __init__(self) -> None:
        self._stack: List[ViewContext] = []

    def push(self, mode: ViewMode, filter_value: str | None) -> None:
        self._stack.append(ViewContext(mode, filter_value))

    def pop(self) -> ViewContext | None:
        if not self._stack:
            return None
        return self._stack.pop()

    def clear(self) -> None:
        self._stack.clear()

    def has_history(self) -> bool:
        return bool(self._stack)


class FlowLayout(QtWidgets.QLayout):
    """A simple flowing layout borrowed from Qt docs."""

    def __init__(self, parent: QtWidgets.QWidget | None = None, margin: int = 0, spacing: int = -1) -> None:
        super().__init__(parent)
        self.setContentsMargins(margin, margin, margin, margin)
        self._item_list: List[QtWidgets.QLayoutItem] = []
        self.setSpacing(spacing)

    def __del__(self) -> None:
        item = self.takeAt(0)
        while item:
            item = self.takeAt(0)

    def addItem(self, item: QtWidgets.QLayoutItem) -> None:  # type: ignore[override]
        self._item_list.append(item)

    def count(self) -> int:  # type: ignore[override]
        return len(self._item_list)

    def itemAt(self, index: int) -> QtWidgets.QLayoutItem | None:  # type: ignore[override]
        if 0 <= index < len(self._item_list):
            return self._item_list[index]
        return None

    def takeAt(self, index: int) -> QtWidgets.QLayoutItem | None:  # type: ignore[override]
        if 0 <= index < len(self._item_list):
            return self._item_list.pop(index)
        return None

    def expandingDirections(self) -> QtCore.Qt.Orientations:  # type: ignore[override]
        return QtCore.Qt.Orientations(QtCore.Qt.Orientation(0))

    def hasHeightForWidth(self) -> bool:  # type: ignore[override]
        return True

    def heightForWidth(self, width: int) -> int:  # type: ignore[override]
        height = self._do_layout(QtCore.QRect(0, 0, width, 0), True)
        return height

    def setGeometry(self, rect: QtCore.QRect) -> None:  # type: ignore[override]
        super().setGeometry(rect)
        self._do_layout(rect, False)

    def sizeHint(self) -> QtCore.QSize:  # type: ignore[override]
        return self.minimumSize()

    def minimumSize(self) -> QtCore.QSize:  # type: ignore[override]
        size = QtCore.QSize()
        for item in self._item_list:
            size = size.expandedTo(item.minimumSize())
        margin, _, _, _ = self.getContentsMargins()
        size += QtCore.QSize(2 * margin, 2 * margin)
        return size

    def _do_layout(self, rect: QtCore.QRect, test_only: bool) -> int:
        x = rect.x()
        y = rect.y()
        line_height = 0

        for item in self._item_list:
            next_x = x + item.sizeHint().width() + self.spacing()
            if next_x - self.spacing() > rect.right() and line_height > 0:
                x = rect.x()
                y = y + line_height + self.spacing()
                next_x = x + item.sizeHint().width() + self.spacing()
                line_height = 0
            if not test_only:
                item.setGeometry(QtCore.QRect(QtCore.QPoint(x, y), item.sizeHint()))
            x = next_x
            line_height = max(line_height, item.sizeHint().height())
        return y + line_height - rect.y()


class PillListWidget(QtWidgets.QWidget):
    """Renders clickable rounded labels in a flow layout."""

    item_clicked = QtCore.pyqtSignal(str)

    def __init__(self, placeholder: str, parent: QtWidgets.QWidget | None = None) -> None:
        super().__init__(parent)
        self._placeholder = placeholder
        self._layout = FlowLayout(self, spacing=6)
        self.setSizePolicy(QtWidgets.QSizePolicy.Policy.Expanding, QtWidgets.QSizePolicy.Policy.Preferred)

    def set_items(self, items: Sequence[str]) -> None:
        while self._layout.count():
            item = self._layout.takeAt(0)
            if item:
                widget = item.widget()
                if widget:
                    widget.deleteLater()
        if not items:
            placeholder_label = QtWidgets.QLabel(self._placeholder)
            placeholder_label.setEnabled(False)
            self._layout.addWidget(placeholder_label)
            return
        for text in items:
            chip = QtWidgets.QPushButton(text)
            chip.setCursor(QtGui.QCursor(QtCore.Qt.CursorShape.PointingHandCursor))
            chip.setStyleSheet(
                """
                QPushButton {
                    border: 1px solid palette(mid);
                    border-radius: 12px;
                    padding: 2px 8px;
                    background: palette(alternate-base);
                }
                QPushButton:hover {
                    background: palette(light);
                }
                """
            )
            chip.clicked.connect(lambda _, value=text: self.item_clicked.emit(value))
            chip.setFocusPolicy(QtCore.Qt.FocusPolicy.NoFocus)
            self._layout.addWidget(chip)


class TagEditorDialog(QtWidgets.QDialog):
    """Allows editing tags for a prompt."""

    def __init__(self, prompt: str, tags: Sequence[str], parent: QtWidgets.QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Edit Tags")
        self._tags_edit = QtWidgets.QLineEdit(", ".join(tags))
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(QtWidgets.QLabel(f"Prompt:\n{prompt}"))
        layout.addWidget(QtWidgets.QLabel("Tags (comma separated, first tag is the source):"))
        layout.addWidget(self._tags_edit)
        buttons = QtWidgets.QDialogButtonBox(
            QtWidgets.QDialogButtonBox.StandardButton.Ok | QtWidgets.QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def tags(self) -> List[str]:
        return [tag.strip() for tag in self._tags_edit.text().split(",") if tag.strip()]


class SettingsDialog(QtWidgets.QDialog):
    """Displays storage settings and supports uninstallation."""

    def __init__(self, storage_path: Path, app_paths: StoragePaths, parent: QtWidgets.QWidget | None = None) -> None:
        super().__init__(parent)
        self._storage_path = storage_path
        self._paths = app_paths
        self.setWindowTitle("Topic Walker Settings")

        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(QtWidgets.QLabel("Storage directory:"))
        path_edit = QtWidgets.QLineEdit(str(storage_path))
        path_edit.setReadOnly(True)
        layout.addWidget(path_edit)

        uninstall_btn = QtWidgets.QPushButton("Uninstall Topic Walker")
        uninstall_btn.setStyleSheet("background-color: #d32f2f; color: white;")
        uninstall_btn.clicked.connect(self._confirm_uninstall)
        layout.addWidget(uninstall_btn)

        close_btn = QtWidgets.QPushButton("Close")
        close_btn.clicked.connect(self.accept)
        layout.addWidget(close_btn)

    def _confirm_uninstall(self) -> None:
        reply = QtWidgets.QMessageBox.question(
            self,
            "Uninstall",
            "This removes the application data, database, and settings. Continue?",
        )
        if reply == QtWidgets.QMessageBox.StandardButton.Yes:
            remove_application_artifacts(self._paths)
            QtWidgets.QMessageBox.information(self, "Uninstalled", "Application data removed successfully.")
            QtWidgets.QApplication.instance().quit()


class MainWindow(QtWidgets.QMainWindow):
    """Primary application window."""

    def __init__(self, db: TopicDatabase, app_paths: StoragePaths) -> None:
        super().__init__()
        self._db = db
        self._paths = app_paths
        self._view_mode = ViewMode.BY_PROMPT
        self._filter_value: str | None = None
        self._navigation = NavigationTrail()
        self._zoom_factor = 1.0
        self._base_font_size = self.font().pointSizeF() or 12.0
        self._new_prompt_tags: List[str] = []

        self.setWindowTitle("C++ Topic Walker")
        self.resize(1080, 720)

        central = QtWidgets.QWidget()
        self.setCentralWidget(central)
        outer_layout = QtWidgets.QVBoxLayout(central)

        outer_layout.addLayout(self._build_entry_panel())
        outer_layout.addWidget(self._build_mode_selector())
        outer_layout.addLayout(self._build_filter_panel())

        self._table = QtWidgets.QTableWidget()
        self._table.setEditTriggers(QtWidgets.QAbstractItemView.EditTrigger.NoEditTriggers)
        self._table.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectionBehavior.SelectRows)
        self._table.setSelectionMode(QtWidgets.QAbstractItemView.SelectionMode.ExtendedSelection)
        self._table.setAlternatingRowColors(True)
        self._table.setDragEnabled(True)
        self._table.verticalHeader().setVisible(False)
        outer_layout.addWidget(self._table)

        self._status_label = QtWidgets.QLabel()
        outer_layout.addWidget(self._status_label)

        self._setup_zoom_shortcuts()
        self._refresh_view()

    # UI builders ------------------------------------------------------

    def _build_entry_panel(self) -> QtWidgets.QLayout:
        prompt_label = QtWidgets.QLabel("Prompt")
        self._prompt_edit = QtWidgets.QPlainTextEdit()
        self._prompt_edit.setPlaceholderText("Describe the topic or question…")
        self._prompt_edit.setFixedHeight(80)

        tags_label = QtWidgets.QLabel("Tags (first tag is the source)")
        self._tag_input = QtWidgets.QLineEdit()
        self._tag_input.setPlaceholderText("Add a tag and press Enter")
        self._tag_input.returnPressed.connect(self._handle_add_tag)

        add_tag_btn = QtWidgets.QPushButton("Add Tag")
        add_tag_btn.clicked.connect(self._handle_add_tag)

        self._tag_list_widget = QtWidgets.QWidget()
        self._tag_list_layout = QtWidgets.QVBoxLayout(self._tag_list_widget)
        self._tag_list_layout.setContentsMargins(0, 0, 0, 0)
        self._tag_list_layout.setSpacing(4)
        self._refresh_new_prompt_tags()

        add_prompt_btn = QtWidgets.QPushButton("Add Prompt")
        add_prompt_btn.clicked.connect(self._handle_add_prompt)

        container = QtWidgets.QVBoxLayout()
        container.addWidget(prompt_label)
        container.addWidget(self._prompt_edit)
        container.addWidget(tags_label)
        tag_entry_row = QtWidgets.QHBoxLayout()
        tag_entry_row.addWidget(self._tag_input)
        tag_entry_row.addWidget(add_tag_btn)
        container.addLayout(tag_entry_row)
        container.addWidget(self._tag_list_widget)

        submit_row = QtWidgets.QHBoxLayout()
        submit_row.addStretch(1)
        submit_row.addWidget(add_prompt_btn)
        container.addLayout(submit_row)
        return container

    def _build_mode_selector(self) -> QtWidgets.QWidget:
        widget = QtWidgets.QWidget()
        layout = QtWidgets.QHBoxLayout(widget)
        layout.setContentsMargins(0, 0, 0, 0)

        self._mode_group = QtWidgets.QButtonGroup(self)
        prompt_btn = QtWidgets.QToolButton()
        prompt_btn.setText("By prompt")
        prompt_btn.setCheckable(True)
        prompt_btn.setChecked(True)
        tag_btn = QtWidgets.QToolButton()
        tag_btn.setText("By tag")
        tag_btn.setCheckable(True)

        self._mode_group.addButton(prompt_btn, int(ViewMode.BY_PROMPT.value))
        self._mode_group.addButton(tag_btn, int(ViewMode.BY_TAG.value))
        self._mode_group.buttonToggled.connect(self._handle_mode_change)

        layout.addWidget(prompt_btn)
        layout.addWidget(tag_btn)
        layout.addStretch(1)

        settings_btn = QtWidgets.QPushButton("Settings")
        settings_btn.clicked.connect(self._open_settings)
        layout.addWidget(settings_btn)

        return widget

    def _build_filter_panel(self) -> QtWidgets.QLayout:
        layout = QtWidgets.QHBoxLayout()
        self._filter_label = QtWidgets.QLabel()
        layout.addWidget(self._filter_label)

        self._back_btn = QtWidgets.QPushButton("Back")
        self._back_btn.clicked.connect(self._navigate_back)
        self._back_btn.setEnabled(False)
        layout.addWidget(self._back_btn)

        self._clear_filter_btn = QtWidgets.QPushButton("Clear filter")
        self._clear_filter_btn.clicked.connect(self._clear_filter)
        self._clear_filter_btn.setEnabled(False)
        layout.addWidget(self._clear_filter_btn)
        layout.addStretch(1)
        return layout

    # Event handlers ---------------------------------------------------

    def _handle_mode_change(self, button: QtWidgets.QAbstractButton, checked: bool) -> None:
        if not checked:
            return
        new_mode = ViewMode.BY_PROMPT if "prompt" in button.text().lower() else ViewMode.BY_TAG
        if new_mode == self._view_mode:
            return
        self._view_mode = new_mode
        self._filter_value = None
        self._navigation.clear()
        self._refresh_view()

    def _handle_add_prompt(self) -> None:
        prompt = self._prompt_edit.toPlainText().strip()
        tags = list(self._new_prompt_tags)
        if not tags:
            QtWidgets.QMessageBox.warning(self, "Validation", "Please add at least one tag before saving.")
            return
        try:
            self._db.add_prompt(prompt, tags)
        except ValueError as exc:
            QtWidgets.QMessageBox.warning(self, "Invalid data", str(exc))
            return
        self._prompt_edit.clear()
        self._tag_input.clear()
        self._new_prompt_tags.clear()
        self._refresh_new_prompt_tags()
        self._status_label.setText("Prompt saved.")
        self._refresh_view()

    def _handle_add_tag(self) -> None:
        tag = self._tag_input.text().strip()
        if not tag:
            self._status_label.setText("Enter a tag before adding.")
            return
        lowered = tag.lower()
        if any(existing.lower() == lowered for existing in self._new_prompt_tags):
            self._status_label.setText("Tag already added.")
            self._tag_input.selectAll()
            return
        self._new_prompt_tags.append(tag)
        self._tag_input.clear()
        self._refresh_new_prompt_tags()
        self._status_label.setText("Tag added.")

    def _edit_new_tag(self, index: int) -> None:
        current_tag = self._new_prompt_tags[index]
        new_tag, accepted = QtWidgets.QInputDialog.getText(self, "Edit Tag", "Tag:", text=current_tag)
        if not accepted:
            return
        new_tag = new_tag.strip()
        if not new_tag:
            QtWidgets.QMessageBox.warning(self, "Validation", "Tag cannot be empty.")
            return
        lowered = new_tag.lower()
        if any(existing.lower() == lowered for idx, existing in enumerate(self._new_prompt_tags) if idx != index):
            QtWidgets.QMessageBox.warning(self, "Validation", "Tag already exists.")
            return
        self._new_prompt_tags[index] = new_tag
        self._refresh_new_prompt_tags()
        self._status_label.setText("Tag updated.")

    def _delete_new_tag(self, index: int) -> None:
        del self._new_prompt_tags[index]
        self._refresh_new_prompt_tags()
        self._status_label.setText("Tag removed.")

    def _refresh_new_prompt_tags(self) -> None:
        while self._tag_list_layout.count():
            item = self._tag_list_layout.takeAt(0)
            widget = item.widget()
            if widget:
                widget.deleteLater()
        if not self._new_prompt_tags:
            placeholder = QtWidgets.QLabel("No tags added yet.")
            placeholder.setEnabled(False)
            self._tag_list_layout.addWidget(placeholder)
            return

        for index, tag in enumerate(self._new_prompt_tags):
            item_widget = QtWidgets.QWidget()
            item_layout = QtWidgets.QHBoxLayout(item_widget)
            item_layout.setContentsMargins(0, 0, 0, 0)
            item_layout.addWidget(QtWidgets.QLabel(tag))
            item_layout.addStretch(1)

            edit_btn = QtWidgets.QPushButton("Edit")
            edit_btn.clicked.connect(lambda _, idx=index: self._edit_new_tag(idx))
            delete_btn = QtWidgets.QPushButton("Delete")
            delete_btn.clicked.connect(lambda _, idx=index: self._delete_new_tag(idx))

            button_row = QtWidgets.QHBoxLayout()
            button_row.setContentsMargins(0, 0, 0, 0)
            button_row.setSpacing(4)
            button_row.addWidget(edit_btn)
            button_row.addWidget(delete_btn)

            item_layout.addLayout(button_row)
            self._tag_list_layout.addWidget(item_widget)

    def _navigate_to_tag(self, tag: str) -> None:
        self._navigation.push(self._view_mode, self._filter_value)
        self._view_mode = ViewMode.BY_TAG
        self._filter_value = tag
        self._mode_group.button(int(ViewMode.BY_TAG.value)).setChecked(True)
        self._refresh_view()

    def _navigate_to_prompt(self, prompt: str) -> None:
        self._navigation.push(self._view_mode, self._filter_value)
        self._view_mode = ViewMode.BY_PROMPT
        self._filter_value = prompt
        self._mode_group.button(int(ViewMode.BY_PROMPT.value)).setChecked(True)
        self._refresh_view()

    def _navigate_back(self) -> None:
        context = self._navigation.pop()
        if context:
            self._view_mode = context.mode
            self._filter_value = context.filter_value
            self._mode_group.button(int(self._view_mode.value)).setChecked(True)
            self._refresh_view()

    def _clear_filter(self) -> None:
        self._filter_value = None
        self._navigation.clear()
        self._refresh_view()

    def _open_settings(self) -> None:
        dialog = SettingsDialog(self._paths.base_path, self._paths, self)
        dialog.exec()

    # View rendering ---------------------------------------------------

    def _refresh_view(self) -> None:
        if self._view_mode == ViewMode.BY_PROMPT:
            self._render_prompt_view()
        else:
            self._render_tag_view()
        self._back_btn.setEnabled(self._navigation.has_history())
        self._clear_filter_btn.setEnabled(self._filter_value is not None)
        if self._filter_value:
            self._filter_label.setText(f"Showing {self._view_mode.label()} filtered by '{self._filter_value}'")
        else:
            self._filter_label.setText(f"Showing {self._view_mode.label()} view")

    def _render_prompt_view(self) -> None:
        records = self._db.get_prompts(filter_prompt=self._filter_value)
        self._table.clear()
        self._table.setColumnCount(3)
        self._table.setHorizontalHeaderLabels(["Prompt", "Tags", "Manage"])
        self._table.setRowCount(len(records))

        for row_idx, record in enumerate(records):
            prompt_item = QtWidgets.QTableWidgetItem(record.prompt)
            prompt_item.setData(QtCore.Qt.ItemDataRole.UserRole, record.prompt_id)
            self._table.setItem(row_idx, 0, prompt_item)

            tags_widget = PillListWidget("No tags")
            tags_widget.set_items(record.tags)
            tags_widget.item_clicked.connect(self._navigate_to_tag)
            self._table.setCellWidget(row_idx, 1, tags_widget)

            edit_btn = QtWidgets.QPushButton("Edit tags")
            edit_btn.clicked.connect(lambda _, pr=record: self._edit_tags(pr))
            self._table.setCellWidget(row_idx, 2, edit_btn)

        self._table.resizeColumnsToContents()
        self._status_label.setText(f"{len(records)} prompts loaded.")

    def _render_tag_view(self) -> None:
        records = self._db.get_tags(filter_tag=self._filter_value)
        self._table.clear()
        self._table.setColumnCount(2)
        self._table.setHorizontalHeaderLabels(["Tag", "Prompts"])
        self._table.setRowCount(len(records))

        for row_idx, record in enumerate(records):
            tag_item = QtWidgets.QTableWidgetItem(record.tag)
            tag_item.setData(QtCore.Qt.ItemDataRole.UserRole, record.tag_id)
            self._table.setItem(row_idx, 0, tag_item)

            prompts_widget = PillListWidget("No prompts")
            prompts_widget.set_items(record.prompts)
            prompts_widget.item_clicked.connect(self._navigate_to_prompt)
            self._table.setCellWidget(row_idx, 1, prompts_widget)

        self._table.resizeColumnsToContents()
        self._status_label.setText(f"{len(records)} tags loaded.")

    def _edit_tags(self, record: PromptRecord) -> None:
        dialog = TagEditorDialog(record.prompt, record.tags, self)
        if dialog.exec() == QtWidgets.QDialog.DialogCode.Accepted:
            tags = dialog.tags()
            if not tags:
                QtWidgets.QMessageBox.warning(self, "Validation", "Please provide at least one tag.")
                return
            self._db.update_prompt_tags(record.prompt_id, tags)
            self._status_label.setText("Tags updated.")
            self._refresh_view()

    # Zoom handling ----------------------------------------------------

    def _setup_zoom_shortcuts(self) -> None:
        zoom_in = QtGui.QShortcut(QtGui.QKeySequence.StandardKey.ZoomIn, self)
        zoom_in.activated.connect(lambda: self._update_zoom(0.1))
        zoom_out = QtGui.QShortcut(QtGui.QKeySequence.StandardKey.ZoomOut, self)
        zoom_out.activated.connect(lambda: self._update_zoom(-0.1))

    def _update_zoom(self, delta: float) -> None:
        self._zoom_factor = max(0.7, min(1.6, self._zoom_factor + delta))
        font = self.font()
        font.setPointSizeF(self._base_font_size * self._zoom_factor)
        self.setFont(font)


def run_app() -> None:
    app = QtWidgets.QApplication(sys.argv)
    app.setOrganizationName("TopicWalker")
    app.setApplicationName("Topic Walker")

    paths = StoragePaths()
    try:
        base_path = paths.ensure()
    except RuntimeError:
        return

    db_file = base_path / "database" / "topicwalker.db"
    db = TopicDatabase(db_file)

    window = MainWindow(db, paths)
    window.show()
    app.exec()
    db.close()


if __name__ == "__main__":
    run_app()
