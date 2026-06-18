import struct
import sys
import os
from pathlib import Path

def read_bmp(path):
    with open(path, 'rb') as f:
        header = f.read(14)
        if header[:2] != b'BM':
            raise ValueError("Not a BMP file")
        _, _, _, data_offset = struct.unpack_from('<I H H I', header, 2)

        dib = f.read(4)
        dib_size = struct.unpack('<I', dib)[0]
        dib += f.read(dib_size - 4)

        width = struct.unpack_from('<i', dib, 4)[0]
        height = struct.unpack_from('<i', dib, 8)[0]
        if width <= 0 or height <= 0:
            raise ValueError("Unsupported BMP: negative or zero dimensions")

        bpp = struct.unpack_from('<H', dib, 14)[0]
        compression = struct.unpack_from('<I', dib, 16)[0]
        if compression != 0:
            raise ValueError(f"Compressed BMP not supported (compression={compression})")

        row_size = ((width * bpp + 31) // 32) * 4

        pixels = [None] * (width * height)
        f.seek(data_offset)
        for y in range(height):
            row_data = f.read(row_size)
            base = y * width
            for x in range(width):
                offset = x * (bpp // 8)
                r = row_data[offset + 2]
                g = row_data[offset + 1]
                b = row_data[offset]
                pixels[base + x] = (r, g, b)

        return width, height, pixels

def rgb_to_4bit(r, g, b):
    gray = (r * 30 + g * 59 + b * 11) // 100
    return (gray * 15 + 127) // 255

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <image.bmp>")
        sys.exit(1)

    bmp_path = sys.argv[1]
    width, height, pixels = read_bmp(bmp_path)

    bytes_per_row = (width + 1) // 2
    pixel_bytes = bytearray(bytes_per_row * height)

    for y in range(height):
        base = y * width
        row_out = (height - 1 - y) * bytes_per_row
        for x in range(0, width, 2):
            lo = rgb_to_4bit(*pixels[base + x])
            hi = 0
            if x + 1 < width:
                hi = rgb_to_4bit(*pixels[base + x + 1])
            pixel_bytes[row_out + (x // 2)] = (lo & 0x0F) | ((hi & 0x0F) << 4)

    data = bytearray(4 + len(pixel_bytes))
    struct.pack_into('<H', data, 0, width)
    struct.pack_into('<H', data, 2, height)
    data[4:] = pixel_bytes

    stem = Path(bmp_path).stem
    safe_name = ''.join(c if c.isalnum() else '_' for c in stem)

    out_path = os.path.join(os.path.dirname(bmp_path) or '.', f'img_{safe_name}.c')
    lines = []
    lines.append('#include <stdint.h>')
    lines.append('')
    lines.append(f'const uint8_t img_{safe_name}[] = {{')

    chunks = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        chunks.append('    ' + ', '.join(f'0x{b:02X}' for b in chunk))

    lines.append(',\n'.join(chunks))
    lines.append('};')
    lines.append('')

    with open(out_path, 'w') as f:
        f.write('\n'.join(lines))

    print(f"Output: {out_path}")
    print(f"  Dimensions: {width}x{height}")
    print(f"  Array size: {len(data)} bytes")

if __name__ == '__main__':
    main()
