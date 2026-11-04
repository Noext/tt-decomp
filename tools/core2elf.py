#!/usr/bin/env python3
"""Convert a Polyphony Digital PS2 CORE executable (CORE.GT4, CORE.TT) into an ELF.

Layout, outermost first (published at the Gran Turismo Modding Hub, "PS2 Executables (CORE)"):

  u8[8] IV | Salsa20/20 stream | u32 CRC          (key: the 16 bytes "PolyphonyDigital")
    -> u16 boot flags | u32 decompressed size | raw deflate data
       -> u16 hash size | hash | u16 hash size | hash | i32 section count | i32 entry
          | per section: i32 EE address | i32 size | bytes

usage: core2elf.py CORE.TT out.elf [--info]

Nothing is written except the ELF; the input is read only.
"""
import struct
import sys
import zlib

KEY = b"PolyphonyDigital"


def _rotl(v, c):
    return ((v << c) & 0xFFFFFFFF) | (v >> (32 - c))


def salsa20_block(key, iv, counter):
    """One 64-byte Salsa20/20 keystream block for a 16-byte key (constant "expand 16-byte k")."""
    k = struct.unpack("<4I", key)
    c = struct.unpack("<4I", b"expand 16-byte k")
    n = struct.unpack("<2I", iv)
    state = [c[0], k[0], k[1], k[2], k[3], c[1], n[0], n[1],
             counter & 0xFFFFFFFF, counter >> 32, c[2], k[0], k[1], k[2], k[3], c[3]]
    x = list(state)
    for _ in range(10):
        for a, b, cc, d in ((0, 4, 8, 12), (5, 9, 13, 1), (10, 14, 2, 6), (15, 3, 7, 11),
                            (0, 1, 2, 3), (5, 6, 7, 4), (10, 11, 8, 9), (15, 12, 13, 14)):
            x[b] ^= _rotl((x[a] + x[d]) & 0xFFFFFFFF, 7)
            x[cc] ^= _rotl((x[b] + x[a]) & 0xFFFFFFFF, 9)
            x[d] ^= _rotl((x[cc] + x[b]) & 0xFFFFFFFF, 13)
            x[a] ^= _rotl((x[d] + x[cc]) & 0xFFFFFFFF, 18)
    return struct.pack("<16I", *[(x[i] + state[i]) & 0xFFFFFFFF for i in range(16)])


