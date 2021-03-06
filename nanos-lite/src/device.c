#include "common.h"
#include <amdev.h>
#ifndef KEYDOWN_MASK
#define KEYDOWN_MASK 0x8000
#endif
size_t serial_write(const void *buf, size_t offset, size_t len) {
    char *str = (char *)buf;
    size_t i = 0;
    for (i = 0; i < len; ++i)
        _putc(str[i]);
    return len;
  return 0;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
    int keycode = read_key();
    if (keycode != _KEY_NONE)
    {
        if (keycode & KEYDOWN_MASK)
        {
            keycode ^= KEYDOWN_MASK;
            sprintf(buf, "kd %s\n", keyname[keycode]);
        }
        else
        {
            sprintf(buf, "ku %s\n", keyname[keycode]);
        }
    }
    else
    {
        sprintf(buf, "t %u\n", uptime());
    }

    return strlen(buf);
}

static char dispinfo[128] __attribute__((used)) = {};
size_t dispinfo_len()
{
    return strlen(dispinfo);
}
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    strncpy(buf, dispinfo + offset, len);
    return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    offset /= 4;
    int x = offset % screen_width();
    int y = offset / screen_width();
    draw_rect((uint32_t *)buf, x, y, len / 4, 1);
    return len;
}

size_t fbsync_write(const void *buf, size_t offset, size_t len) {
    draw_sync();
    return 0;
}

void init_device() {

    Log("Initializing devices...");
    _ioe_init();

    // TODO: print the string to array `dispinfo` with the format
    // described in the Navy-apps convention
    sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
