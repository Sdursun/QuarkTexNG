#!/usr/bin/env python3
"""
Generates the parts of QuarkTex's minigl.library that follow from MiniGL's
SDK headers (docs/phase7-minigl.md):

  mglenum.auto.c   MiniGL's GL constants (29 SDK: #defines, OpenGL's own
                   values but for MiniGL's private ones, 0x7000-0x7FFF) to
                   OpenGL's and back
  qgl.auto.h       OpenGL's constants as QGL_* and the command buffer
                   functions (_gl*) with plain C types, so that they can be
                   used next to MiniGL's headers, which define GL_* too
  dispatch.auto.c  the dispatch table in the header's order: generated
                   wrappers for the GL functions that only need their enum
                   arguments translated, the hand-written mgl_<entry>
                   functions for the others, and a logging stub for every
                   hand-written one that does not exist yet

A program built on MiniGL's older SDK (before the 29 release) is not
supported: its GL tokens were auto-numbered positions, not OpenGL's values,
and nothing at OpenLibrary or GetDispatchTable time tells the library which
numbering a given caller uses (neither passes the caller's own version down
to it: checked against a real OpenLibrary call, see minigl.library/lib.c).
The 29 SDK's own release notes call this the same break: a program built on
the older header "needs to get recompiled... with the latest SDK".

usage: mglgen.py <29 SDK include dir> <gl dir> <out dir> <hand-written .c files...>
"""
import os
import re
import sys

sdk, gldir, out = sys.argv[1:4]
handwritten = sys.argv[4:]

# --- MiniGL's constants (29 SDK: #defines, OpenGL's own values) -------------

gl_h = open(os.path.join(sdk, "mgl", "gl.h"), encoding="latin-1").read()
if not re.search(r"^#define\s+GL_BASE\s", gl_h, re.M):
    sys.exit("mglgen: %s is not MiniGL's 29 SDK (its mgl/gl.h has no GL_BASE #define)" % sdk)
defines = dict((n, int(v, 0)) for n, v in
               re.findall(r"^#define\s+((?:GL|MGL)_\w+)\s+(0x[0-9A-Fa-f]+|\d+)\b", gl_h, re.M))
# #defines, some naming others (aliases, see below).
raw = dict(re.findall(r"^#define\s+((?:GL|MGL)_\w+)\s+(\w+)", gl_h, re.M))
mgl = {}
for name, value in raw.items():
    while value in raw:
        value = raw[value]
    try:
        mgl[name] = int(value, 0)
    except ValueError:
        pass

# --- OpenGL's constants -----------------------------------------------------

gl = dict((n, int(v, 0)) for n, v in re.findall(
    r"^#define\s+(GL_\w+)\s+(0x[0-9A-Fa-f]+|\d+)", open(os.path.join(gldir, "gldefines.h")).read(), re.M))
