#!/usr/bin/env python3
"""Build a visual gallery from rendered module PPM frames.

Pure stdlib (zlib only). Reads <frames_dir>/<module>_NNNN.ppm produced by
`ad-harness --render`, and writes to <out_dir>:
  - png/<module>_NN.png        one PNG per captured frame
  - montage.png                a captioned grid of one frame per module
  - index.html                 a flipbook gallery that animates each module

Usage: make_gallery.py <frames_dir> <out_dir>
"""
import glob
import os
import struct
import sys
import zlib

# --- 3x5 bitmap font (low 3 bits per row), matching sdk/afterdark/text.h -----
FONT = {
    ' ': [0,0,0,0,0], '-': [0,0,0b111,0,0], '!': [0b010,0b010,0b010,0,0b010],
    '.': [0,0,0,0,0b010], ':': [0,0b010,0,0b010,0],
    '0': [0b111,0b101,0b101,0b101,0b111], '1': [0b010,0b110,0b010,0b010,0b111],
    '2': [0b111,0b001,0b111,0b100,0b111], '3': [0b111,0b001,0b111,0b001,0b111],
    '4': [0b101,0b101,0b111,0b001,0b001], '5': [0b111,0b100,0b111,0b001,0b111],
    '6': [0b111,0b100,0b111,0b101,0b111], '7': [0b111,0b001,0b010,0b010,0b010],
    '8': [0b111,0b101,0b111,0b101,0b111], '9': [0b111,0b101,0b111,0b001,0b111],
    'A': [0b111,0b101,0b111,0b101,0b101], 'B': [0b110,0b101,0b110,0b101,0b110],
    'C': [0b111,0b100,0b100,0b100,0b111], 'D': [0b110,0b101,0b101,0b101,0b110],
    'E': [0b111,0b100,0b110,0b100,0b111], 'F': [0b111,0b100,0b110,0b100,0b100],
    'G': [0b111,0b100,0b101,0b101,0b111], 'H': [0b101,0b101,0b111,0b101,0b101],
    'I': [0b111,0b010,0b010,0b010,0b111], 'J': [0b001,0b001,0b001,0b101,0b111],
    'K': [0b101,0b101,0b110,0b101,0b101], 'L': [0b100,0b100,0b100,0b100,0b111],
    'M': [0b101,0b111,0b111,0b101,0b101], 'N': [0b101,0b111,0b111,0b111,0b101],
    'O': [0b111,0b101,0b101,0b101,0b111], 'P': [0b111,0b101,0b111,0b100,0b100],
    'Q': [0b111,0b101,0b101,0b111,0b011], 'R': [0b111,0b101,0b110,0b101,0b101],
    'S': [0b111,0b100,0b111,0b001,0b111], 'T': [0b111,0b010,0b010,0b010,0b010],
    'U': [0b101,0b101,0b101,0b101,0b111], 'V': [0b101,0b101,0b101,0b101,0b010],
    'W': [0b101,0b101,0b111,0b111,0b101], 'X': [0b101,0b101,0b010,0b101,0b101],
    'Y': [0b101,0b101,0b010,0b010,0b010], 'Z': [0b111,0b001,0b010,0b100,0b111],
}


