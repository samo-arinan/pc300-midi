// PC-300 の USB ディスクリプタを表示する調査用
#include <stdio.h>
#include <libusb.h>

int main(void) {
    libusb_context *ctx;
    if (libusb_init(&ctx)) return 1;
    libusb_device_handle *h = libusb_open_device_with_vid_pid(ctx, 0x0582, 0x0008);
    if (!h) { printf("not found / cannot open\n"); return 1; }
    libusb_device *d = libusb_get_device(h);
    struct libusb_config_descriptor *cfg;
    int cur = -1;
    libusb_get_configuration(h, &cur);
    printf("current config %d\n", cur);
    if (libusb_get_config_descriptor(d, 0, &cfg)) { printf("no config\n"); return 1; }
    printf("config %d, %d interfaces\n", cfg->bConfigurationValue, cfg->bNumInterfaces);
    for (int i = 0; i < cfg->bNumInterfaces; i++) {
        const struct libusb_interface *it = &cfg->interface[i];
        for (int a = 0; a < it->num_altsetting; a++) {
            const struct libusb_interface_descriptor *id = &it->altsetting[a];
            printf(" if %d alt %d class %02x/%02x/%02x eps %d extra %d\n", id->bInterfaceNumber,
                   id->bAlternateSetting, id->bInterfaceClass, id->bInterfaceSubClass,
                   id->bInterfaceProtocol, id->bNumEndpoints, id->extra_length);
            for (int e = 0; e < id->bNumEndpoints; e++) {
                const struct libusb_endpoint_descriptor *ep = &id->endpoint[e];
                printf("   ep %02x attr %02x max %d interval %d\n", ep->bEndpointAddress,
                       ep->bmAttributes, ep->wMaxPacketSize, ep->bInterval);
            }
        }
    }
    unsigned char s[256];
    struct libusb_device_descriptor dd;
    libusb_get_device_descriptor(d, &dd);
    if (dd.iProduct && libusb_get_string_descriptor_ascii(h, dd.iProduct, s, sizeof s) > 0) printf("product: %s\n", s);
    libusb_free_config_descriptor(cfg);
    libusb_close(h);
    libusb_exit(ctx);
    return 0;
}