# Extensions and later versions MiniGL names, beyond gldefines.h (OpenGL 1.1).
gl.update({
    "GL_TEXTURE0_ARB": 0x84C0, "GL_TEXTURE1_ARB": 0x84C1, "GL_TEXTURE2_ARB": 0x84C2, "GL_TEXTURE3_ARB": 0x84C3,
    "GL_ACTIVE_TEXTURE_ARB": 0x84E0, "GL_CLIENT_ACTIVE_TEXTURE_ARB": 0x84E1, "GL_MAX_TEXTURE_UNITS_ARB": 0x84E2,
    "GL_COLOR_TABLE": 0x80D0, "GL_COLOR_INDEX8_EXT": 0x80E5, "GL_SHARED_TEXTURE_PALETTE_EXT": 0x81FB,
    "GL_BGR": 0x80E0, "GL_BGRA": 0x80E1, "GL_BGR_EXT": 0x80E0, "GL_BGRA_EXT": 0x80E1,
    "GL_UNSIGNED_SHORT_5_6_5": 0x8363, "GL_UNSIGNED_SHORT_4_4_4_4": 0x8033, "GL_UNSIGNED_SHORT_5_5_5_1": 0x8034,
    "GL_UNSIGNED_SHORT_4_4_4_4_REV": 0x8365, "GL_UNSIGNED_SHORT_1_5_5_5_REV": 0x8366, "GL_UNSIGNED_INT_8_8_8_8_REV": 0x8367,
    "GL_CLAMP_TO_EDGE": 0x812F, "GL_TEXTURE_MAX_LEVEL": 0x813D, "GL_GENERATE_MIPMAP": 0x8191,
    "GL_FUNC_ADD": 0x8006, "GL_MIN": 0x8007, "GL_MAX": 0x8008, "GL_BLEND_EQUATION": 0x8009,
    "GL_FUNC_SUBTRACT": 0x800A, "GL_FUNC_REVERSE_SUBTRACT": 0x800B,
    "GL_RESCALE_NORMAL": 0x803A, "GL_TEXTURE_3D": 0x806F, "GL_POLYGON_OFFSET_FILL": 0x8037,
    "GL_MULTISAMPLE": 0x809D, "GL_COMPRESSED_RGB": 0x84ED, "GL_COMPRESSED_RGBA": 0x84EE,
})
# MiniGL's own: no OpenGL value. Marked, so that the functions taking them
# can recognise and skip them.
QT_MGL_ONLY = 0x7F000000
# MiniGL's packed pixel types name OpenGL's.
aliases = {"MGL_UNSIGNED_SHORT_5_6_5": "GL_UNSIGNED_SHORT_5_6_5", "MGL_UNSIGNED_SHORT_4_4_4_4": "GL_UNSIGNED_SHORT_4_4_4_4",
           "MGL_UBYTE_BGRA": "GL_BGRA", "GL_TEXTURE_DOOR_ARRAY_POINTER": "GL_TEXTURE_COORD_ARRAY_POINTER",
           "GL_POLYGON_OFFSET": "GL_POLYGON_OFFSET_FILL", "GL_TEXTURE_2D_BINDING": "GL_TEXTURE_BINDING_2D"}

# MiniGL's private constants (no OpenGL meaning but a few, caught by aliases).
PRIVATE = range(0x7000, 0x8000)

to_gl = {}
unknown = []
for name, value in mgl.items():
    if name == "GL_NO_ERROR":
        target = 0
    elif value not in PRIVATE:
        target = value  # MiniGL 29's tokens already carry OpenGL's value.
        if name in gl and gl[name] != value:
            print("mglgen: warning: %s is 0x%X in MiniGL's header, 0x%X in OpenGL" % (name, value, gl[name]))
    elif aliases.get(name) in gl:
        target = gl[aliases[name]]
    else:
        target = QT_MGL_ONLY | value
        if name.startswith("GL_"):
            unknown.append(name)
    if value in to_gl and to_gl[value][1] != target:
        sys.exit("mglgen: MiniGL value %d is both %s and %s" % (value, to_gl[value][0], name))
    to_gl[value] = (name, target)

os.makedirs(out, exist_ok=True)
with open(os.path.join(out, "mglenum.auto.c"), "w") as f:
    f.write("/* Generated by minigl.library/mglgen.py from MiniGL's mgl/gl.h - do not edit */\n")
    f.write("/* GL names MiniGL has but this table does not know: %s */\n\n" % (", ".join(sorted(unknown)) or "none"))
    known = sorted(set(v for v in gl.values() if v > 1) | set(v for v, (n, t) in to_gl.items() if t == v and v > 1))
    f.write("static const unsigned int known[%d] = {\n" % len(known))
    for i in range(0, len(known), 8):
        f.write("\t" + ", ".join("0x%X" % v for v in known[i:i + 8]) + ",\n")
    f.write("};\n\n/* Whether value is an OpenGL constant (binary search). */\n")
    f.write("static int mgl_known(unsigned int value) {\n\tint low = 0, high = %d;\n" % (len(known) - 1))
    f.write("\twhile (low <= high) {\n\t\tint middle = (low + high) / 2;\n")
    f.write("\t\tif (known[middle] == value) return 1;\n\t\tif (known[middle] < value) low = middle + 1;\n")
    f.write("\t\telse high = middle - 1;\n\t}\n\treturn 0;\n}\n\n")
    f.write("unsigned int mgl_enum(unsigned int value) {\n\tswitch (value) {\n")
    for value in sorted(to_gl):
        if to_gl[value][1] != value:
            f.write("\tcase %d: return 0x%X; /* %s */\n" % (value, to_gl[value][1], to_gl[value][0]))
    # A value that is not one of MiniGL's private ones is passed on if it is
    # an OpenGL constant (a MiniGL 29 token already is one, and applications
    # use OpenGL's own for extensions); anything else is marked unknown, as
    # applications define values of their own for what MiniGL lacks (RTCW:
    # GL_CLIP_PLANE0 = 0x7A07), which MiniGL ignores.
    f.write("\t}\n\treturn value <= 1 || mgl_known(value) ? value : 0x%X | value;\n}\n\n" % QT_MGL_ONLY)
    f.write("unsigned int mgl_enum_back(unsigned int value) {\n\treturn value;\n}\n\n")
    # glClear's mask bits are already real OpenGL values in the 29 header
    # (GL_COLOR_BUFFER_BIT 0x4000, GL_DEPTH_BUFFER_BIT 0x100); MiniGL has no
    # GL_STENCIL_BUFFER_BIT or GL_ACCUM_BUFFER_BIT of its own to carry.
    f.write("unsigned int mgl_bits(unsigned int value) {\n\treturn value;\n}\n")

