#!/bin/sh
# PC-300 MIDI.app を作って ~/Applications に置く。libusb は静的リンクなので Homebrew を上げても壊れない。
# Builds "PC-300 MIDI.app" and installs it to ~/Applications. libusb is linked statically.
#
# 署名 / Signing:
#   SIGN_IDENTITY を指定すればその証明書で署名する。無ければアドホック署名（-）。
#   Set SIGN_IDENTITY to sign with your own certificate; otherwise ad-hoc ("-") is used.
set -e
cd "$(dirname "$0")"
LIBUSB="$(brew --prefix libusb)"
APP="build/PC-300 MIDI.app"
rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS"
cp Info.plist "$APP/Contents/Info.plist"
clang -O2 -Wall -o "$APP/Contents/MacOS/pc300midi" src/pc300midi.c \
  -I"$LIBUSB/include/libusb-1.0" "$LIBUSB/lib/libusb-1.0.a" \
  -framework CoreMIDI -framework CoreFoundation -framework IOKit -framework Security
codesign --force --sign "${SIGN_IDENTITY:--}" "$APP"
mkdir -p "$HOME/Applications"
rm -rf "$HOME/Applications/PC-300 MIDI.app"
cp -R "$APP" "$HOME/Applications/"
echo "installed: $HOME/Applications/PC-300 MIDI.app"
