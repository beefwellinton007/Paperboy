#!/usr/bin/env python3
"""Convert PNGs to the engine's .adspr raw-RGBA format.

Pure stdlib. Supports 8-bit PNGs: RGB, RGBA, grayscale, gray+alpha, and palette
(with optional tRNS). This is the build-time step so the runtime needs no PNG or
zlib decoder.

Usage:
  pack_assets.py <in.png> <out.adspr>
  pack_assets.py <src_dir> <dst_dir>     # converts every *.png in src_dir
"""
import glob
import os
import struct
import sys
import zlib


def _paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    return b if pb <= pc else c


def read_png(path):
    """Return (width, height, rgba_bytes)."""
    with open(path, 'rb') as f:
        data = f.read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError(f'{path}: not a PNG')
    pos = 8
    width = height = bit_depth = color_type = 0
    idat = bytearray()
    palette = b''
    trns = b''
    while pos < len(data):
        (length,) = struct.unpack('>I', data[pos:pos + 4])
        ctype = data[pos + 4:pos + 8]
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length  # length + type + data + crc
        if ctype == b'IHDR':
            width, height, bit_depth, color_type = struct.unpack(
                '>IIBB', chunk[:10])
        elif ctype == b'PLTE':
            palette = chunk
        elif ctype == b'tRNS':
            trns = chunk
        elif ctype == b'IDAT':
            idat += chunk
        elif ctype == b'IEND':
            break
    if bit_depth != 8:
        raise ValueError(f'{path}: only 8-bit PNGs supported (got {bit_depth})')

    channels = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}[color_type]
    raw = zlib.decompress(bytes(idat))
    stride = width * channels
    out = bytearray()
    prev = bytearray(stride)
    p = 0
    for _ in range(height):
        filt = raw[p]
        p += 1
        line = bytearray(raw[p:p + stride])
        p += stride
        for i in range(stride):
            a = line[i - channels] if i >= channels else 0
            b = prev[i]
            c = prev[i - channels] if i >= channels else 0
            x = line[i]
            if filt == 1:
                x += a
            elif filt == 2:
                x += b
            elif filt == 3:
                x += (a + b) >> 1
            elif filt == 4:
                x += _paeth(a, b, c)
            line[i] = x & 0xff
        out += line
        prev = line

    # Normalize to RGBA.
    rgba = bytearray(width * height * 4)
    for i in range(width * height):
        if color_type == 6:      # RGBA
            rgba[i * 4:i * 4 + 4] = out[i * 4:i * 4 + 4]
        elif color_type == 2:    # RGB
            rgba[i * 4:i * 4 + 3] = out[i * 3:i * 3 + 3]
            rgba[i * 4 + 3] = 255
        elif color_type == 0:    # gray
            g = out[i]
            rgba[i * 4:i * 4 + 3] = bytes((g, g, g))
            rgba[i * 4 + 3] = 255
        elif color_type == 4:    # gray + alpha
            g = out[i * 2]
            rgba[i * 4:i * 4 + 3] = bytes((g, g, g))
            rgba[i * 4 + 3] = out[i * 2 + 1]
        elif color_type == 3:    # palette
            idx = out[i]
            rgba[i * 4:i * 4 + 3] = palette[idx * 3:idx * 3 + 3]
            rgba[i * 4 + 3] = trns[idx] if idx < len(trns) else 255
    return width, height, bytes(rgba)


def write_adspr(path, w, h, rgba):
    with open(path, 'wb') as f:
        f.write(b'ADSP')
        f.write(struct.pack('<HH', w, h))
        f.write(rgba)


def convert(src, dst):
    w, h, rgba = read_png(src)
    write_adspr(dst, w, h, rgba)
    print(f'{os.path.basename(src)} -> {os.path.basename(dst)} ({w}x{h})')


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 1
    src, dst = sys.argv[1], sys.argv[2]
    if os.path.isdir(src):
        os.makedirs(dst, exist_ok=True)
        for png in sorted(glob.glob(os.path.join(src, '*.png'))):
            name = os.path.splitext(os.path.basename(png))[0] + '.adspr'
            convert(png, os.path.join(dst, name))
    else:
        convert(src, dst)
    return 0


if __name__ == '__main__':
    sys.exit(main())
