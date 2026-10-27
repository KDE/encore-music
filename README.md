<!--
SPDX-FileCopyrightText: None
SPDX-License-Identifier: GPL-2.0-or-later
-->

# Bigscreen Music

A music player for Plasma Bigscreen, built with [Bigscreen UI](https://invent.kde.org/devinlin/bigscreen-ui).

Features:
* Find music organized by artists, albums
* Custom playlists (TODO)
* Track search
* MPRIS2 integration
* UI for TVs and handhelds

## Links

* Project page: https://invent.kde.org/devinlin/bigscreen-music
* File issues: https://invent.kde.org/devinlin/bigscreen-music/-/work-items
* Development channel: https://matrix.to/#/#plasma-bigscreen:kde.org

## Building

```bash
cmake -B build
cmake --build build
./build/bin/bigscreen-music
```

## Stored data

* `~/.config/bigscreen-musicrc` - Settings
* `~/.local/share/bigscreen-music/library.sqlite` - Scanned library database
* `~/.cache/bigscreen-music/covers/` - Cover art cache
