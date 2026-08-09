#!/usr/bin/env python3
"""Generate the synthetic AMAP.DAT used by the gakuen_king scenario test.

Each page is a 100x100 grid of 16-bit base cells. The cell value is derived
from the coordinates:

    tile  = (x + y * 7 + page * 11) % 192   -> low byte, N's D03
    bit13 = 1 when (x + y) is odd           -> N's D02 (blocked)
    bit14 = 1 when (x * y) % 3 == 0         -> N's D01

Bits 8-12 of the cell are left at 0; N derives D04 from the low byte anyway.

Usage:
    ./make_amap.py && dri create AMAP.DAT page1.bin page2.bin
"""

import struct

WIDTH = HEIGHT = 100
PAGES = 2


def cell(page, x, y):
    tile = (x + y * 7 + page * 11) % 192
    value = tile
    if (x + y) % 2:
        value |= 1 << 13
    if (x * y) % 3 == 0:
        value |= 1 << 14
    return value


def main():
    for page in range(1, PAGES + 1):
        data = b"".join(
            struct.pack("<H", cell(page, x, y))
            for y in range(HEIGHT)
            for x in range(WIDTH)
        )
        with open(f"page{page}.bin", "wb") as f:
            f.write(data)


if __name__ == "__main__":
    main()
