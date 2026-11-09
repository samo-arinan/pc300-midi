// PC-300 から届く USB パケットを秒数分だけ表示する調査用
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <libusb.h>

int main(int argc, char **argv) {
    int secs = argc > 1 ? atoi(argv[1]) : 30;
    libusb_context *ctx;
    if (libusb_init(&ctx)) return 1;
    libusb_device_handle *h = libusb_open_device_with_vid_pid(ctx, 0x0582, 0x0008);
    if (!h) { printf("open failed\n"); return 1; }
    int r = libusb_set_configuration(h, 1);
    printf("set_configuration: %s\n", libusb_error_name(r));
    r = libusb_claim_interface(h, 2);
    printf("claim: %s\n", libusb_error_name(r));
    if (r) return 1;
    fflush(stdout);
    time_t end = time(NULL) + secs;
    unsigned char buf[64];
    while (time(NULL) < end) {
        int n = 0;
        r = libusb_bulk_transfer(h, 0x81, buf, sizeof buf, &n, 500);
        if (r == LIBUSB_ERROR_TIMEOUT) continue;
        if (r) { printf("read error: %s\n", libusb_error_name(r)); break; }
        for (int i = 0; i < n; i++) printf("%02x ", buf[i]);
        printf("\n");
        fflush(stdout);
    }
    libusb_release_interface(h, 2);
    libusb_close(h);
    libusb_exit(ctx);
    return 0;
}
