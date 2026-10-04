GLvoid _glAccum(GLenum op, GLfloat value) {
	ULONG qt_r[12] = {(ULONG) op, qt_f2l(value)};
	qt_call(qtfn_Accum, qt_r);
}
GLvoid _glAlphaFunc(GLenum func, GLclampf ref) {
	ULONG qt_r[12] = {(ULONG) func, qt_f2l(ref)};
	qt_call(qtfn_AlphaFunc, qt_r);
}
GLboolean _glAreTexturesResident(GLsizei n, GLuint* textures, GLboolean* residences) {
	ULONG qt_r[12] = {(ULONG) n, (ULONG) textures, (ULONG) residences};
	return (GLboolean) qt_call(qtfn_AreTexturesResident, qt_r);
}
GLvoid _glArrayElement(GLint i) {
	ULONG qt_r[12] = {(ULONG) i};
	qt_call(qtfn_ArrayElement, qt_r);
}
GLvoid _glBegin(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_Begin, qt_r);
}
GLvoid _glBindTexture(GLenum target, GLuint texture) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) texture};
	qt_call(qtfn_BindTexture, qt_r);
}
GLvoid _glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, GLubyte* bitmap) {
	ULONG qt_r[12] = {(ULONG) width, (ULONG) height, qt_f2l(xorig), qt_f2l(yorig), qt_f2l(xmove), qt_f2l(ymove), (ULONG) bitmap};
	qt_call(qtfn_Bitmap, qt_r);
}
GLvoid _glBlendFunc(GLenum sfactor, GLenum dfactor) {
	ULONG qt_r[12] = {(ULONG) sfactor, (ULONG) dfactor};
	qt_call(qtfn_BlendFunc, qt_r);
}
GLvoid _glCallList(GLuint list) {
	ULONG qt_r[12] = {(ULONG) list};
	qt_call(qtfn_CallList, qt_r);
}
GLvoid _glCallLists(GLsizei n, GLenum type, GLvoid* lists) {
	ULONG qt_r[12] = {(ULONG) n, (ULONG) type, (ULONG) lists};
	qt_call(qtfn_CallLists, qt_r);
}
GLvoid _glClear(GLbitfield mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_Clear, qt_r);
}
GLvoid _glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	ULONG qt_r[12] = {qt_f2l(red), qt_f2l(green), qt_f2l(blue), qt_f2l(alpha)};
	qt_call(qtfn_ClearAccum, qt_r);
}
GLvoid _glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	ULONG qt_r[12] = {qt_f2l(red), qt_f2l(green), qt_f2l(blue), qt_f2l(alpha)};
	qt_call(qtfn_ClearColor, qt_r);
}
GLvoid _glClearDepth(GLclampd depth) {
	ULONG qt_r[12] = {qt_dlo(depth), qt_dhi(depth)};
	qt_call(qtfn_ClearDepth, qt_r);
}
GLvoid _glClearIndex(GLfloat c) {
	ULONG qt_r[12] = {qt_f2l(c)};
	qt_call(qtfn_ClearIndex, qt_r);
}
GLvoid _glClearStencil(GLint s) {
	ULONG qt_r[12] = {(ULONG) s};
	qt_call(qtfn_ClearStencil, qt_r);
}
GLvoid _glClipPlane(GLenum plane, GLdouble* equation) {
	ULONG qt_r[12] = {(ULONG) plane, (ULONG) equation};
	qt_call(qtfn_ClipPlane, qt_r);
}
GLvoid _glColor3b(GLbyte red, GLbyte green, GLbyte blue) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue};
	qt_call(qtfn_Color3b, qt_r);
}
GLvoid _glColor3bv(GLbyte* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3bv, qt_r);
}
GLvoid _glColor3d(GLdouble red, GLdouble green, GLdouble blue) {
	ULONG qt_r[12] = {qt_dlo(red), qt_dhi(red), qt_dlo(green), qt_dhi(green), qt_dlo(blue), qt_dhi(blue)};
	qt_call(qtfn_Color3d, qt_r);
}
GLvoid _glColor3dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3dv, qt_r);
}
GLvoid _glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
	ULONG qt_r[12] = {qt_f2l(red), qt_f2l(green), qt_f2l(blue)};
	qt_call(qtfn_Color3f, qt_r);
}
GLvoid _glColor3fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3fv, qt_r);
}
GLvoid _glColor3i(GLint red, GLint green, GLint blue) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue};
	qt_call(qtfn_Color3i, qt_r);
}
GLvoid _glColor3iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3iv, qt_r);
}
GLvoid _glColor3s(GLshort red, GLshort green, GLshort blue) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue};
	qt_call(qtfn_Color3s, qt_r);
}
GLvoid _glColor3sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3sv, qt_r);
}
GLvoid _glColor3ub(GLubyte red, GLubyte green, GLubyte blue) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue};
	qt_call(qtfn_Color3ub, qt_r);
}
GLvoid _glColor3ubv(GLubyte* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3ubv, qt_r);
}
GLvoid _glColor3ui(GLuint red, GLuint green, GLuint blue) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue};
	qt_call(qtfn_Color3ui, qt_r);
}
GLvoid _glColor3uiv(GLuint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3uiv, qt_r);
}
GLvoid _glColor3us(GLushort red, GLushort green, GLushort blue) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue};
	qt_call(qtfn_Color3us, qt_r);
}
GLvoid _glColor3usv(GLushort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color3usv, qt_r);
}
GLvoid _glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_Color4b, qt_r);
}
GLvoid _glColor4bv(GLbyte* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4bv, qt_r);
}
GLvoid _glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) {
	ULONG qt_r[12] = {qt_dlo(red), qt_dhi(red), qt_dlo(green), qt_dhi(green), qt_dlo(blue), qt_dhi(blue), qt_dlo(alpha), qt_dhi(alpha)};
	qt_call(qtfn_Color4d, qt_r);
}
GLvoid _glColor4dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4dv, qt_r);
}
GLvoid _glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	ULONG qt_r[12] = {qt_f2l(red), qt_f2l(green), qt_f2l(blue), qt_f2l(alpha)};
	qt_call(qtfn_Color4f, qt_r);
}
GLvoid _glColor4fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4fv, qt_r);
}
GLvoid _glColor4i(GLint red, GLint green, GLint blue, GLint alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_Color4i, qt_r);
}
GLvoid _glColor4iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4iv, qt_r);
}
GLvoid _glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_Color4s, qt_r);
}
GLvoid _glColor4sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4sv, qt_r);
}
GLvoid _glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_Color4ub, qt_r);
}
GLvoid _glColor4ubv(GLubyte* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4ubv, qt_r);
}
GLvoid _glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_Color4ui, qt_r);
}
GLvoid _glColor4uiv(GLuint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4uiv, qt_r);
}
GLvoid _glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_Color4us, qt_r);
}
GLvoid _glColor4usv(GLushort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Color4usv, qt_r);
}
GLvoid _glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	ULONG qt_r[12] = {(ULONG) red, (ULONG) green, (ULONG) blue, (ULONG) alpha};
	qt_call(qtfn_ColorMask, qt_r);
}
GLvoid _glColorMaterial(GLenum face, GLenum mode) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) mode};
	qt_call(qtfn_ColorMaterial, qt_r);
}
GLvoid _glColorPointer(GLint size, GLenum type, GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) size, (ULONG) type, (ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_ColorPointer, qt_r);
}
GLvoid _glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) width, (ULONG) height, (ULONG) type};
	qt_call(qtfn_CopyPixels, qt_r);
}
GLvoid _glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) internalFormat, (ULONG) x, (ULONG) y, (ULONG) width, (ULONG) border};
	qt_call(qtfn_CopyTexImage1D, qt_r);
}
GLvoid _glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) internalFormat, (ULONG) x, (ULONG) y, (ULONG) width, (ULONG) height, (ULONG) border};
	qt_call(qtfn_CopyTexImage2D, qt_r);
}
GLvoid _glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) xoffset, (ULONG) x, (ULONG) y, (ULONG) width};
	qt_call(qtfn_CopyTexSubImage1D, qt_r);
}
GLvoid _glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) xoffset, (ULONG) yoffset, (ULONG) x, (ULONG) y, (ULONG) width, (ULONG) height};
	qt_call(qtfn_CopyTexSubImage2D, qt_r);
}
GLvoid _glCullFace(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_CullFace, qt_r);
}
GLvoid _glDeleteLists(GLuint list, GLsizei range) {
	ULONG qt_r[12] = {(ULONG) list, (ULONG) range};
	qt_call(qtfn_DeleteLists, qt_r);
}
GLvoid _glDeleteTextures(GLsizei n, GLuint* textures) {
	ULONG qt_r[12] = {(ULONG) n, (ULONG) textures};
	qt_call(qtfn_DeleteTextures, qt_r);
}
GLvoid _glDepthFunc(GLenum func) {
	ULONG qt_r[12] = {(ULONG) func};
	qt_call(qtfn_DepthFunc, qt_r);
}
GLvoid _glDepthMask(GLboolean flag) {
	ULONG qt_r[12] = {(ULONG) flag};
	qt_call(qtfn_DepthMask, qt_r);
}
GLvoid _glDepthRange(GLclampd zNear, GLclampd zFar) {
	ULONG qt_r[12] = {qt_dlo(zNear), qt_dhi(zNear), qt_dlo(zFar), qt_dhi(zFar)};
	qt_call(qtfn_DepthRange, qt_r);
}
GLvoid _glDisable(GLenum cap) {
	ULONG qt_r[12] = {(ULONG) cap};
	qt_call(qtfn_Disable, qt_r);
}
GLvoid _glDisableClientState(GLenum array) {
	ULONG qt_r[12] = {(ULONG) array};
	qt_call(qtfn_DisableClientState, qt_r);
}
GLvoid _glDrawArrays(GLenum mode, GLint first, GLsizei count) {
	ULONG qt_r[12] = {(ULONG) mode, (ULONG) first, (ULONG) count};
	qt_call(qtfn_DrawArrays, qt_r);
}
GLvoid _glDrawBuffer(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_DrawBuffer, qt_r);
}
GLvoid _glDrawElements(GLenum mode, GLsizei count, GLenum type, GLvoid* indices) {
	ULONG qt_r[12] = {(ULONG) mode, (ULONG) count, (ULONG) type, (ULONG) indices};
	qt_call(qtfn_DrawElements, qt_r);
}
GLvoid _glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) width, (ULONG) height, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_DrawPixels, qt_r);
}
GLvoid _glEdgeFlag(GLboolean flag) {
	ULONG qt_r[12] = {(ULONG) flag};
	qt_call(qtfn_EdgeFlag, qt_r);
}
GLvoid _glEdgeFlagPointer(GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_EdgeFlagPointer, qt_r);
}
GLvoid _glEdgeFlagv(GLboolean* flag) {
	ULONG qt_r[12] = {(ULONG) flag};
	qt_call(qtfn_EdgeFlagv, qt_r);
}
GLvoid _glEnable(GLenum cap) {
	ULONG qt_r[12] = {(ULONG) cap};
	qt_call(qtfn_Enable, qt_r);
}
GLvoid _glEnableClientState(GLenum array) {
	ULONG qt_r[12] = {(ULONG) array};
	qt_call(qtfn_EnableClientState, qt_r);
}
GLvoid _glEnd(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_End, qt_r);
}
GLvoid _glEndList(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_EndList, qt_r);
}
GLvoid _glEvalCoord1d(GLdouble u) {
	ULONG qt_r[12] = {qt_dlo(u), qt_dhi(u)};
	qt_call(qtfn_EvalCoord1d, qt_r);
}
GLvoid _glEvalCoord1dv(GLdouble* u) {
	ULONG qt_r[12] = {(ULONG) u};
	qt_call(qtfn_EvalCoord1dv, qt_r);
}
GLvoid _glEvalCoord1f(GLfloat u) {
	ULONG qt_r[12] = {qt_f2l(u)};
	qt_call(qtfn_EvalCoord1f, qt_r);
}
GLvoid _glEvalCoord1fv(GLfloat* u) {
	ULONG qt_r[12] = {(ULONG) u};
	qt_call(qtfn_EvalCoord1fv, qt_r);
}
GLvoid _glEvalCoord2d(GLdouble u, GLdouble v) {
	ULONG qt_r[12] = {qt_dlo(u), qt_dhi(u), qt_dlo(v), qt_dhi(v)};
	qt_call(qtfn_EvalCoord2d, qt_r);
}
GLvoid _glEvalCoord2dv(GLdouble* u) {
	ULONG qt_r[12] = {(ULONG) u};
	qt_call(qtfn_EvalCoord2dv, qt_r);
}
GLvoid _glEvalCoord2f(GLfloat u, GLfloat v) {
	ULONG qt_r[12] = {qt_f2l(u), qt_f2l(v)};
	qt_call(qtfn_EvalCoord2f, qt_r);
}
GLvoid _glEvalCoord2fv(GLfloat* u) {
	ULONG qt_r[12] = {(ULONG) u};
	qt_call(qtfn_EvalCoord2fv, qt_r);
}
GLvoid _glEvalMesh1(GLenum mode, GLint i1, GLint i2) {
	ULONG qt_r[12] = {(ULONG) mode, (ULONG) i1, (ULONG) i2};
	qt_call(qtfn_EvalMesh1, qt_r);
}
GLvoid _glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {
	ULONG qt_r[12] = {(ULONG) mode, (ULONG) i1, (ULONG) i2, (ULONG) j1, (ULONG) j2};
	qt_call(qtfn_EvalMesh2, qt_r);
}
GLvoid _glEvalPoint1(GLint i) {
	ULONG qt_r[12] = {(ULONG) i};
	qt_call(qtfn_EvalPoint1, qt_r);
}
GLvoid _glEvalPoint2(GLint i, GLint j) {
	ULONG qt_r[12] = {(ULONG) i, (ULONG) j};
	qt_call(qtfn_EvalPoint2, qt_r);
}
GLvoid _glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer) {
	ULONG qt_r[12] = {(ULONG) size, (ULONG) type, (ULONG) buffer};
	qt_call(qtfn_FeedbackBuffer, qt_r);
}
GLvoid _glFinish(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_Finish, qt_r);
}
GLvoid _glFlush(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_Flush, qt_r);
}
GLvoid _glFogf(GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_Fogf, qt_r);
}
GLvoid _glFogfv(GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_Fogfv, qt_r);
}
GLvoid _glFogi(GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) param};
	qt_call(qtfn_Fogi, qt_r);
}
GLvoid _glFogiv(GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_Fogiv, qt_r);
}
GLvoid _glFrontFace(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_FrontFace, qt_r);
}
GLvoid _glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
	ULONG qt_r[12] = {qt_dlo(left), qt_dhi(left), qt_dlo(right), qt_dhi(right), qt_dlo(bottom), qt_dhi(bottom), qt_dlo(top), qt_dhi(top), qt_dlo(zNear), qt_dhi(zNear), qt_dlo(zFar), qt_dhi(zFar)};
	qt_call(qtfn_Frustum, qt_r);
}
GLuint _glGenLists(GLsizei range) {
	ULONG qt_r[12] = {(ULONG) range};
	return (GLuint) qt_call(qtfn_GenLists, qt_r);
}
GLvoid _glGenTextures(GLsizei n, GLuint* textures) {
	ULONG qt_r[12] = {(ULONG) n, (ULONG) textures};
	qt_call(qtfn_GenTextures, qt_r);
}
GLvoid _glGetBooleanv(GLenum pname, GLboolean* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetBooleanv, qt_r);
}
GLvoid _glGetClipPlane(GLenum plane, GLdouble* equation) {
	ULONG qt_r[12] = {(ULONG) plane, (ULONG) equation};
	qt_call(qtfn_GetClipPlane, qt_r);
}
GLvoid _glGetDoublev(GLenum pname, GLdouble* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetDoublev, qt_r);
}
GLenum _glGetError(void) {
	ULONG qt_r[12] = {};
	return (GLenum) qt_call(qtfn_GetError, qt_r);
}
GLvoid _glGetFloatv(GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetFloatv, qt_r);
}
GLvoid _glGetIntegerv(GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetIntegerv, qt_r);
}
GLvoid _glGetLightfv(GLenum light, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) light, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetLightfv, qt_r);
}
GLvoid _glGetLightiv(GLenum light, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) light, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetLightiv, qt_r);
}
GLvoid _glGetMapdv(GLenum target, GLenum query, GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) query, (ULONG) v};
	qt_call(qtfn_GetMapdv, qt_r);
}
GLvoid _glGetMapfv(GLenum target, GLenum query, GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) query, (ULONG) v};
	qt_call(qtfn_GetMapfv, qt_r);
}
GLvoid _glGetMapiv(GLenum target, GLenum query, GLint* v) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) query, (ULONG) v};
	qt_call(qtfn_GetMapiv, qt_r);
}
GLvoid _glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetMaterialfv, qt_r);
}
GLvoid _glGetMaterialiv(GLenum face, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetMaterialiv, qt_r);
}
GLvoid _glGetPixelMapfv(GLenum map, GLfloat* values) {
	ULONG qt_r[12] = {(ULONG) map, (ULONG) values};
	qt_call(qtfn_GetPixelMapfv, qt_r);
}
GLvoid _glGetPixelMapuiv(GLenum map, GLuint* values) {
	ULONG qt_r[12] = {(ULONG) map, (ULONG) values};
	qt_call(qtfn_GetPixelMapuiv, qt_r);
}
GLvoid _glGetPixelMapusv(GLenum map, GLushort* values) {
	ULONG qt_r[12] = {(ULONG) map, (ULONG) values};
	qt_call(qtfn_GetPixelMapusv, qt_r);
}
GLvoid _glGetPointerv(GLenum pname, GLvoid** params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetPointerv, qt_r);
}
GLvoid _glGetPolygonStipple(GLubyte* mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_GetPolygonStipple, qt_r);
}
GLubyte* _glGetString(GLenum name) {
	ULONG qt_r[12] = {(ULONG) name};
	return (GLubyte*) qt_call(qtfn_GetString, qt_r);
}
GLvoid _glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexEnvfv, qt_r);
}
GLvoid _glGetTexEnviv(GLenum target, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexEnviv, qt_r);
}
GLvoid _glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexGendv, qt_r);
}
GLvoid _glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexGenfv, qt_r);
}
GLvoid _glGetTexGeniv(GLenum coord, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexGeniv, qt_r);
}
GLvoid _glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_GetTexImage, qt_r);
}
GLvoid _glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexLevelParameterfv, qt_r);
}
GLvoid _glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexLevelParameteriv, qt_r);
}
GLvoid _glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexParameterfv, qt_r);
}
GLvoid _glGetTexParameteriv(GLenum target, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_GetTexParameteriv, qt_r);
}
GLvoid _glHint(GLenum target, GLenum mode) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) mode};
	qt_call(qtfn_Hint, qt_r);
}
GLvoid _glIndexMask(GLuint mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_IndexMask, qt_r);
}
GLvoid _glIndexPointer(GLenum type, GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) type, (ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_IndexPointer, qt_r);
}
GLvoid _glIndexd(GLdouble c) {
	ULONG qt_r[12] = {qt_dlo(c), qt_dhi(c)};
	qt_call(qtfn_Indexd, qt_r);
}
GLvoid _glIndexdv(GLdouble* c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexdv, qt_r);
}
GLvoid _glIndexf(GLfloat c) {
	ULONG qt_r[12] = {qt_f2l(c)};
	qt_call(qtfn_Indexf, qt_r);
}
GLvoid _glIndexfv(GLfloat* c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexfv, qt_r);
}
GLvoid _glIndexi(GLint c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexi, qt_r);
}
GLvoid _glIndexiv(GLint* c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexiv, qt_r);
}
GLvoid _glIndexs(GLshort c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexs, qt_r);
}
GLvoid _glIndexsv(GLshort* c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexsv, qt_r);
}
GLvoid _glIndexub(GLubyte c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexub, qt_r);
}
GLvoid _glIndexubv(GLubyte* c) {
	ULONG qt_r[12] = {(ULONG) c};
	qt_call(qtfn_Indexubv, qt_r);
}
GLvoid _glInitNames(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_InitNames, qt_r);
}
GLvoid _glInterleavedArrays(GLenum format, GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) format, (ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_InterleavedArrays, qt_r);
}
GLboolean _glIsEnabled(GLenum cap) {
	ULONG qt_r[12] = {(ULONG) cap};
	return (GLboolean) qt_call(qtfn_IsEnabled, qt_r);
}
GLboolean _glIsList(GLuint list) {
	ULONG qt_r[12] = {(ULONG) list};
	return (GLboolean) qt_call(qtfn_IsList, qt_r);
}
GLboolean _glIsTexture(GLuint texture) {
	ULONG qt_r[12] = {(ULONG) texture};
	return (GLboolean) qt_call(qtfn_IsTexture, qt_r);
}
GLvoid _glLightModelf(GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_LightModelf, qt_r);
}
GLvoid _glLightModelfv(GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_LightModelfv, qt_r);
}
GLvoid _glLightModeli(GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) param};
	qt_call(qtfn_LightModeli, qt_r);
}
GLvoid _glLightModeliv(GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) params};
	qt_call(qtfn_LightModeliv, qt_r);
}
GLvoid _glLightf(GLenum light, GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) light, (ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_Lightf, qt_r);
}
GLvoid _glLightfv(GLenum light, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) light, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_Lightfv, qt_r);
}
GLvoid _glLighti(GLenum light, GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) light, (ULONG) pname, (ULONG) param};
	qt_call(qtfn_Lighti, qt_r);
}
GLvoid _glLightiv(GLenum light, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) light, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_Lightiv, qt_r);
}
GLvoid _glLineStipple(GLint factor, GLushort pattern) {
	ULONG qt_r[12] = {(ULONG) factor, (ULONG) pattern};
	qt_call(qtfn_LineStipple, qt_r);
}
GLvoid _glLineWidth(GLfloat width) {
	ULONG qt_r[12] = {qt_f2l(width)};
	qt_call(qtfn_LineWidth, qt_r);
}
GLvoid _glListBase(GLuint base) {
	ULONG qt_r[12] = {(ULONG) base};
	qt_call(qtfn_ListBase, qt_r);
}
GLvoid _glLoadIdentity(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_LoadIdentity, qt_r);
}
GLvoid _glLoadMatrixd(GLdouble* m) {
	ULONG qt_r[12] = {(ULONG) m};
	qt_call(qtfn_LoadMatrixd, qt_r);
}
GLvoid _glLoadMatrixf(GLfloat* m) {
	ULONG qt_r[12] = {(ULONG) m};
	qt_call(qtfn_LoadMatrixf, qt_r);
}
GLvoid _glLoadName(GLuint name) {
	ULONG qt_r[12] = {(ULONG) name};
	qt_call(qtfn_LoadName, qt_r);
}
GLvoid _glLogicOp(GLenum opcode) {
	ULONG qt_r[12] = {(ULONG) opcode};
	qt_call(qtfn_LogicOp, qt_r);
}
GLvoid _glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, GLdouble* points) {
	ULONG qt_r[12] = {(ULONG) target, qt_dlo(u1), qt_dhi(u1), qt_dlo(u2), qt_dhi(u2), (ULONG) stride, (ULONG) order, (ULONG) points};
	qt_call(qtfn_Map1d, qt_r);
}
GLvoid _glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, GLfloat* points) {
	ULONG qt_r[12] = {(ULONG) target, qt_f2l(u1), qt_f2l(u2), (ULONG) stride, (ULONG) order, (ULONG) points};
	qt_call(qtfn_Map1f, qt_r);
}
GLvoid _glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble* points) {
	/* more than 12 register slots - not forwarded */
}
GLvoid _glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat* points) {
	ULONG qt_r[12] = {(ULONG) target, qt_f2l(u1), qt_f2l(u2), (ULONG) ustride, (ULONG) uorder, qt_f2l(v1), qt_f2l(v2), (ULONG) vstride, (ULONG) vorder, (ULONG) points};
	qt_call(qtfn_Map2f, qt_r);
}
GLvoid _glMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {
	ULONG qt_r[12] = {(ULONG) un, qt_dlo(u1), qt_dhi(u1), qt_dlo(u2), qt_dhi(u2)};
	qt_call(qtfn_MapGrid1d, qt_r);
}
GLvoid _glMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {
	ULONG qt_r[12] = {(ULONG) un, qt_f2l(u1), qt_f2l(u2)};
	qt_call(qtfn_MapGrid1f, qt_r);
}
GLvoid _glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {
	ULONG qt_r[12] = {(ULONG) un, qt_dlo(u1), qt_dhi(u1), qt_dlo(u2), qt_dhi(u2), (ULONG) vn, qt_dlo(v1), qt_dhi(v1), qt_dlo(v2), qt_dhi(v2)};
	qt_call(qtfn_MapGrid2d, qt_r);
}
GLvoid _glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) {
	ULONG qt_r[12] = {(ULONG) un, qt_f2l(u1), qt_f2l(u2), (ULONG) vn, qt_f2l(v1), qt_f2l(v2)};
	qt_call(qtfn_MapGrid2f, qt_r);
}
GLvoid _glMaterialf(GLenum face, GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_Materialf, qt_r);
}
GLvoid _glMaterialfv(GLenum face, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_Materialfv, qt_r);
}
GLvoid _glMateriali(GLenum face, GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) pname, (ULONG) param};
	qt_call(qtfn_Materiali, qt_r);
}
GLvoid _glMaterialiv(GLenum face, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_Materialiv, qt_r);
}
GLvoid _glMatrixMode(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_MatrixMode, qt_r);
}
GLvoid _glMultMatrixd(GLdouble* m) {
	ULONG qt_r[12] = {(ULONG) m};
	qt_call(qtfn_MultMatrixd, qt_r);
}
GLvoid _glMultMatrixf(GLfloat* m) {
	ULONG qt_r[12] = {(ULONG) m};
	qt_call(qtfn_MultMatrixf, qt_r);
}
GLvoid _glNewList(GLuint list, GLenum mode) {
	ULONG qt_r[12] = {(ULONG) list, (ULONG) mode};
	qt_call(qtfn_NewList, qt_r);
}
GLvoid _glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) {
	ULONG qt_r[12] = {(ULONG) nx, (ULONG) ny, (ULONG) nz};
	qt_call(qtfn_Normal3b, qt_r);
}
GLvoid _glNormal3bv(GLbyte* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Normal3bv, qt_r);
}
GLvoid _glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {
	ULONG qt_r[12] = {qt_dlo(nx), qt_dhi(nx), qt_dlo(ny), qt_dhi(ny), qt_dlo(nz), qt_dhi(nz)};
	qt_call(qtfn_Normal3d, qt_r);
}
GLvoid _glNormal3dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Normal3dv, qt_r);
}
GLvoid _glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
	ULONG qt_r[12] = {qt_f2l(nx), qt_f2l(ny), qt_f2l(nz)};
	qt_call(qtfn_Normal3f, qt_r);
}
GLvoid _glNormal3fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Normal3fv, qt_r);
}
GLvoid _glNormal3i(GLint nx, GLint ny, GLint nz) {
	ULONG qt_r[12] = {(ULONG) nx, (ULONG) ny, (ULONG) nz};
	qt_call(qtfn_Normal3i, qt_r);
}
GLvoid _glNormal3iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Normal3iv, qt_r);
}
GLvoid _glNormal3s(GLshort nx, GLshort ny, GLshort nz) {
	ULONG qt_r[12] = {(ULONG) nx, (ULONG) ny, (ULONG) nz};
	qt_call(qtfn_Normal3s, qt_r);
}
GLvoid _glNormal3sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Normal3sv, qt_r);
}
GLvoid _glNormalPointer(GLenum type, GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) type, (ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_NormalPointer, qt_r);
}
GLvoid _glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
	ULONG qt_r[12] = {qt_dlo(left), qt_dhi(left), qt_dlo(right), qt_dhi(right), qt_dlo(bottom), qt_dhi(bottom), qt_dlo(top), qt_dhi(top), qt_dlo(zNear), qt_dhi(zNear), qt_dlo(zFar), qt_dhi(zFar)};
	qt_call(qtfn_Ortho, qt_r);
}
GLvoid _glPassThrough(GLfloat token) {
	ULONG qt_r[12] = {qt_f2l(token)};
	qt_call(qtfn_PassThrough, qt_r);
}
GLvoid _glPixelMapfv(GLenum map, GLsizei mapsize, GLfloat* values) {
	ULONG qt_r[12] = {(ULONG) map, (ULONG) mapsize, (ULONG) values};
	qt_call(qtfn_PixelMapfv, qt_r);
}
GLvoid _glPixelMapuiv(GLenum map, GLsizei mapsize, GLuint* values) {
	ULONG qt_r[12] = {(ULONG) map, (ULONG) mapsize, (ULONG) values};
	qt_call(qtfn_PixelMapuiv, qt_r);
}
GLvoid _glPixelMapusv(GLenum map, GLsizei mapsize, GLushort* values) {
	ULONG qt_r[12] = {(ULONG) map, (ULONG) mapsize, (ULONG) values};
	qt_call(qtfn_PixelMapusv, qt_r);
}
GLvoid _glPixelStoref(GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_PixelStoref, qt_r);
}
GLvoid _glPixelStorei(GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) param};
	qt_call(qtfn_PixelStorei, qt_r);
}
GLvoid _glPixelTransferf(GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_PixelTransferf, qt_r);
}
GLvoid _glPixelTransferi(GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) pname, (ULONG) param};
	qt_call(qtfn_PixelTransferi, qt_r);
}
GLvoid _glPixelZoom(GLfloat xfactor, GLfloat yfactor) {
	ULONG qt_r[12] = {qt_f2l(xfactor), qt_f2l(yfactor)};
	qt_call(qtfn_PixelZoom, qt_r);
}
GLvoid _glPointSize(GLfloat size) {
	ULONG qt_r[12] = {qt_f2l(size)};
	qt_call(qtfn_PointSize, qt_r);
}
GLvoid _glPolygonMode(GLenum face, GLenum mode) {
	ULONG qt_r[12] = {(ULONG) face, (ULONG) mode};
	qt_call(qtfn_PolygonMode, qt_r);
}
GLvoid _glPolygonOffset(GLfloat factor, GLfloat units) {
	ULONG qt_r[12] = {qt_f2l(factor), qt_f2l(units)};
	qt_call(qtfn_PolygonOffset, qt_r);
}
GLvoid _glPolygonStipple(GLubyte* mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_PolygonStipple, qt_r);
}
GLvoid _glPopAttrib(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_PopAttrib, qt_r);
}
GLvoid _glPopClientAttrib(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_PopClientAttrib, qt_r);
}
GLvoid _glPopMatrix(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_PopMatrix, qt_r);
}
GLvoid _glPopName(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_PopName, qt_r);
}
GLvoid _glPrioritizeTextures(GLsizei n, GLuint* textures, GLclampf* priorities) {
	ULONG qt_r[12] = {(ULONG) n, (ULONG) textures, (ULONG) priorities};
	qt_call(qtfn_PrioritizeTextures, qt_r);
}
GLvoid _glPushAttrib(GLbitfield mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_PushAttrib, qt_r);
}
GLvoid _glPushClientAttrib(GLbitfield mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_PushClientAttrib, qt_r);
}
GLvoid _glPushMatrix(void) {
	ULONG qt_r[12] = {};
	qt_call(qtfn_PushMatrix, qt_r);
}
GLvoid _glPushName(GLuint name) {
	ULONG qt_r[12] = {(ULONG) name};
	qt_call(qtfn_PushName, qt_r);
}
GLvoid _glRasterPos2d(GLdouble x, GLdouble y) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y)};
	qt_call(qtfn_RasterPos2d, qt_r);
}
GLvoid _glRasterPos2dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos2dv, qt_r);
}
GLvoid _glRasterPos2f(GLfloat x, GLfloat y) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y)};
	qt_call(qtfn_RasterPos2f, qt_r);
}
GLvoid _glRasterPos2fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos2fv, qt_r);
}
GLvoid _glRasterPos2i(GLint x, GLint y) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y};
	qt_call(qtfn_RasterPos2i, qt_r);
}
GLvoid _glRasterPos2iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos2iv, qt_r);
}
GLvoid _glRasterPos2s(GLshort x, GLshort y) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y};
	qt_call(qtfn_RasterPos2s, qt_r);
}
GLvoid _glRasterPos2sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos2sv, qt_r);
}
GLvoid _glRasterPos3d(GLdouble x, GLdouble y, GLdouble z) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z)};
	qt_call(qtfn_RasterPos3d, qt_r);
}
GLvoid _glRasterPos3dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos3dv, qt_r);
}
GLvoid _glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y), qt_f2l(z)};
	qt_call(qtfn_RasterPos3f, qt_r);
}
GLvoid _glRasterPos3fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos3fv, qt_r);
}
GLvoid _glRasterPos3i(GLint x, GLint y, GLint z) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z};
	qt_call(qtfn_RasterPos3i, qt_r);
}
GLvoid _glRasterPos3iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos3iv, qt_r);
}
GLvoid _glRasterPos3s(GLshort x, GLshort y, GLshort z) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z};
	qt_call(qtfn_RasterPos3s, qt_r);
}
GLvoid _glRasterPos3sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos3sv, qt_r);
}
GLvoid _glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z), qt_dlo(w), qt_dhi(w)};
	qt_call(qtfn_RasterPos4d, qt_r);
}
GLvoid _glRasterPos4dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos4dv, qt_r);
}
GLvoid _glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y), qt_f2l(z), qt_f2l(w)};
	qt_call(qtfn_RasterPos4f, qt_r);
}
GLvoid _glRasterPos4fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos4fv, qt_r);
}
GLvoid _glRasterPos4i(GLint x, GLint y, GLint z, GLint w) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z, (ULONG) w};
	qt_call(qtfn_RasterPos4i, qt_r);
}
GLvoid _glRasterPos4iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos4iv, qt_r);
}
GLvoid _glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z, (ULONG) w};
	qt_call(qtfn_RasterPos4s, qt_r);
}
GLvoid _glRasterPos4sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_RasterPos4sv, qt_r);
}
GLvoid _glReadBuffer(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_ReadBuffer, qt_r);
}
GLvoid _glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) width, (ULONG) height, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_ReadPixels, qt_r);
}
GLvoid _glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {
	ULONG qt_r[12] = {qt_dlo(x1), qt_dhi(x1), qt_dlo(y1), qt_dhi(y1), qt_dlo(x2), qt_dhi(x2), qt_dlo(y2), qt_dhi(y2)};
	qt_call(qtfn_Rectd, qt_r);
}
GLvoid _glRectdv(GLdouble* v1, GLdouble* v2) {
	ULONG qt_r[12] = {(ULONG) v1, (ULONG) v2};
	qt_call(qtfn_Rectdv, qt_r);
}
GLvoid _glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	ULONG qt_r[12] = {qt_f2l(x1), qt_f2l(y1), qt_f2l(x2), qt_f2l(y2)};
	qt_call(qtfn_Rectf, qt_r);
}
GLvoid _glRectfv(GLfloat* v1, GLfloat* v2) {
	ULONG qt_r[12] = {(ULONG) v1, (ULONG) v2};
	qt_call(qtfn_Rectfv, qt_r);
}
GLvoid _glRecti(GLint x1, GLint y1, GLint x2, GLint y2) {
	ULONG qt_r[12] = {(ULONG) x1, (ULONG) y1, (ULONG) x2, (ULONG) y2};
	qt_call(qtfn_Recti, qt_r);
}
GLvoid _glRectiv(GLint* v1, GLint* v2) {
	ULONG qt_r[12] = {(ULONG) v1, (ULONG) v2};
	qt_call(qtfn_Rectiv, qt_r);
}
GLvoid _glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2) {
	ULONG qt_r[12] = {(ULONG) x1, (ULONG) y1, (ULONG) x2, (ULONG) y2};
	qt_call(qtfn_Rects, qt_r);
}
GLvoid _glRectsv(GLshort* v1, GLshort* v2) {
	ULONG qt_r[12] = {(ULONG) v1, (ULONG) v2};
	qt_call(qtfn_Rectsv, qt_r);
}
GLint _glRenderMode(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	return (GLint) qt_call(qtfn_RenderMode, qt_r);
}
GLvoid _glRotated(GLdouble ane, GLdouble x, GLdouble y, GLdouble z) {
	ULONG qt_r[12] = {qt_dlo(ane), qt_dhi(ane), qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z)};
	qt_call(qtfn_Rotated, qt_r);
}
GLvoid _glRotatef(GLfloat ane, GLfloat x, GLfloat y, GLfloat z) {
	ULONG qt_r[12] = {qt_f2l(ane), qt_f2l(x), qt_f2l(y), qt_f2l(z)};
	qt_call(qtfn_Rotatef, qt_r);
}
GLvoid _glScaled(GLdouble x, GLdouble y, GLdouble z) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z)};
	qt_call(qtfn_Scaled, qt_r);
}
GLvoid _glScalef(GLfloat x, GLfloat y, GLfloat z) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y), qt_f2l(z)};
	qt_call(qtfn_Scalef, qt_r);
}
GLvoid _glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) width, (ULONG) height};
	qt_call(qtfn_Scissor, qt_r);
}
GLvoid _glSelectBuffer(GLsizei size, GLuint* buffer) {
	ULONG qt_r[12] = {(ULONG) size, (ULONG) buffer};
	qt_call(qtfn_SelectBuffer, qt_r);
}
GLvoid _glShadeModel(GLenum mode) {
	ULONG qt_r[12] = {(ULONG) mode};
	qt_call(qtfn_ShadeModel, qt_r);
}
GLvoid _glStencilFunc(GLenum func, GLint ref, GLuint mask) {
	ULONG qt_r[12] = {(ULONG) func, (ULONG) ref, (ULONG) mask};
	qt_call(qtfn_StencilFunc, qt_r);
}
GLvoid _glStencilMask(GLuint mask) {
	ULONG qt_r[12] = {(ULONG) mask};
	qt_call(qtfn_StencilMask, qt_r);
}
GLvoid _glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
	ULONG qt_r[12] = {(ULONG) fail, (ULONG) zfail, (ULONG) zpass};
	qt_call(qtfn_StencilOp, qt_r);
}
GLvoid _glTexCoord1d(GLdouble s) {
	ULONG qt_r[12] = {qt_dlo(s), qt_dhi(s)};
	qt_call(qtfn_TexCoord1d, qt_r);
}
GLvoid _glTexCoord1dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord1dv, qt_r);
}
GLvoid _glTexCoord1f(GLfloat s) {
	ULONG qt_r[12] = {qt_f2l(s)};
	qt_call(qtfn_TexCoord1f, qt_r);
}
GLvoid _glTexCoord1fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord1fv, qt_r);
}
GLvoid _glTexCoord1i(GLint s) {
	ULONG qt_r[12] = {(ULONG) s};
	qt_call(qtfn_TexCoord1i, qt_r);
}
GLvoid _glTexCoord1iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord1iv, qt_r);
}
GLvoid _glTexCoord1s(GLshort s) {
	ULONG qt_r[12] = {(ULONG) s};
	qt_call(qtfn_TexCoord1s, qt_r);
}
GLvoid _glTexCoord1sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord1sv, qt_r);
}
GLvoid _glTexCoord2d(GLdouble s, GLdouble t) {
	ULONG qt_r[12] = {qt_dlo(s), qt_dhi(s), qt_dlo(t), qt_dhi(t)};
	qt_call(qtfn_TexCoord2d, qt_r);
}
GLvoid _glTexCoord2dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord2dv, qt_r);
}
GLvoid _glTexCoord2f(GLfloat s, GLfloat t) {
	ULONG qt_r[12] = {qt_f2l(s), qt_f2l(t)};
	qt_call(qtfn_TexCoord2f, qt_r);
}
GLvoid _glTexCoord2fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord2fv, qt_r);
}
GLvoid _glTexCoord2i(GLint s, GLint t) {
	ULONG qt_r[12] = {(ULONG) s, (ULONG) t};
	qt_call(qtfn_TexCoord2i, qt_r);
}
GLvoid _glTexCoord2iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord2iv, qt_r);
}
GLvoid _glTexCoord2s(GLshort s, GLshort t) {
	ULONG qt_r[12] = {(ULONG) s, (ULONG) t};
	qt_call(qtfn_TexCoord2s, qt_r);
}
GLvoid _glTexCoord2sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord2sv, qt_r);
}
GLvoid _glTexCoord3d(GLdouble s, GLdouble t, GLdouble r) {
	ULONG qt_r[12] = {qt_dlo(s), qt_dhi(s), qt_dlo(t), qt_dhi(t), qt_dlo(r), qt_dhi(r)};
	qt_call(qtfn_TexCoord3d, qt_r);
}
GLvoid _glTexCoord3dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord3dv, qt_r);
}
GLvoid _glTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {
	ULONG qt_r[12] = {qt_f2l(s), qt_f2l(t), qt_f2l(r)};
	qt_call(qtfn_TexCoord3f, qt_r);
}
GLvoid _glTexCoord3fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord3fv, qt_r);
}
GLvoid _glTexCoord3i(GLint s, GLint t, GLint r) {
	ULONG qt_r[12] = {(ULONG) s, (ULONG) t, (ULONG) r};
	qt_call(qtfn_TexCoord3i, qt_r);
}
GLvoid _glTexCoord3iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord3iv, qt_r);
}
GLvoid _glTexCoord3s(GLshort s, GLshort t, GLshort r) {
	ULONG qt_r[12] = {(ULONG) s, (ULONG) t, (ULONG) r};
	qt_call(qtfn_TexCoord3s, qt_r);
}
GLvoid _glTexCoord3sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord3sv, qt_r);
}
GLvoid _glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
	ULONG qt_r[12] = {qt_dlo(s), qt_dhi(s), qt_dlo(t), qt_dhi(t), qt_dlo(r), qt_dhi(r), qt_dlo(q), qt_dhi(q)};
	qt_call(qtfn_TexCoord4d, qt_r);
}
GLvoid _glTexCoord4dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord4dv, qt_r);
}
GLvoid _glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
	ULONG qt_r[12] = {qt_f2l(s), qt_f2l(t), qt_f2l(r), qt_f2l(q)};
	qt_call(qtfn_TexCoord4f, qt_r);
}
GLvoid _glTexCoord4fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord4fv, qt_r);
}
GLvoid _glTexCoord4i(GLint s, GLint t, GLint r, GLint q) {
	ULONG qt_r[12] = {(ULONG) s, (ULONG) t, (ULONG) r, (ULONG) q};
	qt_call(qtfn_TexCoord4i, qt_r);
}
GLvoid _glTexCoord4iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord4iv, qt_r);
}
GLvoid _glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) {
	ULONG qt_r[12] = {(ULONG) s, (ULONG) t, (ULONG) r, (ULONG) q};
	qt_call(qtfn_TexCoord4s, qt_r);
}
GLvoid _glTexCoord4sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_TexCoord4sv, qt_r);
}
GLvoid _glTexCoordPointer(GLint size, GLenum type, GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) size, (ULONG) type, (ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_TexCoordPointer, qt_r);
}
GLvoid _glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_TexEnvf, qt_r);
}
GLvoid _glTexEnvfv(GLenum target, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexEnvfv, qt_r);
}
GLvoid _glTexEnvi(GLenum target, GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) param};
	qt_call(qtfn_TexEnvi, qt_r);
}
GLvoid _glTexEnviv(GLenum target, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexEnviv, qt_r);
}
GLvoid _glTexGend(GLenum coord, GLenum pname, GLdouble param) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, qt_dlo(param), qt_dhi(param)};
	qt_call(qtfn_TexGend, qt_r);
}
GLvoid _glTexGendv(GLenum coord, GLenum pname, GLdouble* params) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexGendv, qt_r);
}
GLvoid _glTexGenf(GLenum coord, GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_TexGenf, qt_r);
}
GLvoid _glTexGenfv(GLenum coord, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexGenfv, qt_r);
}
GLvoid _glTexGeni(GLenum coord, GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) param};
	qt_call(qtfn_TexGeni, qt_r);
}
GLvoid _glTexGeniv(GLenum coord, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) coord, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexGeniv, qt_r);
}
GLvoid _glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) internalformat, (ULONG) width, (ULONG) border, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_TexImage1D, qt_r);
}
GLvoid _glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) internalformat, (ULONG) width, (ULONG) height, (ULONG) border, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_TexImage2D, qt_r);
}
GLvoid _glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, qt_f2l(param)};
	qt_call(qtfn_TexParameterf, qt_r);
}
GLvoid _glTexParameterfv(GLenum target, GLenum pname, GLfloat* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexParameterfv, qt_r);
}
GLvoid _glTexParameteri(GLenum target, GLenum pname, GLint param) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) param};
	qt_call(qtfn_TexParameteri, qt_r);
}
GLvoid _glTexParameteriv(GLenum target, GLenum pname, GLint* params) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) pname, (ULONG) params};
	qt_call(qtfn_TexParameteriv, qt_r);
}
GLvoid _glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) xoffset, (ULONG) width, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_TexSubImage1D, qt_r);
}
GLvoid _glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) {
	ULONG qt_r[12] = {(ULONG) target, (ULONG) level, (ULONG) xoffset, (ULONG) yoffset, (ULONG) width, (ULONG) height, (ULONG) format, (ULONG) type, (ULONG) pixels};
	qt_call(qtfn_TexSubImage2D, qt_r);
}
GLvoid _glTranslated(GLdouble x, GLdouble y, GLdouble z) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z)};
	qt_call(qtfn_Translated, qt_r);
}
GLvoid _glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y), qt_f2l(z)};
	qt_call(qtfn_Translatef, qt_r);
}
GLvoid _glVertex2d(GLdouble x, GLdouble y) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y)};
	qt_call(qtfn_Vertex2d, qt_r);
}
GLvoid _glVertex2dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex2dv, qt_r);
}
GLvoid _glVertex2f(GLfloat x, GLfloat y) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y)};
	qt_call(qtfn_Vertex2f, qt_r);
}
GLvoid _glVertex2fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex2fv, qt_r);
}
GLvoid _glVertex2i(GLint x, GLint y) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y};
	qt_call(qtfn_Vertex2i, qt_r);
}
GLvoid _glVertex2iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex2iv, qt_r);
}
GLvoid _glVertex2s(GLshort x, GLshort y) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y};
	qt_call(qtfn_Vertex2s, qt_r);
}
GLvoid _glVertex2sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex2sv, qt_r);
}
GLvoid _glVertex3d(GLdouble x, GLdouble y, GLdouble z) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z)};
	qt_call(qtfn_Vertex3d, qt_r);
}
GLvoid _glVertex3dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex3dv, qt_r);
}
GLvoid _glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y), qt_f2l(z)};
	qt_call(qtfn_Vertex3f, qt_r);
}
GLvoid _glVertex3fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex3fv, qt_r);
}
GLvoid _glVertex3i(GLint x, GLint y, GLint z) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z};
	qt_call(qtfn_Vertex3i, qt_r);
}
GLvoid _glVertex3iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex3iv, qt_r);
}
GLvoid _glVertex3s(GLshort x, GLshort y, GLshort z) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z};
	qt_call(qtfn_Vertex3s, qt_r);
}
GLvoid _glVertex3sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex3sv, qt_r);
}
GLvoid _glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
	ULONG qt_r[12] = {qt_dlo(x), qt_dhi(x), qt_dlo(y), qt_dhi(y), qt_dlo(z), qt_dhi(z), qt_dlo(w), qt_dhi(w)};
	qt_call(qtfn_Vertex4d, qt_r);
}
GLvoid _glVertex4dv(GLdouble* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex4dv, qt_r);
}
GLvoid _glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	ULONG qt_r[12] = {qt_f2l(x), qt_f2l(y), qt_f2l(z), qt_f2l(w)};
	qt_call(qtfn_Vertex4f, qt_r);
}
GLvoid _glVertex4fv(GLfloat* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex4fv, qt_r);
}
GLvoid _glVertex4i(GLint x, GLint y, GLint z, GLint w) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z, (ULONG) w};
	qt_call(qtfn_Vertex4i, qt_r);
}
GLvoid _glVertex4iv(GLint* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex4iv, qt_r);
}
GLvoid _glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) z, (ULONG) w};
	qt_call(qtfn_Vertex4s, qt_r);
}
GLvoid _glVertex4sv(GLshort* v) {
	ULONG qt_r[12] = {(ULONG) v};
	qt_call(qtfn_Vertex4sv, qt_r);
}
GLvoid _glVertexPointer(GLint size, GLenum type, GLsizei stride, GLvoid* pointer) {
	ULONG qt_r[12] = {(ULONG) size, (ULONG) type, (ULONG) stride, (ULONG) pointer};
	qt_call(qtfn_VertexPointer, qt_r);
}
GLvoid _glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
	ULONG qt_r[12] = {(ULONG) x, (ULONG) y, (ULONG) width, (ULONG) height};
	qt_call(qtfn_Viewport, qt_r);
}
