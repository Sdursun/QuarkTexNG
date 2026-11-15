#!/usr/bin/env python3
"""
Generates ogles2.library's dispatch table from the PiStorm/Emu68 SDK's
<libraries/ogles2_dispatch.h> (docs/phase9-ogles2.md, stage 1).

Unlike minigl.library's table, GLES2's tokens are already OpenGL's own
values, so there is no enum translation here: this only has to place each
entry -- the six agl* ones and every hand-written GL one (HANDWRITTEN below,
implemented as ogles2_<Name> in context.c/glfuncs.c) -- and stub the rest
(logged once if a program calls one).

Each GL entry is a small wrapper that selects the host context of its
context argument before the hand-written ogles2_<Name>: the commands go to
the selected one, and a program may call the table with any of its
contexts, from any task, without aglMakeCurrent (the SDK's way of drawing
from a second task). When the context is the task's selected one already,
that is two compares (gl/gl.c's useContext).

usage: oglesgen.py [--ogles3] <SDK include dir> <out dir> <hand-written .c files...>

With --ogles3 it reads the ogles3 SDK's <libraries/ogles3_dispatch.h>
instead (ogles3.library: the same 164 entries first, then OpenGL ES 3.0/3.1's
and their extensions'; docs/phase10-ogles3.md); the entries ogles3.library
implements beyond ogles2's are HANDWRITTEN3, in ogles3.library/*.c.
"""
import os
import re
import sys

args = sys.argv[1:]
ogles3 = args[0] == "--ogles3"
if ogles3:
    args = args[1:]
sdk, out = args[0:2]
handwritten_files = args[2:]
api = "ogles3" if ogles3 else "ogles2"
dispatch = open(os.path.join(sdk, "libraries", api + "_dispatch.h"), encoding="latin-1").read()
start = dispatch.index("struct %sDispatch\n{" % api.upper())
struct = dispatch[start:dispatch.index("};", start)]
struct = re.sub(r"/\*.*?\*/", "", struct, flags=re.S)
# One entry per line: return type, (*name)(parameters).
entries = re.findall(r"^\s*([^\n(;{]+?)\s*\(\*(\w+)\)\s*\(([^\n]*)\);", struct, re.M)

