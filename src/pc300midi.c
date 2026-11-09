// PC-300 MIDI: Roland PC-300 (USB 0582:0008) を macOS の MIDI 入力「PC-300」として見せる常駐アプリ。
//
// PC-300 はベンダー独自クラスの USB 機器で、今の macOS 用の Roland ドライバは無い。
// 中身は USB-MIDI と同じ 4 バイトのイベントパケットを interface 2 / endpoint 0x81 から送ってくる
// （Linux の snd-usb-audio の quirk と同じ）。それを libusb で読んで CoreMIDI の仮想ソースに流す。
// 抜き差しに備えて、見つからない間・読めなくなった時は数秒ごとに開き直す。

#include <CoreMIDI/CoreMIDI.h>
#include <libusb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define VID 0x0582
#define PID 0x0008
#define IFACE 2
#define EP_IN 0x81

static MIDIEndpointRef g_src;

static void logf_(const char *msg, const char *detail) {
    time_t t = time(NULL);
    char ts[32];
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&t));
    fprintf(stderr, "%s %s%s%s\n", ts, msg, detail ? ": " : "", detail ? detail : "");
}

// USB-MIDI のコードインデックス番号 (CIN) から、続く MIDI バイト数を返す。0 は捨てる。
static int cin_len(int cin) {
    switch (cin) {
    case 0x5: case 0xF: return 1;
    case 0x2: case 0x6: case 0xC: case 0xD: return 2;
    case 0x3: case 0x4: case 0x7: case 0x8: case 0x9: case 0xA: case 0xB: case 0xE: return 3;
    default: return 0;
    }
}

static void forward(const unsigned char *buf, int n) {
    Byte pl_buf[1024];
    MIDIPacketList *pl = (MIDIPacketList *)pl_buf;
    MIDIPacket *p = MIDIPacketListInit(pl);
    for (int i = 0; i + 4 <= n; i += 4) {
        int len = cin_len(buf[i] & 0x0F);
        if (!len) continue;
        p = MIDIPacketListAdd(pl, sizeof pl_buf, p, 0, len, &buf[i + 1]);
        if (!p) break;
    }
    if (pl->numPackets) MIDIReceived(g_src, pl);
}

static void run_device(libusb_context *ctx) {
    libusb_device_handle *h = libusb_open_device_with_vid_pid(ctx, VID, PID);
    if (!h) return;
    int cfg = 0;
    libusb_get_configuration(h, &cfg);
    if (cfg != 1) libusb_set_configuration(h, 1);
    int r = libusb_claim_interface(h, IFACE);
    if (r) {
        logf_("claim failed", libusb_error_name(r));
        libusb_close(h);
        return;
    }
    logf_("PC-300 connected", NULL);
    unsigned char buf[64];
    for (;;) {
        int n = 0;
        r = libusb_bulk_transfer(h, EP_IN, buf, sizeof buf, &n, 1000);
        if (r == LIBUSB_ERROR_TIMEOUT) continue;
        if (r) {
            logf_("read stopped", libusb_error_name(r));
            break;
        }
        forward(buf, n);
    }
    libusb_release_interface(h, IFACE);
    libusb_close(h);
    logf_("PC-300 disconnected", NULL);
}

int main(void) {
    MIDIClientRef client;
    if (MIDIClientCreate(CFSTR("PC-300 MIDI"), NULL, NULL, &client)) {
        logf_("MIDIClientCreate failed", NULL);
        return 1;
    }
    if (MIDISourceCreate(client, CFSTR("PC-300"), &g_src)) {
        logf_("MIDISourceCreate failed", NULL);
        return 1;
    }
    // 再起動しても Cubase などから同じ入力として見えるよう、固定の ID を付ける
    MIDIObjectSetIntegerProperty(g_src, kMIDIPropertyUniqueID, 0x50433330);  // "PC30"
    MIDIObjectSetStringProperty(g_src, kMIDIPropertyManufacturer, CFSTR("Roland"));

    libusb_context *ctx;
    if (libusb_init(&ctx)) {
        logf_("libusb_init failed", NULL);
        return 1;
    }
    logf_("started", NULL);
    for (;;) {
        run_device(ctx);
        sleep(2);
    }
}
