#!/bin/sh
# ログイン時に PC-300 MIDI.app を起動する LaunchAgent を入れる（先に build.sh を実行しておく）。
# Installs a LaunchAgent that starts PC-300 MIDI.app at login (run build.sh first).
set -e
cd "$(dirname "$0")"
LABEL=com.j.pc300midi
DEST="$HOME/Library/LaunchAgents/$LABEL.plist"
mkdir -p "$HOME/Library/LaunchAgents" "$HOME/Library/Logs"
sed "s|__HOME__|$HOME|g" "$LABEL.plist" > "$DEST"
launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
launchctl bootstrap "gui/$(id -u)" "$DEST"
echo "installed: $DEST (log: $HOME/Library/Logs/pc300midi.log)"