# Implemented by hand (context.c for the agl* ones, glfuncs.c for the rest);
# everything else in the table is a logging stub until a later stage writes
# it. Keep in sync with the ogles2_<Name> functions those files define.
AGL = ["aglCreateContext2", "aglDestroyContext", "aglMakeCurrent", "aglSwapBuffers", "aglSetParams2", "aglSetBitmap"]
HANDWRITTEN = {
    # Stage 1: no pointer argument, forwarded to the existing command buffer.
    "GLGetError", "GLGetString", "GLClear", "GLClearColor", "GLClearDepthf", "GLClearStencil",
    "GLViewport", "GLFlush", "GLFinish", "GLEnable", "GLDisable", "GLColorMask", "GLDepthMask",
    "GLDepthFunc", "GLCullFace", "GLFrontFace", "GLIsEnabled", "GLHint", "GLLineWidth", "GLScissor",
    # Stage 2: shaders, programs, uniforms, buffers, vertex attributes and
    # drawing (gl/oglescmd.h, host/ogles2.cpp).
    "GLCreateShader", "GLDeleteShader", "GLCompileShader", "GLAttachShader", "GLDetachShader",
    "GLCreateProgram", "GLDeleteProgram", "GLLinkProgram", "GLUseProgram", "GLValidateProgram",
    "GLIsShader", "GLIsProgram", "GLShaderSource", "GLBindAttribLocation", "GLGetAttribLocation",
    "GLGetUniformLocation", "GLUniform1f", "GLUniform2f", "GLUniform3f", "GLUniform4f",
    "GLUniform1i", "GLUniform2i", "GLUniform3i", "GLUniform4i", "GLGetShaderiv", "GLGetProgramiv",
    "GLGetShaderInfoLog", "GLGetProgramInfoLog", "GLGenBuffers", "GLDeleteBuffers", "GLBindBuffer",
    "GLBufferData", "GLVertexAttribPointer", "GLEnableVertexAttribArray", "GLDisableVertexAttribArray",
    "GLDrawArrays", "GLDrawElements",
    # Textures and framebuffers (cube/boingocean).
    "GLGenTextures", "GLDeleteTextures", "GLBindTexture", "GLActiveTexture", "GLGenerateMipmap",
    "GLTexParameteri", "GLTexImage2D", "GLUniformMatrix4fv", "GLGenFramebuffers", "GLDeleteFramebuffers",
    "GLBindFramebuffer", "GLFramebufferTexture2D", "GLCheckFramebufferStatus", "GLGenRenderbuffers",
    "GLDeleteRenderbuffers", "GLBindRenderbuffer", "GLRenderbufferStorage", "GLFramebufferRenderbuffer",
    # Stage 3: the rest of GLES2 and the SDK's extensions.
    "GLBlendFunc", "GLStencilFunc", "GLStencilMask", "GLStencilOp", "GLPolygonOffset", "GLPixelStorei",
    "GLDepthRangef", "GLPolygonMode", "GLIsTexture", "GLTexParameterf", "GLTexParameterfv", "GLTexParameteriv",
    "GLCopyTexImage2D", "GLCopyTexSubImage2D", "GLBlendColor", "GLBlendEquation", "GLBlendEquationSeparate",
    "GLBlendFuncSeparate", "GLStencilFuncSeparate", "GLStencilMaskSeparate", "GLStencilOpSeparate",
    "GLSampleCoverage", "GLIsFramebuffer", "GLIsRenderbuffer", "GLIsBuffer", "GLProvokingVertex",
    "GLReleaseShaderCompiler", "GLDiscardFramebufferEXT", "GLInsertEventMarkerEXT", "GLPushGroupMarkerEXT",
    "GLPopGroupMarkerEXT", "GLGetShaderPrecisionFormat", "GLVertexAttrib1f", "GLVertexAttrib2f",
    "GLVertexAttrib3f", "GLVertexAttrib4f", "GLVertexAttrib1fv", "GLVertexAttrib2fv", "GLVertexAttrib3fv",
    "GLVertexAttrib4fv", "GLGetVertexAttribiv", "GLGetVertexAttribfv", "GLGetVertexAttribPointerv",
    "GLUniform1fv", "GLUniform2fv", "GLUniform3fv", "GLUniform4fv", "GLUniform1iv", "GLUniform2iv",
    "GLUniform3iv", "GLUniform4iv", "GLUniformMatrix2fv", "GLUniformMatrix3fv", "GLBufferSubData",
    "GLGetBufferParameteriv", "GLGetBufferParameterivOES", "GLMapBufferOES", "GLUnmapBufferOES",
    "GLGetBufferPointervOES", "GLGetTexParameteriv", "GLGetTexParameterfv", "GLGetRenderbufferParameteriv",
    "GLGetFramebufferAttachmentParameteriv", "GLGetUniformfv", "GLGetUniformiv", "GLGetIntegerv",
    "GLGetFloatv", "GLGetBooleanv", "GLGetActiveAttrib", "GLGetActiveUniform", "GLGetAttachedShaders",
    "GLGetShaderSource", "GLTexSubImage2D", "GLReadPixels", "GLDrawBuffersEXT", "GLDrawRangeElements",
    "GLDrawElementsBaseVertexOES",
    # Compressed textures (ETC1/ETC2/EAC, passed on); binaries refused (no
    # formats offered); glClipControlEXT where the host has OpenGL 4.5's.
    "GLCompressedTexImage2D", "GLCompressedTexSubImage2D", "GLShaderBinary", "GLGetProgramBinaryOES",
    "GLProgramBinaryOES", "GLClipControlEXT",
}
# ogles3.library's own (ogles3.library/*.c), on top of the above.
# ogles3.library's own: every ogles2_GL<Name> its files (ogles3.library/*.c,
# among the files given) define.
HANDWRITTEN3 = set()
for f in handwritten_files:
    if "ogles3.library" in f:
        HANDWRITTEN3 |= set("GL" + n for n in re.findall(r"^[^\s#/*][^\n(;]*\bogles2_GL(\w+)\(", open(f, encoding="utf-8").read(), re.M))
if ogles3:
    HANDWRITTEN |= HANDWRITTEN3

os.makedirs(out, exist_ok=True)
lines = ["/* Generated by ogles2.library/oglesgen.py from the SDK's %s_dispatch.h - do not edit */" % api,
         '#include "ogles2.h"', "", "void ogles2_missing(const char *name);", ""]
table = []
generated = handwritten = stubs = 0
for ret, name, params in entries:
    ret = ret.strip()
    signature = "%s %%s(%s)" % (ret, params.strip() or "void")
    if name in AGL:
        lines.append(signature % ("ogles2_" + name) + ";")
        table.append("\t.%s = ogles2_%s," % (name, name))
        generated += 1
    elif name in HANDWRITTEN:
        names = [re.search(r"(\w+)\s*$", p.strip()).group(1) for p in params.split(",")]
        assert names[0] == "ctx", name
        lines.append(signature % ("ogles2_" + name) + ";")
        call = "ogles2_%s(%s);" % (name, ", ".join(names))
        lines.append("static " + (signature % ("d_" + name)) + " {\n\tif (ctx) useContext(ctx->host);\n\t" +
                     ("" if ret == "void" else "return ") + call + "\n}")
        table.append("\t.%s = d_%s," % (name, name))
        handwritten += 1
    else:
        void_return = ret.strip() == "void"
        body = 'ogles2_missing("%s");%s' % (name, "" if void_return else "\n\treturn 0;")
        lines.append("static " + (signature % ("stub_" + name)) + " {\n\t" + body + "\n}")
        table.append("\t.%s = stub_%s," % (name, name))
        stubs += 1

lines += ["", "const struct OGLES2Dispatch ogles2_dispatch = {",
          "\t.abiVersion = OGLES2_DISPATCH_ABI_VERSION,", "\t.structSize = sizeof(struct OGLES2Dispatch),",
          "\t.backendFlags = 0,", "\t.reserved = 0,"] + table + ["};", ""]
with open(os.path.join(out, "dispatch.auto.c"), "w") as f:
    f.write("\n".join(lines))
print("%sgen: %d entries: %d context calls, %d hand-written, %d stubs"
      % (api, len(entries), generated, handwritten, stubs))