# --- The command buffer functions with plain C types ------------------------

ctype = {"GLenum": "unsigned int", "GLuint": "unsigned int", "GLbitfield": "unsigned int", "GLint": "int",
         "GLsizei": "int", "GLboolean": "unsigned char", "GLubyte": "unsigned char", "GLbyte": "signed char",
         "GLshort": "short", "GLushort": "unsigned short", "GLfloat": "float", "GLclampf": "float",
         "GLdouble": "double", "GLclampd": "double", "GLvoid": "void"}

def plain_type(t):
    return re.sub(r"\bGL\w+\b", lambda m: ctype.get(m.group(0), m.group(0)), t)

encoders = {}  # name without _gl -> (return type, [parameter types])
with open(os.path.join(out, "qgl.auto.h"), "w") as f:
    f.write("/* Generated by minigl.library/mglgen.py - do not edit */\n#ifndef QGL_AUTO_H\n#define QGL_AUTO_H\n\n")
    for name, value in sorted(gl.items()):
        f.write("#define Q%s 0x%X\n" % (name, value))
    f.write("\n")
    for line in open(os.path.join(gldir, "gldeclarations.auto.h")):
        m = re.match(r"^(\w+\*?)\s+_gl(\w+)\((.*)\);", line.strip())
        if not m:
            continue
        ret, name, params = m.groups()
        types = [] if params.strip() in ("", "void") else [re.sub(r"\s*\w+$", "", p.strip()) for p in params.split(",")]
        encoders[name] = (ret, types)
        f.write("%s _gl%s(%s);\n" % (plain_type(ret), name, ", ".join(plain_type(t) for t in types) or "void"))
    f.write("\nunsigned int mgl_enum(unsigned int value);\nunsigned int mgl_enum_back(unsigned int value);\n")
    f.write("unsigned int mgl_bits(unsigned int value);\n#define QT_MGL_ONLY 0x%X\n\n#endif\n" % QT_MGL_ONLY)

# --- The dispatch table -----------------------------------------------------

dispatch = open(os.path.join(sdk, "libraries", "minigl_dispatch.h"), encoding="latin-1").read()
struct = dispatch[dispatch.index("typedef struct MGLDispatchTable {"):dispatch.index("} MGLDispatchTable;")]
struct = re.sub(r"/\*.*?\*/", "", struct, flags=re.S)
# One entry per line: return type, (*name), parameters.
entries = re.findall(r"^[ \t]*([^\n(;{]+?)[ \t]*\(\*(\w+)\)[ \t]*\(([^\n]*)\);", struct, re.M)

implemented = set()
for path in handwritten:
    implemented |= set(re.findall(r"^\S.*\bmgl_(\w+)\s*\(", open(path, encoding="latin-1").read(), re.M))

# Scalar GL functions that still need hand-written code: enum values in int
# parameters, MiniGL-only capabilities, results to translate back.
manual = {"TexEnvi", "TexParameteri", "PixelStorei", "Fogf", "GetError", "IsEnabled", "TexImage2D",
          "LockArrays", "UnlockArrays", "ArrayElement", "DrawArrays", "DrawElements",
          "EnableClientState", "DisableClientState"}
