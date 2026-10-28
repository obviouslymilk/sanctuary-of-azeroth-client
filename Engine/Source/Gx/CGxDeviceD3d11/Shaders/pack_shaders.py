"""Packs compiled shader permutations into .bls blobs and writes the lookup table source.

    pack_shaders.py <out.cpp> <Vertex|Pixel>:<name>=<perm0.cso>,<perm1.cso> ...
"""
import struct
import sys

PROFILES = {"Vertex": "vs_5_0", "Pixel": "ps_5_0"}


def blob(files):
    out = struct.pack("<4sII", b"HSXG", 0x10003, 0)
    for path in files:
        with open(path, "rb") as f:
            code = f.read()
        out += struct.pack("<IIHHI", 0, 0, 0, 0, len(code)) + code
        out += b"\0" * (-len(code) % 4)
    return out


def main():
    out_path, entries = sys.argv[1], sys.argv[2:]
    lines = ['#include "Engine/Source/Gx/CGxDeviceD3d11/GxEmbeddedShaders.h"', ""]
    table = []
    for i, entry in enumerate(entries):
        key, files = entry.split("=", 1)
        target, name = key.split(":", 1)
        data = blob(files.split(","))
        lines.append("static const unsigned char s_blob%d[] = {" % i)
        for pos in range(0, len(data), 16):
            lines.append("    " + ", ".join("0x%02x" % b for b in data[pos:pos + 16]) + ",")
        lines.append("};")
        lines.append("")
        path = chr(92).join(['Shaders', target, PROFILES[target], name + '.bls'])
        table.append('    { "%s", s_blob%d, sizeof(s_blob%d) },' % (path.replace(chr(92), chr(92) * 2), i, i))
    lines.append("const GxEmbeddedShader g_gxEmbeddedShaders[] = {")
    lines += table
    lines.append("};")
    lines.append("")
    lines.append("const unsigned int g_gxEmbeddedShaderCount = %d;" % len(entries))
    with open(out_path, "w", newline="\n") as f:
        f.write("\n".join(lines) + "\n")


main()
