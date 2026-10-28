"""Extracts DX9 .bls shader blobs from the 3.3.5a MPQs, splits them into permutations and
prints the disassembly.

    python -I bls_dump.py <Data dir> list [substring]
    python -I bls_dump.py <Data dir> dump <path in MPQ> [--out <dir>]

Example path: Shaders\\Pixel\\ps_3_0\\UI.bls
Needs Windows (d3dcompiler_47.dll) for the disassembly.
"""
import argparse
import bz2
import ctypes
import os
import struct
import sys
import zlib
from pathlib import Path

MPQ_FILE_COMPRESS = 0x200
MPQ_FILE_IMPLODE = 0x100
MPQ_FILE_ENCRYPTED = 0x10000
MPQ_FILE_FIX_KEY = 0x20000
MPQ_FILE_PATCH = 0x100000
MPQ_FILE_SINGLE_UNIT = 0x1000000
MPQ_FILE_CRC = 0x4000000
MPQ_FILE_EXISTS = 0x80000000

# Later entries override earlier ones, like the client's own load order.
ARCHIVE_ORDER = [
    "common.MPQ", "common-2.MPQ", "expansion.MPQ", "lichking.MPQ",
    "patch.MPQ", "patch-2.MPQ", "patch-3.MPQ",
]


def _build_crypt_table():
    table = [0] * 0x500
    seed = 0x00100001
    for i in range(0x100):
        for j in range(5):
            index = i + j * 0x100
            seed = (seed * 125 + 3) % 0x2AAAAB
            t1 = (seed & 0xFFFF) << 16
            seed = (seed * 125 + 3) % 0x2AAAAB
            t2 = seed & 0xFFFF
            table[index] = t1 | t2
    return table


CRYPT = _build_crypt_table()


def hash_string(name, kind):
    seed1, seed2 = 0x7FED7FED, 0xEEEEEEEE
    for ch in name.upper().replace("/", "\\"):
        c = ord(ch)
        seed1 = CRYPT[(kind << 8) + c] ^ ((seed1 + seed2) & 0xFFFFFFFF)
        seed2 = (c + seed1 + seed2 + (seed2 << 5) + 3) & 0xFFFFFFFF
    return seed1


def decrypt(data, key):
    out = []
    seed = 0xEEEEEEEE
    for (v,) in struct.iter_unpack("<I", data):
        seed = (seed + CRYPT[0x400 + (key & 0xFF)]) & 0xFFFFFFFF
        v ^= (key + seed) & 0xFFFFFFFF
        key = ((((~key) << 21) + 0x11111111) | (key >> 11)) & 0xFFFFFFFF
        seed = (v + seed + (seed << 5) + 3) & 0xFFFFFFFF
        out.append(v)
    return struct.pack("<%dI" % len(out), *out)


class Mpq:
    def __init__(self, path):
        self.path = path
        self.f = open(path, "rb")
        head = self.f.read(0x20)
        self.base = 0
        if head[:4] == b"MPQ\x1b":
            self.base = struct.unpack_from("<I", head, 8)[0]
            self.f.seek(self.base)
            head = self.f.read(0x20)
        if head[:4] != b"MPQ\x1a":
            raise ValueError("not an MPQ: %s" % path)
        (_, _, _, version, shift, hash_off, block_off, hash_n, block_n) = struct.unpack_from("<4sIIHHIIII", head)
        if version > 1:
            raise ValueError("MPQ v%d is not supported: %s" % (version + 1, path))
        self.sector_size = 512 << shift
        hash_data = decrypt(self._read(hash_off, hash_n * 16), hash_string("(hash table)", 3))
        self.hashes = list(struct.iter_unpack("<IIHHI", hash_data))
        block_data = decrypt(self._read(block_off, block_n * 16), hash_string("(block table)", 3))
        self.blocks = list(struct.iter_unpack("<IIII", block_data))

    def _read(self, offset, size):
        self.f.seek(self.base + offset)
        return self.f.read(size)

    def find(self, name):
        a, b = hash_string(name, 1), hash_string(name, 2)
        mask = len(self.hashes) - 1
        i = hash_string(name, 0) & mask
        for _ in range(len(self.hashes)):
            ha, hb, _, _, block = self.hashes[i]
            if block == 0xFFFFFFFF:
                return None
            if ha == a and hb == b and block != 0xFFFFFFFE:
                return self.blocks[block]
            i = (i + 1) & mask
        return None

    def read(self, name):
        entry = self.find(name)
        if entry is None:
            return None
        offset, comp_size, size, flags = entry
        if not flags & MPQ_FILE_EXISTS:
            return None
        if flags & MPQ_FILE_PATCH:
            raise ValueError("%s is a patch file in %s" % (name, self.path))
        key = 0
        if flags & MPQ_FILE_ENCRYPTED:
            key = hash_string(name.replace("/", "\\").split("\\")[-1], 3)
            if flags & MPQ_FILE_FIX_KEY:
                key = ((key + offset) ^ size) & 0xFFFFFFFF
        raw = self._read(offset, comp_size)
        if flags & MPQ_FILE_SINGLE_UNIT:
            if flags & MPQ_FILE_ENCRYPTED:
                raw = decrypt(raw, key)
            return self._unpack(raw, size, flags)
        sectors = (size + self.sector_size - 1) // self.sector_size
        table_len = (sectors + 1 + (1 if flags & MPQ_FILE_CRC else 0)) * 4
        if flags & (MPQ_FILE_COMPRESS | MPQ_FILE_IMPLODE):
            table = raw[:table_len]
            if flags & MPQ_FILE_ENCRYPTED:
                table = decrypt(table, key - 1)
            offsets = struct.unpack_from("<%dI" % (sectors + 1), table)
        else:
            offsets = [i * self.sector_size for i in range(sectors)] + [size]
        out = bytearray()
        for i in range(sectors):
            chunk = raw[offsets[i]:offsets[i + 1]]
            if flags & MPQ_FILE_ENCRYPTED:
                chunk = decrypt(chunk.ljust((len(chunk) + 3) & ~3, b"\0"), key + i)[:len(chunk)]
            want = min(self.sector_size, size - i * self.sector_size)
            out += self._unpack(chunk, want, flags)
        return bytes(out)

    @staticmethod
    def _unpack(data, want, flags):
        if len(data) >= want or not flags & (MPQ_FILE_COMPRESS | MPQ_FILE_IMPLODE):
            return data
        if flags & MPQ_FILE_IMPLODE:
            raise ValueError("PKWARE implode is not supported")
        mask, body = data[0], data[1:]
        if mask == 0x02:
            return zlib.decompress(body)
        if mask == 0x10:
            return bz2.decompress(body)
        raise ValueError("compression 0x%02x is not supported" % mask)


