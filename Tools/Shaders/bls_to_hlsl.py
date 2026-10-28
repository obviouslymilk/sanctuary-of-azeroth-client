"""Rewrites DX9 shader model 2/3 .bls shaders from the 3.3.5a MPQs as HLSL for the D3D11 device.

    python -I bls_to_hlsl.py <Data dir> <Vertex|Pixel> <name> <out.hlsl> [--profile vs_3_0|ps_3_0]
        [--perms 0,1] [--cmp 5,6@0-3]

Every permutation becomes a `#if PERMUTATION == n` block with the same constant registers, so the
cbuffer layout the game fills (`c[N]` in b0) stays as it is. Instructions are copied one to one;
fog and alpha test, which the DX9 fixed function stages did after the pixel shader, are left to the
caller through the helpers in Common.hlsli.

`--cmp <samplers>[@<permutations>]` marks depth texture samplers: DX9 hardware PCF returns the
comparison result of coord.z there, which D3D11 only does through a comparison sampler.
"""
import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bls_dump  # noqa: E402

USAGE = {
    "position": "POSITION", "normal": "NORMAL", "color": "COLOR", "texcoord": "TEXCOORD",
    "blendweight": "BLENDWEIGHT", "blendindices": "BLENDINDICES", "fog": "FOGFACTOR",
}

OPERAND = re.compile(r"^(-?)(\w+?)(\d*)(?:\[a0\.([xyzw])\])?(_abs)?(?:\.([xyzw]+))?$")


class Operand:
    def __init__(self, text, defs):
        m = OPERAND.match(text)
        if not m:
            raise ValueError("operand %r" % text)
        self.neg, self.kind, self.index, self.relative, self.abs, swizzle = m.groups()
        self.swizzle = swizzle
        self.defs = defs

    def name(self):
        if self.relative:
            return "c[%s + a0.%s]" % (self.index, self.relative)
        if self.kind == "c":
            return ("d%s" % self.index) if int(self.index) in self.defs else "c[%s]" % self.index
        return "%s%s" % (self.kind, self.index)

    def mask(self):
        return self.swizzle or "xyzw"

    def src(self, count=4):
        s = self.swizzle or "xyzw"
        if len(s) == 1:
            s = s * 4
        s = (s + s[-1] * 4)[:4]
        text = "%s.%s" % (self.name(), s)
        if self.abs:
            text = "abs(%s)" % text
        if self.neg:
            text = "-" + text
        return text if count == 4 else "(%s).%s" % (text, "xyzw"[:count])

    def scalar(self):
        s = (self.swizzle or "x")[0]
        text = "%s.%s" % (self.name(), s)
        if self.abs:
            text = "abs(%s)" % text
        if self.neg:
            text = "-" + text
        return text


def split_operands(rest):
    return [part.strip() for part in rest.split(",")] if rest else []


