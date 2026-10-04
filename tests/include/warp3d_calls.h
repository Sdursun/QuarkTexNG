/*
 * Calls into Warp3D.library for the test programs, taken from
 * Warp3D.library/Warp3D.fd. Only the functions the tests use are listed.
 */
#ifndef QUARKTEX_WARP3D_CALLS_H
#define QUARKTEX_WARP3D_CALLS_H

#include <inline/macros.h>
#include "Warp3D.h"

extern struct Library *Warp3DBase;

#define W3D_CreateContext(error, tags) \
	LP2(30, W3D_Context *, W3D_CreateContext, ULONG *, error, a0, struct TagItem *, tags, a1, , Warp3DBase)
#define W3D_DestroyContext(context) \
	LP1NR(36, W3D_DestroyContext, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_SetState(context, state, action) \
	LP3(48, ULONG, W3D_SetState, W3D_Context *, context, a0, ULONG, state, d0, ULONG, action, d1, , Warp3DBase)
#define W3D_LockHardware(context) \
	LP1(60, ULONG, W3D_LockHardware, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_UnLockHardware(context) \
	LP1NR(66, W3D_UnLockHardware, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_AllocTexObj(context, error, tags) \
	LP3(96, W3D_Texture *, W3D_AllocTexObj, W3D_Context *, context, a0, ULONG *, error, a1, struct TagItem *, tags, a2, , Warp3DBase)
#define W3D_FreeTexObj(context, texture) \
	LP2NR(102, W3D_FreeTexObj, W3D_Context *, context, a0, W3D_Texture *, texture, a1, , Warp3DBase)
#define W3D_SetFilter(context, texture, min, mag) \
	LP4(120, ULONG, W3D_SetFilter, W3D_Context *, context, a0, W3D_Texture *, texture, a1, ULONG, min, d0, ULONG, mag, d1, , Warp3DBase)
#define W3D_SetTexEnv(context, texture, envparam, envcolor) \
	LP4(126, ULONG, W3D_SetTexEnv, W3D_Context *, context, a0, W3D_Texture *, texture, a1, ULONG, envparam, d1, W3D_Color *, envcolor, a2, , Warp3DBase)
#define W3D_SetWrapMode(context, texture, s_mode, t_mode, bordercolor) \
	LP5(132, ULONG, W3D_SetWrapMode, W3D_Context *, context, a0, W3D_Texture *, texture, a1, ULONG, s_mode, d0, ULONG, t_mode, d1, W3D_Color *, bordercolor, a2, , Warp3DBase)
#define W3D_UpdateTexImage(context, texture, teximage, level, palette) \
	LP5(138, ULONG, W3D_UpdateTexImage, W3D_Context *, context, a0, W3D_Texture *, texture, a1, void *, teximage, a2, int, level, d1, ULONG *, palette, a3, , Warp3DBase)
#define W3D_UpdateTexSubImage(context, texture, teximage, level, palette, scissor, srcbpr) \
	LP7(372, ULONG, W3D_UpdateTexSubImage, W3D_Context *, context, a0, W3D_Texture *, texture, a1, void *, teximage, a2, ULONG, level, d1, ULONG *, palette, a3, W3D_Scissor *, scissor, a4, ULONG, srcbpr, d0, , Warp3DBase)
#define W3D_FreeAllTexObj(context) \
	LP1(378, ULONG, W3D_FreeAllTexObj, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_ReadZPixel(context, x, y, z) \
	LP4(234, ULONG, W3D_ReadZPixel, W3D_Context *, context, a0, ULONG, x, d0, ULONG, y, d1, W3D_Double *, z, a1, , Warp3DBase)
#define W3D_ReadZSpan(context, x, y, n, z) \
	LP5(240, ULONG, W3D_ReadZSpan, W3D_Context *, context, a0, ULONG, x, d0, ULONG, y, d1, ULONG, n, d2, W3D_Double *, z, a1, , Warp3DBase)
#define W3D_WriteZPixel(context, x, y, z) \
	LP4NR(348, W3D_WriteZPixel, W3D_Context *, context, a0, ULONG, x, d0, ULONG, y, d1, W3D_Double *, z, a1, , Warp3DBase)
#define W3D_WriteZSpan(context, x, y, n, z, mask) \
	LP6NR(354, W3D_WriteZSpan, W3D_Context *, context, a0, ULONG, x, d0, ULONG, y, d1, ULONG, n, d2, W3D_Double *, z, a1, UBYTE *, mask, a2, , Warp3DBase)
#define W3D_DrawLine(context, line) \
	LP2(150, ULONG, W3D_DrawLine, W3D_Context *, context, a0, W3D_Line *, line, a1, , Warp3DBase)
#define W3D_DrawPoint(context, point) \
	LP2(156, ULONG, W3D_DrawPoint, W3D_Context *, context, a0, W3D_Point *, point, a1, , Warp3DBase)
#define W3D_DrawTriangle(context, triangle) \
	LP2(162, ULONG, W3D_DrawTriangle, W3D_Context *, context, a0, W3D_Triangle *, triangle, a1, , Warp3DBase)
#define W3D_DrawTriFan(context, triangles) \
	LP2(168, ULONG, W3D_DrawTriFan, W3D_Context *, context, a0, W3D_Triangles *, triangles, a1, , Warp3DBase)
#define W3D_DrawTriStrip(context, triangles) \
	LP2(174, ULONG, W3D_DrawTriStrip, W3D_Context *, context, a0, W3D_Triangles *, triangles, a1, , Warp3DBase)
#define W3D_SetAlphaMode(context, mode, refval) \
	LP3(180, ULONG, W3D_SetAlphaMode, W3D_Context *, context, a0, ULONG, mode, d1, W3D_Float *, refval, a1, , Warp3DBase)
#define W3D_SetBlendMode(context, srcfunc, dstfunc) \
	LP3(186, ULONG, W3D_SetBlendMode, W3D_Context *, context, a0, ULONG, srcfunc, d0, ULONG, dstfunc, d1, , Warp3DBase)
#define W3D_SetFogParams(context, fog, mode) \
	LP3(198, ULONG, W3D_SetFogParams, W3D_Context *, context, a0, W3D_Fog *, fog, a1, ULONG, mode, d1, , Warp3DBase)
#define W3D_AllocZBuffer(context) \
	LP1(216, ULONG, W3D_AllocZBuffer, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_FreeZBuffer(context) \
	LP1(222, ULONG, W3D_FreeZBuffer, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_ClearZBuffer(context, clearvalue) \
	LP2(228, ULONG, W3D_ClearZBuffer, W3D_Context *, context, a0, W3D_Double *, clearvalue, a1, , Warp3DBase)
#define W3D_SetZCompareMode(context, mode) \
	LP2(246, ULONG, W3D_SetZCompareMode, W3D_Context *, context, a0, ULONG, mode, d1, , Warp3DBase)
#define W3D_Flush(context) \
	LP1(312, ULONG, W3D_Flush, W3D_Context *, context, a0, , Warp3DBase)
#define W3D_DrawLineStrip(context, lines) \
	LP2(390, ULONG, W3D_DrawLineStrip, W3D_Context *, context, a0, W3D_Lines *, lines, a1, , Warp3DBase)
#define W3D_DrawLineLoop(context, lines) \
	LP2(396, ULONG, W3D_DrawLineLoop, W3D_Context *, context, a0, W3D_Lines *, lines, a1, , Warp3DBase)
#define W3D_ClearDrawRegion(context, color) \
	LP2(450, ULONG, W3D_ClearDrawRegion, W3D_Context *, context, a0, ULONG, color, d0, , Warp3DBase)
#define W3D_VertexPointer(context, pointer, stride, mode, flags) \
	LP5(492, ULONG, W3D_VertexPointer, W3D_Context *, context, a0, void *, pointer, a1, int, stride, d0, ULONG, mode, d1, ULONG, flags, d2, , Warp3DBase)
#define W3D_ColorPointer(context, pointer, stride, format, mode, flags) \
	LP6(504, ULONG, W3D_ColorPointer, W3D_Context *, context, a0, void *, pointer, a1, int, stride, d0, ULONG, format, d1, ULONG, mode, d2, ULONG, flags, d3, , Warp3DBase)
#define W3D_BindTexture(context, tmu, texture) \
	LP3(510, ULONG, W3D_BindTexture, W3D_Context *, context, a0, ULONG, tmu, d0, W3D_Texture *, texture, a1, , Warp3DBase)
#define W3D_DrawArray(context, primitive, base, count) \
	LP4(516, ULONG, W3D_DrawArray, W3D_Context *, context, a0, ULONG, primitive, d0, ULONG, base, d1, ULONG, count, d2, , Warp3DBase)

#endif
