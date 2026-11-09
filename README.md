# PC-300 MIDI

Roland PC-300 (USB `0582:0008`) を今の macOS で MIDI 入力として使うための小さな常駐アプリ。

A tiny background app that makes the Roland PC-300 MIDI keyboard (USB `0582:0008`) usable as a MIDI input on current macOS.

[日本語](#日本語) / [English](#english)

---

## 日本語

### これは何

PC-300 はベンダー独自クラスの USB 機器で、今の macOS で動く Roland のドライバはありません。
このアプリは PC-300 を libusb で直接読み、CoreMIDI の仮想ソース **「PC-300」** として見せます。
Cubase などの DAW からは普通の MIDI キーボードとして見えます。

- 確認した環境: Apple Silicon の Mac、Cubase 14 + HALion Sonic
- 鍵盤の入力のみ（PC-300 へ MIDI を送る向きは未対応）
- PC-300 は MIDI チャンネル 8 で送ってきますが、アプリがチャンネル 1 に書き換えて流します（1ch で受ける音源の単体アプリでもそのまま鳴るように）

### 仕組み

PC-300 は interface 2 / endpoint `0x81`（バルク IN）から、USB-MIDI クラスと同じ 4 バイトのイベントパケットを送ってきます。
アプリはそれを読み、コードインデックス番号（CIN）から MIDI バイト数を決めて、CoreMIDI の仮想ソースに流します。
抜き差しに備えて、見つからない間・読めなくなった時は 2 秒ごとに開き直します。

この読み方は Linux カーネルの `snd-usb-audio` にある PC-300 の quirk（`QUIRK_MIDI_FIXED_ENDPOINT`）を参考にしました。

### ビルドと導入

Homebrew の libusb が要ります（静的リンクするので、後で Homebrew を更新しても壊れません）。

```sh
brew install libusb
sh build.sh              # ~/Applications/PC-300 MIDI.app に入る
sh launchd/install.sh    # ログイン時に自動で起動する（任意）
```

署名は既定でアドホック署名です。自分の証明書で署名するなら `SIGN_IDENTITY="証明書名" sh build.sh`。

ログは `~/Library/Logs/pc300midi.log`（LaunchAgent から起動した場合）。PC-300 をつなぐと `PC-300 connected` と出ます。

止める・外す:

```sh
launchctl bootout gui/$(id -u)/com.j.pc300midi
rm ~/Library/LaunchAgents/com.j.pc300midi.plist
```

### 調査用ツール

`tools/` に、作る時に使った物を置いています。

- `probe.c` — USB ディスクリプタを表示
- `listen.c` — 届いた生パケットを表示（引数は秒数、既定 30）

```sh
clang -o probe tools/probe.c -I"$(brew --prefix libusb)/include/libusb-1.0" -L"$(brew --prefix libusb)/lib" -lusb-1.0
```

---

## English

### What it is

The PC-300 uses a vendor-specific USB class, and there is no Roland driver that works on current macOS.
This app reads the PC-300 directly through libusb and exposes it as a CoreMIDI virtual source named **"PC-300"**,
so DAWs such as Cubase see it as an ordinary MIDI keyboard.

- Tested on: Apple Silicon Mac, Cubase 14 + HALion Sonic
- Input only (sending MIDI to the PC-300 is not supported)
- The PC-300 transmits on MIDI channel 8. The app rewrites channel messages to channel 1, so standalone instruments that listen on channel 1 play without extra setup

### How it works

The PC-300 sends standard 4-byte USB-MIDI event packets on interface 2 / endpoint `0x81` (bulk IN).
The app reads them, uses the Code Index Number (CIN) to determine the MIDI message length, and forwards them to the CoreMIDI virtual source.
If the device is missing or a read fails, it retries every 2 seconds, so hot-plugging works.

This approach follows the PC-300 quirk (`QUIRK_MIDI_FIXED_ENDPOINT`) in the Linux kernel's `snd-usb-audio` driver.

### Build and install

Requires libusb from Homebrew (linked statically, so later Homebrew upgrades won't break the app).

```sh
brew install libusb
sh build.sh              # installs ~/Applications/PC-300 MIDI.app
sh launchd/install.sh    # start automatically at login (optional)
```

The app is ad-hoc signed by default. To sign with your own certificate: `SIGN_IDENTITY="Your Cert" sh build.sh`.

Log: `~/Library/Logs/pc300midi.log` (when started by the LaunchAgent). Plugging in the PC-300 logs `PC-300 connected`.

To stop and remove:

```sh
launchctl bootout gui/$(id -u)/com.j.pc300midi
rm ~/Library/LaunchAgents/com.j.pc300midi.plist
```

### Investigation tools

`tools/` contains the helpers used while building this:

- `probe.c` — prints the USB descriptors
- `listen.c` — prints raw incoming packets (argument: seconds, default 30)

```sh
clang -o probe tools/probe.c -I"$(brew --prefix libusb)/include/libusb-1.0" -L"$(brew --prefix libusb)/lib" -lusb-1.0
```

## License

MIT

Roland と PC-300 は Roland Corporation の商標です。このプロジェクトは Roland とは関係のない非公式のものです。

Roland and PC-300 are trademarks of Roland Corporation. This is an unofficial project, not affiliated with or endorsed by Roland.