def perm_body(text, stage, cmp_samplers=frozenset()):
    """Returns (inputs, outputs, samplers, body lines, temps, defs) for one disassembled permutation."""
    defs = {}
    inputs, outputs, samplers, temps = {}, {}, {}, set()
    lines, depth = [], 1

    def emit(line, delta=0):
        nonlocal depth
        if delta < 0:
            depth += delta
        lines.append("    " * depth + line)
        if delta > 0:
            depth += delta

    for raw in text.replace(chr(0), "").splitlines():
        line = raw.strip()
        if not line or line.startswith((";", "//", "vs_", "ps_")):
            continue
        op, _, rest = line.partition(" ")
        rest = rest.strip()

        if op == "def":
            parts = split_operands(rest)
            defs[int(parts[0][1:])] = parts[1:]
            continue

        if op.startswith("dcl"):
            if op in ("dcl_2d", "dcl_cube"):
                samplers[int(rest[1:])] = "TextureCube" if op == "dcl_cube" else "Texture2D"
                continue
            usage = op[4:]
            m = re.match(r"([a-z]+?)(\d*)$", usage)
            name, index = USAGE[m.group(1)], int(m.group(2) or 0)
            reg = rest.split(".")[0]
            target = outputs if reg.startswith("o") and stage == "Vertex" else inputs
            target[reg] = (name, index)
            continue

        sat = False
        base = op
        for suffix in ("_sat", "_pp"):
            if base.endswith(suffix):
                sat = sat or suffix == "_sat"
                base = base[: -len(suffix)]

        if base in ("if_ne", "if_lt", "if_gt", "if_ge", "if_le", "if_eq"):
            a, b = [Operand(p, defs) for p in split_operands(rest)]
            cmp_op = {"ne": "!=", "lt": "<", "gt": ">", "ge": ">=", "le": "<=", "eq": "=="}[base[3:]]
            emit("if (%s %s %s) {" % (a.scalar(), cmp_op, b.scalar()), 1)
            continue

        if base == "else":
            emit("} else {", -1)
            depth += 1
            continue

        if base == "endif":
            emit("}", -1)
            continue

        if base == "texkill":
            src = Operand(rest, defs)
            if src.kind == "r":
                temps.add(src.name())
            emit("clip(%s);" % src.src(3))
            continue

        parts = split_operands(rest)

        if base == "mova":
            dest, src = Operand(parts[0], defs), Operand(parts[1], defs)
            if src.kind == "r":
                temps.add(src.name())
            temps.add("a0")
            for ch in dest.mask():
                emit("a0.%s = (int)floor((%s).%s + 0.5f);" % (ch, src.src(), ch))
            continue

        dest = Operand(parts[0], defs)
        srcs = [Operand(p, defs) for p in parts[1:]]
        dmask = dest.mask() if dest.swizzle else "xyzw"

        if dest.kind == "r":
            temps.add(dest.name())

        for s in srcs:
            if s.kind == "r":
                temps.add(s.name())

        def s4(i):
            return srcs[i].src()

        if base == "mov":
            expr = s4(0)
        elif base == "add":
            expr = "%s + %s" % (s4(0), s4(1))
        elif base == "mul":
            expr = "%s * %s" % (s4(0), s4(1))
        elif base == "mad":
            expr = "%s * %s + %s" % (s4(0), s4(1), s4(2))
        elif base == "max":
            expr = "max(%s, %s)" % (s4(0), s4(1))
        elif base == "min":
            expr = "min(%s, %s)" % (s4(0), s4(1))
        elif base == "abs":
            expr = "abs(%s)" % s4(0)
        elif base == "frc":
            expr = "frac(%s)" % s4(0)
        elif base == "lrp":
            expr = "lerp(%s, %s, %s)" % (s4(2), s4(1), s4(0))
        elif base == "cmp":
            expr = "Cmp(%s, %s, %s)" % (s4(0), s4(1), s4(2))
        elif base == "dp3":
            expr = "dot(%s, %s)" % (srcs[0].src(3), srcs[1].src(3))
        elif base == "dp4":
            expr = "dot(%s, %s)" % (s4(0), s4(1))
        elif base == "dp2add":
            expr = "dot(%s, %s) + %s" % (srcs[0].src(2), srcs[1].src(2), srcs[2].scalar())
        elif base == "rcp":
            expr = "1.0f / %s" % srcs[0].scalar()
        elif base == "rsq":
            expr = "rsqrt(abs(%s))" % srcs[0].scalar()
        elif base == "pow":
            expr = "pow(abs(%s), %s)" % (srcs[0].scalar(), srcs[1].scalar())
        elif base == "nrm":
            expr = "float4(normalize(%s), 0.0f)" % srcs[0].src(3)
        elif base in ("texld", "texldl"):
            sampler = int(srcs[1].index)
            dims = 3 if samplers.get(sampler) == "TextureCube" else 2
            coord = srcs[0].src(4)
            if sampler in cmp_samplers:
                expr = "ShadowCompare(tex%d, smp%d, %s)" % (sampler, sampler, srcs[0].src(3))
            elif base == "texld":
                expr = "tex%d.Sample(smp%d, %s)" % (sampler, sampler, "(%s).%s" % (coord, "xyzw"[:dims]))
            else:
                expr = "tex%d.SampleLevel(smp%d, %s, %s)" % (sampler, sampler, "(%s).%s" % (coord, "xyzw"[:dims]), "(%s).w" % coord)
        else:
            raise ValueError("unsupported instruction %r" % line)

        if base in ("dp3", "dp4", "dp2add", "rcp", "rsq", "pow"):
            value = expr
            if sat:
                value = "saturate(%s)" % value
            emit("%s.%s = %s;" % (dest.name(), dmask, value))
        else:
            value = "(%s)" % expr
            if sat:
                value = "saturate%s" % value
            emit("%s.%s = %s.%s;" % (dest.name(), dmask, value, dmask))

    return inputs, outputs, samplers, lines, temps, defs