# Pointers the host reads as they are (pixel data, with the swap byte modes
# set at context creation).
pointer_ok = {"ReadPixels", "TexSubImage2D"}

def split_params(params):
    params = params.strip()
    if params in ("", "void"):
        return []
    # Split at the commas outside parentheses (function pointer parameters).
    parts, depth, current = [], 0, ""
    for ch in params:
        if ch == "," and depth == 0:
            parts.append(current)
            current = ""
            continue
        depth += {"(": 1, ")": -1}.get(ch, 0)
        current += ch
    parts.append(current)
    result = []
    for p in parts:
        p = p.strip()
        pointer = re.search(r"\(\*(\w+)\)", p)
        if pointer:
            result.append((p, pointer.group(1)))
            continue
        m = re.match(r"^(.*?)(\w+)$", p)
        result.append((m.group(1).strip(), m.group(2)))
    return result

lines = ["/* Generated by minigl.library/mglgen.py from MiniGL's minigl_dispatch.h - do not edit */",
         "#define MINIGL_LIBRARY_BUILD", "#include <libraries/minigl_dispatch.h>", '#include "qgl.auto.h"', "",
         "void mgl_missing(const char *name);", "void mgl_unknown(const char *name);", "extern GLcontext mgl_current;", ""]
table = []
generated = manualcount = stubs = 0
for ret, name, params in entries:
    ret = ret.strip()
    plist = split_params(params)
    glname = name[2:] if name.startswith("GL") else None
    scalar = all("*" not in t for t, _ in plist)
    enc = encoders.get(glname) if glname else None
    can_generate = (enc is not None and name not in implemented and glname not in manual
                    and plist and plist[0][0] == "GLcontext" and len(enc[1]) == len(plist) - 1
                    and (scalar or glname in pointer_ok) and ret in ("void", "GLboolean", "GLuint"))
    signature = "%s %%s(%s)" % (ret, params.strip() or "void")
    if can_generate:
        # Enum arguments are translated first; a call with a constant
        # MiniGL does not know is skipped, as MiniGL skips it.
        args, checks = [], []
        for (t, n) in plist[1:]:
            if t == "GLenum":
                checks.append("unsigned int q_%s = mgl_enum(%s);" % (n, n))
                args.append("q_" + n)
            else:
                args.append("mgl_bits(%s)" % n if t == "GLbitfield" else "(void *) %s" % n if "*" in t else n)
        body = "%s_gl%s(%s);" % ("return " if ret != "void" else "", glname, ", ".join(args))
        if checks:
            unknown_test = " || ".join("(q_%s & QT_MGL_ONLY) == QT_MGL_ONLY" % n for (t, n) in plist[1:] if t == "GLenum")
            body = "\n\t".join(checks) + "\n\tif (%s) {\n\t\tmgl_unknown(\"%s\");\n\t\treturn%s;\n\t}\n\t" % (
                unknown_test, name, "" if ret == "void" else " 0") + body
        lines.append("static " + (signature % ("d_" + name)) + " {\n\t" + body + "\n}")
        table.append("\t.%s = d_%s," % (name, name))
        generated += 1
    elif name in implemented:
        lines.append(signature % ("mgl_" + name) + ";")
        table.append("\t.%s = mgl_%s," % (name, name))
        manualcount += 1
    else:
        body = 'mgl_missing("%s");%s' % (name, "" if ret == "void" else "\n\treturn 0;")
        lines.append("static " + (signature % ("stub_" + name)) + " {\n\t" + body + "\n}")
        table.append("\t.%s = stub_%s," % (name, name))
        stubs += 1

lines += ["", "const MGLDispatchTable mgl_dispatch = {", "\t.abiVersion = MINIGL_DISPATCH_ABI_VERSION,",
          "\t.structSize = sizeof(MGLDispatchTable),", "\t.backendFlags = MGL_QUARKTEX_BACKEND_FLAGS,",
          "\t.currentContext = &mgl_current,"] + table + ["};", ""]
with open(os.path.join(out, "dispatch.auto.c"), "w") as f:
    f.write("\n".join(lines))
print("mglgen: %d enum values (%d without an OpenGL value), %d entries: %d generated, %d hand-written, %d stubs"
      % (len(to_gl), len(unknown), len(entries), generated, manualcount, stubs))
