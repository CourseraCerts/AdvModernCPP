"""SQLite database helpers for Topic Walker."""
from __future__ import annotations

import sqlite3
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List, Sequence


@dataclass
class PromptRecord:
    prompt_id: int
    prompt: str
    tags: List[str]


@dataclass
class TagRecord:
    tag_id: int
    tag: str
    prompts: List[str]


class TopicDatabase:
    """Wrangles all SQLite access for the application."""

    def __init__(self, db_path: Path) -> None:
        db_path.parent.mkdir(parents=True, exist_ok=True)
        self._conn = sqlite3.connect(db_path, check_same_thread=False)
        self._conn.row_factory = sqlite3.Row
        self._initialize()

    def close(self) -> None:
        self._conn.close()

    def _initialize(self) -> None:
        cursor = self._conn.cursor()
        cursor.executescript(
            """
            PRAGMA foreign_keys = ON;
            CREATE TABLE IF NOT EXISTS prompts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                text TEXT NOT NULL UNIQUE
            );
            CREATE TABLE IF NOT EXISTS tags (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL COLLATE NOCASE UNIQUE
            );
            CREATE TABLE IF NOT EXISTS prompt_tags (
                prompt_id INTEGER NOT NULL,
                tag_id INTEGER NOT NULL,
                is_source INTEGER NOT NULL DEFAULT 0,
                PRIMARY KEY (prompt_id, tag_id),
                FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE CASCADE,
                FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
            );
            CREATE INDEX IF NOT EXISTS idx_prompt_tags_prompt ON prompt_tags(prompt_id);
            CREATE INDEX IF NOT EXISTS idx_prompt_tags_tag ON prompt_tags(tag_id);
            """
        )
        self._conn.commit()

    def add_prompt(self, prompt: str, tags: Sequence[str]) -> None:
        prompt = prompt.strip()
        if not prompt:
            raise ValueError("Prompt text cannot be empty")
        tags = tuple(self._normalize_tags(tags))
        if not tags:
            raise ValueError("At least one tag (source) is required")
        with self._conn:
            prompt_id = self._ensure_prompt(prompt)
            self._clear_prompt_tags(prompt_id)
            for idx, tag in enumerate(tags):
                tag_id = self._ensure_tag(tag)
                self._conn.execute(
                    "INSERT OR REPLACE INTO prompt_tags(prompt_id, tag_id, is_source) VALUES (?, ?, ?)",
                    (prompt_id, tag_id, 1 if idx == 0 else 0),
                )

    def update_prompt_tags(self, prompt_id: int, tags: Sequence[str]) -> None:
        tags = tuple(self._normalize_tags(tags))
        with self._conn:
            self._clear_prompt_tags(prompt_id)
            for idx, tag in enumerate(tags):
                tag_id = self._ensure_tag(tag)
                self._conn.execute(
                    "INSERT INTO prompt_tags(prompt_id, tag_id, is_source) VALUES (?, ?, ?)",
                    (prompt_id, tag_id, 1 if idx == 0 else 0),
                )

    def get_prompts(self, *, filter_prompt: str | None = None) -> List[PromptRecord]:
        cursor = self._conn.cursor()
        params: List[str] = []
        prompt_clause = ""
        if filter_prompt:
            prompt_clause = "WHERE p.text = ?"
            params.append(filter_prompt)
        query = f"""
            SELECT p.id as prompt_id, p.text as prompt, t.name as tag, pt.is_source
            FROM prompts p
            LEFT JOIN prompt_tags pt ON p.id = pt.prompt_id
            LEFT JOIN tags t ON t.id = pt.tag_id
            {prompt_clause}
            ORDER BY lower(p.text), pt.is_source DESC, lower(t.name)
        """
        cursor.execute(query, params)
        rows = cursor.fetchall()
        grouped: dict[int, PromptRecord] = {}
        for row in rows:
            prompt_id = row["prompt_id"]
            prompt_text = row["prompt"]
            if prompt_id not in grouped:
                grouped[prompt_id] = PromptRecord(prompt_id, prompt_text, [])
            tag = row["tag"]
            if tag:
                grouped[prompt_id].tags.append(tag)
        return sorted(grouped.values(), key=lambda record: record.prompt.lower())

    def get_tags(self, *, filter_tag: str | None = None) -> List[TagRecord]:
        cursor = self._conn.cursor()
        params: List[str] = []
        tag_clause = ""
        if filter_tag:
            tag_clause = "WHERE t.name = ?"
            params.append(filter_tag)
        query = f"""
            SELECT t.id as tag_id, t.name as tag, p.text as prompt
            FROM tags t
            LEFT JOIN prompt_tags pt ON pt.tag_id = t.id
            LEFT JOIN prompts p ON pt.prompt_id = p.id
            {tag_clause}
            ORDER BY lower(t.name), lower(p.text)
        """
        cursor.execute(query, params)
        rows = cursor.fetchall()
        grouped: dict[int, TagRecord] = {}
        for row in rows:
            tag_id = row["tag_id"]
            tag_name = row["tag"]
            if tag_id not in grouped:
                grouped[tag_id] = TagRecord(tag_id, tag_name, [])
            prompt = row["prompt"]
            if prompt:
                grouped[tag_id].prompts.append(prompt)
        return sorted(grouped.values(), key=lambda record: record.tag.lower())

    def get_prompt_id(self, prompt_text: str) -> int | None:
        cursor = self._conn.cursor()
        cursor.execute("SELECT id FROM prompts WHERE text = ?", (prompt_text,))
        row = cursor.fetchone()
        return int(row[0]) if row else None

    # Internal helpers -------------------------------------------------

    def _normalize_tags(self, tags: Sequence[str]) -> Iterable[str]:
        seen = set()
        for tag in tags:
            tag = tag.strip()
            if not tag:
                continue
            lowered = tag.lower()
            if lowered in seen:
                continue
            seen.add(lowered)
            yield tag

    def _ensure_prompt(self, prompt: str) -> int:
        cursor = self._conn.cursor()
        cursor.execute("INSERT OR IGNORE INTO prompts(text) VALUES (?)", (prompt,))
        if cursor.rowcount == 0:
            cursor.execute("SELECT id FROM prompts WHERE text = ?", (prompt,))
            return int(cursor.fetchone()[0])
        return int(cursor.lastrowid)

    def _ensure_tag(self, tag: str) -> int:
        cursor = self._conn.cursor()
        cursor.execute("INSERT OR IGNORE INTO tags(name) VALUES (?)", (tag,))
        if cursor.rowcount == 0:
            cursor.execute("SELECT id FROM tags WHERE name = ?", (tag,))
            return int(cursor.fetchone()[0])
        return int(cursor.lastrowid)

    def _clear_prompt_tags(self, prompt_id: int) -> None:
        self._conn.execute("DELETE FROM prompt_tags WHERE prompt_id = ?", (prompt_id,))


__all__ = ["TopicDatabase", "PromptRecord", "TagRecord"]