def open_archives(data_dir):
    archives = []
    for name in ARCHIVE_ORDER:
        path = Path(data_dir) / name
        if path.exists():
            archives.append(Mpq(str(path)))
    return archives


def read_file(archives, name):
    for mpq in reversed(archives):
        data = mpq.read(name)
        if data is not None:
            return data, mpq.path
    return None, None


def split_bls(data):
    magic, version, _ = struct.unpack_from("<4sII", data)
    if magic != b"HSXG" or version != 0x10003:
        raise ValueError("not a GXSH 0x10003 blob")
    pos, perms = 12, []
    while pos + 16 <= len(data):
        a, b, c, d, size = struct.unpack_from("<IIHHI", data, pos)
        pos += 16
        perms.append(data[pos:pos + size])
        pos += (size + 3) & ~3
    return perms


def disassemble(code):
    lib = ctypes.WinDLL("d3dcompiler_47.dll")
    blob = ctypes.c_void_p()
    lib.D3DDisassemble.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_uint, ctypes.c_char_p, ctypes.POINTER(ctypes.c_void_p)]
    hr = lib.D3DDisassemble(code, len(code), 0, None, ctypes.byref(blob))
    if hr != 0:
        return "; D3DDisassemble failed: 0x%08x" % (hr & 0xFFFFFFFF)
    vtbl = ctypes.cast(ctypes.cast(blob, ctypes.POINTER(ctypes.c_void_p))[0], ctypes.POINTER(ctypes.c_void_p))
    get_ptr = ctypes.WINFUNCTYPE(ctypes.c_void_p, ctypes.c_void_p)(vtbl[3])
    get_size = ctypes.WINFUNCTYPE(ctypes.c_size_t, ctypes.c_void_p)(vtbl[4])
    release = ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)(vtbl[2])
    text = ctypes.string_at(get_ptr(blob), get_size(blob)).decode("ascii", "replace")
    release(blob)
    return text


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("command", choices=["list", "dump"])
    parser.add_argument("target", nargs="?", default="")
    parser.add_argument("--out", help="also write each permutation as <name>.<n>.bin / .asm here")
    args = parser.parse_args()

    archives = open_archives(args.data_dir)
    if args.command == "list":
        listing = set()
        for mpq in archives:
            data = mpq.read("(listfile)")
            if data:
                listing.update(data.decode("latin-1").split("\r\n"))
        for line in sorted(listing):
            if line.lower().startswith("shaders\\") and args.target.lower() in line.lower():
                print(line)
        return

    data, source = read_file(archives, args.target)
    if data is None:
        sys.exit("not found: %s" % args.target)
    perms = split_bls(data)
    print("; %s from %s, %d permutations" % (args.target, os.path.basename(source), len(perms)))
    for i, code in enumerate(perms):
        text = disassemble(code)
        print("\n; ---- permutation %d (%d bytes) ----\n%s" % (i, len(code), text))
        if args.out:
            out = Path(args.out)
            out.mkdir(parents=True, exist_ok=True)
            stem = Path(args.target.replace("\\", "/")).stem
            (out / ("%s.%d.bin" % (stem, i))).write_bytes(code)
            (out / ("%s.%d.asm" % (stem, i))).write_text(text)


if __name__ == "__main__":
    main()
