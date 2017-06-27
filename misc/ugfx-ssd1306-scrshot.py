# GDB command: dump binary memory /tmp/scr.bin (g->priv) (g->priv + 1024)
# put breakpoint at gdisp_lld_flush (inside gdisp_lld_SSD1306.c)

import sys
from PIL import Image

im = Image.new("RGB", (64, 128), "white")
buf = [ord(ch) for ch in open('/tmp/scr.bin').read()]



def xyaddr(x, y):
    return (x) + ((y)>>3)*128

def xybit(y):
    return (1<<((y)&7))


for y in range(128):
    for x in range(64):
        eff_x = y
        eff_y = 64-1 - x

        pix = bool(buf[xyaddr(eff_x, eff_y)] & xybit(eff_y))
        sys.stdout.write('O' if pix else ' ')

        if pix:
            im.putpixel((x, y), (255,0,0))

    sys.stdout.write('\n')

im.save('lol.png', 'PNG')