class Img:
    def __init__(self, w, h, fill=(0, 0, 0)):
        self.w, self.h = w, h
        self.px = bytearray(fill * (w * h))

    def put(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            i = (y * self.w + x) * 3
            self.px[i:i + 3] = bytes(c)

    def rect(self, x, y, w, h, c):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.put(xx, yy, c)

    def text(self, x, y, s, scale, c):
        cx = x
        for ch in s.upper():
            g = FONT.get(ch)
            if g:
                for r in range(5):
                    for col in range(3):
                        if g[r] & (1 << (2 - col)):
                            self.rect(cx + col * scale, y + r * scale, scale, scale, c)
            cx += 4 * scale

    def blit(self, src, dx, dy):
        for y in range(src.h):
            for x in range(src.w):
                i = (y * src.w + x) * 3
                self.put(dx + x, dy + y, src.px[i:i + 3])


def read_ppm(path):
    with open(path, 'rb') as f:
        data = f.read()
    # Parse P6 header: magic, width, height, maxval, then binary.
    assert data[:2] == b'P6', path
    idx = 2
    vals = []
    while len(vals) < 3:
        while idx < len(data) and data[idx:idx + 1].isspace():
            idx += 1
        if data[idx:idx + 1] == b'#':
            while data[idx:idx + 1] not in (b'\n', b''):
                idx += 1
            continue
        start = idx
        while idx < len(data) and not data[idx:idx + 1].isspace():
            idx += 1
        vals.append(int(data[start:idx]))
    w, h, _maxv = vals
    idx += 1  # single whitespace after maxval
    img = Img(w, h)
    img.px = bytearray(data[idx:idx + w * h * 3])
    return img


def downscale(src, tw, th):
    out = Img(tw, th)
    for y in range(th):
        sy = y * src.h // th
        for x in range(tw):
            sx = x * src.w // tw
            i = (sy * src.w + sx) * 3
            j = (y * tw + x) * 3
            out.px[j:j + 3] = src.px[i:i + 3]
    return out


def write_png(path, img):
    def chunk(tag, data):
        return (struct.pack('>I', len(data)) + tag + data +
                struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff))
    raw = bytearray()
    for y in range(img.h):
        raw.append(0)  # filter type 0
        raw += img.px[y * img.w * 3:(y + 1) * img.w * 3]
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', img.w, img.h, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(bytes(raw), 9))
    png += chunk(b'IEND', b'')
    with open(path, 'wb') as f:
        f.write(png)


def main():
    frames_dir, out_dir = sys.argv[1], sys.argv[2]
    png_dir = os.path.join(out_dir, 'png')
    os.makedirs(png_dir, exist_ok=True)

    modules = {}
    for p in sorted(glob.glob(os.path.join(frames_dir, '*_*.ppm'))):
        base = os.path.basename(p)[:-4]
        mod, _, _ = base.rpartition('_')
        modules.setdefault(mod, []).append(p)
    if not modules:
        print('no frames found in', frames_dir)
        return 1

    # Per-frame PNGs + an HTML flipbook.
    html = ['<!doctype html><meta charset=utf-8><title>After Dark preview</title>',
            '<style>body{background:#111;color:#ddd;font:14px sans-serif;text-align:center}'
            'h1{font-weight:600}.g{display:flex;flex-wrap:wrap;justify-content:center;gap:18px}'
            '.c{background:#000;padding:8px;border-radius:8px}img{width:320px;display:block}'
            '.n{margin-top:6px;color:#9cf}</style>',
            '<h1>After Dark recreation suite — module previews</h1>',
            f'<p>{len(modules)} modules. Frames animate automatically.</p><div class=g>']
    frame_lists = {}
    for mod in sorted(modules):
        names = []
        for k, ppm in enumerate(modules[mod]):
            img = read_ppm(ppm)
            out = os.path.join('png', f'{mod}_{k:02d}.png')
            write_png(os.path.join(out_dir, out), img)
            names.append(out)
        frame_lists[mod] = names
        html.append(f'<div class=c><img id="i_{mod}" src="{names[0]}">'
                    f'<div class=n>{mod}</div></div>')
    html.append('</div><script>var F=' + repr(frame_lists).replace("'", '"') +
                ';var k=0;setInterval(function(){k++;for(var m in F){'
                'var a=F[m];document.getElementById("i_"+m).src=a[k%a.length];}},250);'
                '</script>')
    with open(os.path.join(out_dir, 'index.html'), 'w') as f:
        f.write('\n'.join(html))

    # Captioned montage: one representative frame per module in a grid.
    mods = sorted(modules)
    cols = 4 if len(mods) <= 16 else 5
    rows = (len(mods) + cols - 1) // cols
    tw, th, pad, cap = 300, 188, 10, 18
    cellw, cellh = tw + pad * 2, th + pad * 2 + cap
    montage = Img(cols * cellw, rows * cellh, (16, 16, 20))
    for i, mod in enumerate(mods):
        rep = modules[mod][len(modules[mod]) // 2]  # a mid frame
        thumb = downscale(read_ppm(rep), tw, th)
        cx = (i % cols) * cellw + pad
        cy = (i // cols) * cellh + pad
        montage.blit(thumb, cx, cy)
        montage.text(cx, cy + th + 5, mod, 2, (160, 200, 255))
    write_png(os.path.join(out_dir, 'montage.png'), montage)
    print(f'gallery: {len(mods)} modules -> {out_dir} (montage.png, index.html)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