def build_variant(text, stage, cmp_samplers=frozenset()):
    inputs, outputs, samplers, body, temps, defs = perm_body(text, stage, cmp_samplers)
    out = []

    if stage == "Vertex":
        out.append("struct VsInput {")
        for reg, (name, index) in sorted(inputs.items(), key=lambda kv: int(kv[0][1:])):
            out.append("    float4 %s : %s%d;" % (reg, name, index))
        out.append("};")
        out.append("struct VsOutput {")
        for reg, (name, index) in sorted(outputs.items(), key=lambda kv: int(kv[0][1:])):
            semantic = "SV_Position" if name == "POSITION" else "%s%d" % (name, index)
            out.append("    float4 %s : %s;" % (reg, semantic))
        out.append("};")
        out.append("VsOutput main(VsInput input) {")
        out.append("    VsOutput output = (VsOutput)0;")
        for reg in sorted(inputs, key=lambda r: int(r[1:])):
            out.append("    float4 %s = input.%s;" % (reg, reg))
        for reg in sorted(outputs, key=lambda r: int(r[1:])):
            out.append("    float4 %s = float4(0.0f, 0.0f, 0.0f, 0.0f);" % reg)
    else:
        for index in sorted(samplers):
            out.append("%s tex%d : register(t%d);" % (samplers[index], index, index))
            kind = "SamplerComparisonState" if index in cmp_samplers else "SamplerState"
            out.append("%s smp%d : register(s%d);" % (kind, index, index))
        out.append("struct PsInput {")
        out.append("    float4 position : SV_Position;")
        for reg, (name, index) in sorted(inputs.items(), key=lambda kv: int(kv[0][1:])):
            out.append("    float4 %s : %s%d;" % (reg, name, index))
        out.append("};")
        out.append("float4 main(PsInput input) : SV_Target {")
        for reg in sorted(inputs, key=lambda r: int(r[1:])):
            out.append("    float4 %s = input.%s;" % (reg, reg))
        out.append("    float4 oC0 = float4(0.0f, 0.0f, 0.0f, 0.0f);")

    for index in sorted(defs):
        out.append("    const float4 d%d = float4(%s);" % (index, ", ".join(defs[index])))

    for reg in sorted(temps, key=lambda r: int(r[1:])):
        if reg == "a0":
            out.append("    int4 a0 = int4(0, 0, 0, 0);")
        else:
            out.append("    float4 %s = float4(0.0f, 0.0f, 0.0f, 0.0f);" % reg)

    out.extend(body)

    if stage == "Vertex":
        for reg in sorted(outputs, key=lambda r: int(r[1:])):
            out.append("    output.%s = %s;" % (reg, reg))
        out.append("    return output;")
    else:
        out.append("    AlphaTest(oC0.w);")
        out.append("    return oC0;")

    out.append("}")
    return "\n".join(out)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("data_dir")
    parser.add_argument("stage", choices=["Vertex", "Pixel"])
    parser.add_argument("name")
    parser.add_argument("out")
    parser.add_argument("--profile")
    parser.add_argument("--perms", help="comma separated permutations to keep (default all)")
    parser.add_argument("--cmp", help="depth texture samplers, e.g. 5,6 or 4@4-7 to limit them to permutations")
    args = parser.parse_args()

    profile = args.profile or ("vs_3_0" if args.stage == "Vertex" else "ps_3_0")
    archives = bls_dump.open_archives(args.data_dir)
    path = "Shaders\\%s\\%s\\%s.bls" % (args.stage, profile, args.name)
    data, source = bls_dump.read_file(archives, path)

    if data is None:
        sys.exit("not found: " + path)

    perms = bls_dump.split_bls(data)
    keep = set(int(p) for p in args.perms.split(",")) if args.perms else set(range(len(perms)))
    variants = {}
    cmp_samplers, cmp_perms = set(), None

    if args.cmp:
        samplers, _, limit = args.cmp.partition("@")
        cmp_samplers = set(int(n) for n in samplers.split(","))

        if limit:
            cmp_perms = set()
            for part in limit.split(","):
                low, _, high = part.partition("-")
                cmp_perms.update(range(int(low), int(high or low) + 1))

    for i, code in enumerate(perms):
        if i not in keep:
            continue
        text = bls_dump.disassemble(code)
        active = cmp_samplers if cmp_perms is None or i in cmp_perms else set()
        variants.setdefault(build_variant(text, args.stage, active), []).append(i)

    out = ['// Rewritten from %s (%d permutations, %d distinct) by Tools/Shaders/bls_to_hlsl.py.' % (path, len(perms), len(variants)),
           '#include "../Common.hlsli"', ""]

    for body, indices in variants.items():
        out.append("#if " + " || ".join("PERMUTATION == %d" % i for i in indices))
        out.append(body)
        out.append("#endif")
        out.append("")

    Path(args.out).write_text("\n".join(out), encoding="utf-8", newline="\n")
    print("%s: %d permutations, %d distinct" % (args.name, len(keep), len(variants)))


if __name__ == "__main__":
    main()