def decrypt(raw):
    """raw[8:-4] XOR the Salsa20 keystream, IV = raw[:8]. Returns the compressed CORE."""
    iv, body = raw[:8], raw[8:-4]
    out = bytearray(body)
    for i in range((len(body) + 63) // 64):
        ks = salsa20_block(KEY, iv, i)
        base = i * 64
        chunk = body[base:base + 64]
        for j in range(len(chunk)):
            out[base + j] = chunk[j] ^ ks[j]
    return bytes(out)


def unpack(raw):
    """(flags, [hashes], entry, [(addr, bytes)]) of a CORE file, encrypted or not."""
    first = None
    for plain in (raw, decrypt(raw)):
        try:
            return _unpack(plain)
        except (zlib.error, struct.error, ValueError) as e:
            first = first or e
    raise SystemExit(f"neither the raw nor the decrypted file is a CORE: {first}")


def _unpack(raw):
    flags, size = struct.unpack_from("<HI", raw, 0)
    inflater = zlib.decompressobj(-15)
    data = inflater.decompress(raw[6:])
    if len(data) != size:
        raise ValueError(f"decompressed {len(data)} bytes, header says {size}")
    pos = 0
    hashes = []
    for _ in range(2):
        (hsize,) = struct.unpack_from("<H", data, pos)
        hashes.append(data[pos + 2:pos + 2 + hsize])
        pos += 2 + hsize
    count, entry = struct.unpack_from("<iI", data, pos)
    pos += 8
    if not 0 < count <= 16:
        raise ValueError(f"implausible section count {count}")
    sections = []
    for _ in range(count):
        addr, length = struct.unpack_from("<Ii", data, pos)
        pos += 8
        sections.append((addr, data[pos:pos + length]))
        pos += length
    return flags, hashes, entry, sections


def drop_contained(sections):
    """Drop a section that lies entirely inside another: CORE files repeat the last 24 bytes of
    the code as a section of their own, and splat must see one contiguous code segment."""
    out = []
    for i, (addr, data) in enumerate(sections):
        inside = False
        for j, (addr2, data2) in enumerate(sections):
            if i != j and addr2 <= addr and addr + len(data) <= addr2 + len(data2):
                inside = True
                break
        if not inside:
            out.append((addr, data))
    return out


def write_elf(flags, entry, sections, out):
    """One PT_LOAD per section, section headers, no symbols: enough for splat, Ghidra and objdump."""
    SHT_PROGBITS, SHT_NULL, SHT_STRTAB = 1, 0, 3
    PF_X, PF_W, PF_R = 1, 2, 4
    shstr = b"\0.text\0.data\0.rodata\0.shstrtab\0"
    names = [b".text", b".data", b".rodata"]
    ehsize, phentsize, shentsize = 52, 32, 40
    phnum, shnum = len(sections) + 1, len(sections) + 2
    phoff = ehsize
    body_off = phoff + phentsize * phnum
    body = bytearray()
    placed = []
    for (addr, data) in sections:
        body.extend(b"\0" * ((16 - len(body) % 16) % 16))
        placed.append((addr, body_off + len(body), data))
        body.extend(data)
    body.extend(b"\0" * ((16 - len(body) % 16) % 16))
    stroff = body_off + len(body)
    body.extend(shstr)
    shoff = body_off + len(body)
    pad = (16 - shoff % 16) % 16
    body.extend(b"\0" * pad)
    shoff += pad

    ph = [struct.pack("<IIIIIIII", 0x70000000, 0, 0, 0, 0, 0, PF_R, 4)]  # RN_MIPS_REGINFO-ish
    for i, (addr, off, data) in enumerate(placed):
        ph.append(struct.pack("<IIIIIIII", 1, off, addr, addr, len(data), len(data), PF_R | PF_W | PF_X, 16))
    sh = [struct.pack("<IIIIIIIIII", 0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0, 0)]
    for i, (addr, off, data) in enumerate(placed):
        sh.append(struct.pack("<IIIIIIIIII", 1 + sum(len(n) + 1 for n in names[:i]), SHT_PROGBITS, 6,
                              addr, off, len(data), 0, 0, 16, 0))
    sh.append(struct.pack("<IIIIIIIIII", 1 + sum(len(n) + 1 for n in names), SHT_STRTAB, 0, 0, stroff,
                          len(shstr), 0, 0, 1, 0))
    shbin = b"".join(sh)
    body.extend(b"\0" * max(0, shoff + len(shbin) - (body_off + len(body))))
    body[shoff - body_off:shoff - body_off + len(shbin)] = shbin
    ident = b"\x7fELF\x01\x01\x01" + b"\0" * 9
    hdr = struct.pack("<16sHHIIIIIHHHHHH", ident, 2, 8, 1, entry, phoff, shoff, 0x20924001,
                      ehsize, phentsize, phnum, shentsize, shnum, len(placed) + 1)
    with open(out, "wb") as f:
        f.write(hdr)
        f.write(b"".join(ph))
        f.write(body)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not args:
        sys.exit(__doc__)
    raw = open(args[0], "rb").read()
    flags, hashes, entry, sections = unpack(raw)
    print(f"flags {flags:#06x}  entry {entry:#010x}  sections {len(sections)}  "
          f"hashes {[h.hex()[:16] + '...' for h in hashes]}")
    for addr, data in sections:
        print(f"  {addr:#010x}  {len(data):>9,} bytes")
    if "--info" in sys.argv:
        return
    write_elf(flags, entry, drop_contained(sections), args[1])
    print("wrote", args[1])


if __name__ == "__main__":
    main()
