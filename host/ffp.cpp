// OpenGL 1.1 fixed-function emulation on OpenGL 3.3 core, see ffp.h. The
// state below starts as OpenGL 1.1 defines it; the shader computes what
// OpenGL 1.1 computes for an RGBA texture (texture environment), linear,
// exp and exp2 fog with the absolute eye z as fog coordinate, and the alpha
// test, in that order.
#include "ffp.h"
#include <map>
#include <string>
#include <vector>
#include "gldecode.h"

namespace {
	struct Vertex {
		GLfloat position[4];
		GLfloat color[4];
		GLfloat texCoord[4];
	};

	const char* vertexShader =
		"#version 330 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec4 color;\n"
		"layout(location = 2) in vec4 texCoord;\n"
		"uniform mat4 modelView;\n"
		"out vec4 smoothColor;\n"
		"flat out vec4 flatColor;\n"
		"out vec4 coord;\n"
		"out float fogCoord;\n"
		"void main() {\n"
		"	vec4 eye = modelView * position;\n"
		"	gl_Position = eye; // the projection is the identity\n"
		"	smoothColor = clamp(color, 0.0, 1.0);\n"
		"	flatColor = smoothColor;\n"
		"	coord = texCoord;\n"
		"	fogCoord = abs(eye.z);\n"
		"}\n";

	// envMode: 0 replace, 1 modulate, 2 decal, 3 blend. fogMode: 0 linear,
	// 1 exp, 2 exp2; fogParams: start, end, density. alphaFunction: the
	// OpenGL comparison minus GL_NEVER.
	const char* fragmentShader =
		"#version 330 core\n"
		"in vec4 smoothColor;\n"
		"flat in vec4 flatColor;\n"
		"in vec4 coord;\n"
		"in float fogCoord;\n"
		"uniform bool smoothShading;\n"
		"uniform bool texturing;\n"
		"uniform sampler2D image;\n"
		"uniform int envMode;\n"
		"uniform vec4 envColor;\n"
		"uniform vec4 clampRange; // s, t low; s, t high\n"
		"uniform int chromaMode; // 0 off, 1 texels in the bounds pass, 2 they are rejected\n"
		"uniform vec3 chromaLower;\n"
		"uniform vec3 chromaUpper;\n"
		"uniform bool fogging;\n"
		"uniform int fogMode;\n"
		"uniform vec3 fogParams;\n"
		"uniform vec3 fogColor;\n"
		"uniform bool alphaTesting;\n"
		"uniform int alphaFunction;\n"
		"uniform float alphaReference;\n"
		"layout(location = 0) out vec4 fragColor;\n"
		"void main() {\n"
		"	vec4 c = smoothShading ? smoothColor : flatColor;\n"
		"	if (texturing) {\n"
		"		vec4 t = texture(image, clamp(coord.xy / coord.w, clampRange.xy, clampRange.zw));\n"
		"		if (chromaMode != 0) {\n"
		"			vec3 v = floor(t.rgb * 255.0 + 0.5);\n"
		"			bool inside = all(greaterThanEqual(v, chromaLower)) && all(lessThanEqual(v, chromaUpper));\n"
		"			if (inside == (chromaMode == 2)) discard;\n"
		"		}\n"
		"		if (envMode == 0) c = t;\n"
		"		else if (envMode == 1) c *= t;\n"
		"		else if (envMode == 2) c.rgb = mix(c.rgb, t.rgb, t.a);\n"
		"		else { c.rgb = mix(c.rgb, envColor.rgb, t.rgb); c.a *= t.a; }\n"
		"	}\n"
		"	if (fogging) {\n"
		"		float f;\n"
		"		if (fogMode == 0) f = (fogParams.y - fogCoord) / (fogParams.y - fogParams.x);\n"
		"		else if (fogMode == 1) f = exp(-fogParams.z * fogCoord);\n"
		"		else { float e = fogParams.z * fogCoord; f = exp(-e * e); }\n"
		"		c.rgb = mix(fogColor, c.rgb, clamp(f, 0.0, 1.0));\n"
		"	}\n"
		"	if (alphaTesting) {\n"
		"		float a = c.a, r = alphaReference;\n"
		"		bool pass;\n"
		"		if (alphaFunction == 0) pass = false;\n"
		"		else if (alphaFunction == 1) pass = a < r;\n"
		"		else if (alphaFunction == 2) pass = a == r;\n"
		"		else if (alphaFunction == 3) pass = a <= r;\n"
		"		else if (alphaFunction == 4) pass = a > r;\n"
		"		else if (alphaFunction == 5) pass = a != r;\n"
		"		else if (alphaFunction == 6) pass = a >= r;\n"
		"		else pass = true;\n"
		"		if (!pass) discard;\n"
		"	}\n"
		"	fragColor = c;\n"
		"}\n";

	// DepthPoints: position.z is the depth to write.
	const char* depthVertexShader =
		"#version 330 core\n"
		"layout(location = 0) in vec4 position;\n"
		"uniform mat4 modelView;\n"
		"flat out float depth;\n"
		"void main() {\n"
		"	gl_Position = modelView * vec4(position.xy, 0.0, 1.0);\n"
		"	depth = position.z;\n"
		"}\n";

	const char* depthFragmentShader =
		"#version 330 core\n"
		"flat in float depth;\n"
		"void main() {\n"
		"	gl_FragDepth = depth;\n"
		"}\n";

	struct Uniforms {
		GLint modelView, smoothShading, texturing, envMode, envColor, fogging, fogMode, fogParams, fogColor,
			alphaTesting, alphaFunction, alphaReference, clampRange, chromaMode, chromaLower, chromaUpper;
	};

	// GL_CLAMP, which OpenGL 3.3 does not have: the coordinate is clamped to
	// 0..1, and linear filtering at the edge mixes in the border colour. The
	// shader clamps (clampRange), the texture wraps GL_CLAMP_TO_BORDER. With
	// GL_NEAREST the clamp ends at the centre of the edge texel instead, as
	// GL_CLAMP never reaches the border there.
	struct TextureInfo {
		bool image;
		bool mipmapFilter; // never complete: QuarkTex makes no mipmaps
		bool clampS, clampT;
		bool magLinear;
		GLsizei width, height;
		int chromaMode; // 0 none, 1 texels in the bounds pass, 2 they are rejected
		int chromaLower[3], chromaUpper[3]; // r, g, b, 0..255
	};

	struct State {
		bool texture2D, fog, alphaTest, smooth;
		GLenum alphaFunction;
		GLfloat alphaReference;
		GLenum fogMode;
		GLfloat fogStart, fogEnd, fogDensity, fogColor[4];
		GLenum envMode;
		GLfloat envColor[4];
		bool chromaTest; // Warp3D's, see ChromaTest in ffp.h
	};

	GLuint program, depthProgram, vertexArray, vertexBuffer;
	Uniforms uniforms;
	GLfloat modelView[16];

	State state;
	bool dirty; // state differs from the uniforms
	bool texturingSent;
	GLfloat textureSent[11];
	GLfloat currentColor[4];
	GLfloat currentTexCoord[4];
	bool inside;
	GLenum primitive;
	std::vector<Vertex> vertices;
	GLuint bound;
	std::map<GLuint, TextureInfo> textures;

	GLuint compile(GLenum type, const char* source) {
		GLuint shader = gl3::CreateShader(type);
		gl3::ShaderSource(shader, 1, &source, 0);
		gl3::CompileShader(shader);
		GLint ok = 0, length = 0;
		gl3::GetShaderiv(shader, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			gl3::GetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			std::string log(length > 0 ? length : 1, '\0');
			gl3::GetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), 0, &log[0]);
			qt_report(("Warning: shader: " + log).c_str());
		}
		return shader;
	}

	GLuint link(const char* vertexSource, const char* fragmentSource) {
		GLuint vertex = compile(GL_VERTEX_SHADER, vertexSource), fragment = compile(GL_FRAGMENT_SHADER, fragmentSource);
		GLuint linked = gl3::CreateProgram();
		gl3::AttachShader(linked, vertex);
		gl3::AttachShader(linked, fragment);
		gl3::LinkProgram(linked);
		gl3::DeleteShader(vertex);
		gl3::DeleteShader(fragment);
		GLint ok = 0, length = 0;
		gl3::GetProgramiv(linked, GL_LINK_STATUS, &ok);
		if (ok) return linked;
		gl3::GetProgramiv(linked, GL_INFO_LOG_LENGTH, &length);
		std::string log(length > 0 ? length : 1, '\0');
		gl3::GetProgramInfoLog(linked, static_cast<GLsizei>(log.size()), 0, &log[0]);
		qt_report(("Warning: shader program: " + log).c_str());
		gl3::DeleteProgram(linked);
		return 0;
	}

	// The bound texture if texturing is on and it is complete, else 0.
	const TextureInfo* texturing() {
		if (!state.texture2D || !bound) return 0;
		std::map<GLuint, TextureInfo>::const_iterator info = textures.find(bound);
		if (info == textures.end() || !info->second.image || info->second.mipmapFilter) return 0;
		return &info->second;
	}

	void clampRange(bool clamp, bool linear, GLsizei size, GLfloat& low, GLfloat& high) {
		if (!clamp) {
			low = -1e30f;
			high = 1e30f;
		}
		else if (linear || size <= 0) {
			low = 0.0f;
			high = 1.0f;
		}
		else {
			low = 0.5f / static_cast<float>(size);
			high = 1.0f - low;
		}
	}

	void sendState() {
		const TextureInfo* info = texturing();
		bool texture = info != 0;
		// The uniforms that depend on the bound texture: clamp range, chroma
		// test mode and bounds.
		GLfloat values[11] = {-1e30f, -1e30f, 1e30f, 1e30f, 0, 0, 0, 0, 0, 0, 0};
		if (info) {
			clampRange(info->clampS, info->magLinear, info->width, values[0], values[2]);
			clampRange(info->clampT, info->magLinear, info->height, values[1], values[3]);
			if (state.chromaTest && info->chromaMode) {
				values[4] = static_cast<GLfloat>(info->chromaMode);
				for (int i = 0; i < 3; ++i) {
					values[5 + i] = static_cast<GLfloat>(info->chromaLower[i]);
					values[8 + i] = static_cast<GLfloat>(info->chromaUpper[i]);
				}
			}
		}
		if (memcmp(values, textureSent, sizeof(values)) != 0) {
			gl3::Uniform4f(uniforms.clampRange, values[0], values[1], values[2], values[3]);
			gl3::Uniform1i(uniforms.chromaMode, static_cast<GLint>(values[4]));
			gl3::Uniform3f(uniforms.chromaLower, values[5], values[6], values[7]);
			gl3::Uniform3f(uniforms.chromaUpper, values[8], values[9], values[10]);
			memcpy(textureSent, values, sizeof(values));
		}
		if (!dirty && texture == texturingSent) return;
		gl3::Uniform1i(uniforms.smoothShading, state.smooth);
		gl3::Uniform1i(uniforms.texturing, texture);
		gl3::Uniform1i(uniforms.envMode, state.envMode == GL_REPLACE ? 0 : state.envMode == GL_MODULATE ? 1 : state.envMode == GL_DECAL ? 2 : 3);
		gl3::Uniform4f(uniforms.envColor, state.envColor[0], state.envColor[1], state.envColor[2], state.envColor[3]);
		gl3::Uniform1i(uniforms.fogging, state.fog);
		gl3::Uniform1i(uniforms.fogMode, state.fogMode == GL_LINEAR ? 0 : state.fogMode == GL_EXP ? 1 : 2);
		gl3::Uniform3f(uniforms.fogParams, state.fogStart, state.fogEnd, state.fogDensity);
		gl3::Uniform3f(uniforms.fogColor, state.fogColor[0], state.fogColor[1], state.fogColor[2]);
		gl3::Uniform1i(uniforms.alphaTesting, state.alphaTest);
		gl3::Uniform1i(uniforms.alphaFunction, static_cast<GLint>(state.alphaFunction - GL_NEVER));
		gl3::Uniform1f(uniforms.alphaReference, state.alphaReference);
		dirty = false;
		texturingSent = texture;
	}

	void draw(GLenum mode, const Vertex* data, size_t count) {
		if (!count || !program) return;
		sendState();
		gl3::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(Vertex)), data, GL_STREAM_DRAW);
		glDrawArrays(mode, 0, static_cast<GLsizei>(count));
	}

	// Separate triangles, lines and points are collected while the state
	// stays the same and drawn with one call: every function that changes
	// state or reads the frame buffer calls flushBatch first.
	std::vector<Vertex> batch;
	GLenum batchMode;
	const size_t batchLimit = 65536;

	void flushBatch() {
		if (batch.empty()) return;
		draw(batchMode, &batch[0], batch.size());
		batch.clear();
	}

	// Vertices per primitive for the modes that can be batched, else 0.
	size_t independent(GLenum mode) {
		return mode == GL_TRIANGLES ? 3 : mode == GL_LINES ? 2 : mode == GL_POINTS ? 1 : 0;
	}

	void addVertex(GLfloat x, GLfloat y, GLfloat z) {
		if (!inside) return;
		Vertex v = {{x, y, z, 1.0f}, {currentColor[0], currentColor[1], currentColor[2], currentColor[3]},
			{currentTexCoord[0], currentTexCoord[1], currentTexCoord[2], currentTexCoord[3]}};
		vertices.push_back(v);
	}

	GLfloat clamp01(GLfloat value) {
		return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
	}

	// An OpenGL 1.1 pixel format for a texture with internal format GL_RGBA:
	// the format to upload, the internal format and the swizzle that give
	// the RGBA values OpenGL 1.1 stores. False for formats taken as they are.
	bool legacyFormat(GLenum format, GLenum& upload, GLint& internalFormat, GLint swizzle[4]) {
		switch (format) {
		case GL_ALPHA:
			upload = GL_RED; internalFormat = GL_R8;
			swizzle[0] = swizzle[1] = swizzle[2] = GL_ZERO; swizzle[3] = GL_RED;
			return true;
		case GL_LUMINANCE:
			upload = GL_RED; internalFormat = GL_R8;
			swizzle[0] = swizzle[1] = swizzle[2] = GL_RED; swizzle[3] = GL_ONE;
			return true;
		case GL_LUMINANCE_ALPHA:
			upload = GL_RG; internalFormat = GL_RG8;
			swizzle[0] = swizzle[1] = swizzle[2] = GL_RED; swizzle[3] = GL_GREEN;
			return true;
		case GL_INTENSITY: // not a pixel format in OpenGL 1.1, which left the texture empty
			upload = GL_RED; internalFormat = GL_R8;
			swizzle[0] = swizzle[1] = swizzle[2] = swizzle[3] = GL_RED;
			return true;
		case GL_COLOR_INDEX: // with the default index maps every index is 0, 0, 0, 0
			upload = GL_RED; internalFormat = GL_R8;
			swizzle[0] = swizzle[1] = swizzle[2] = swizzle[3] = GL_ZERO;
			return true;
		}
		return false;
	}
}

namespace ffp {
	bool init(int width, int height) {
		program = link(vertexShader, fragmentShader);
		depthProgram = link(depthVertexShader, depthFragmentShader);
		if (!program || !depthProgram) {
			shutdown();
			return false;
		}
#define QT_UNIFORM(name) uniforms.name = gl3::GetUniformLocation(program, #name);
		QT_UNIFORM(modelView) QT_UNIFORM(smoothShading) QT_UNIFORM(texturing) QT_UNIFORM(envMode) QT_UNIFORM(envColor)
		QT_UNIFORM(fogging) QT_UNIFORM(fogMode) QT_UNIFORM(fogParams) QT_UNIFORM(fogColor) QT_UNIFORM(alphaTesting)
		QT_UNIFORM(alphaFunction) QT_UNIFORM(alphaReference) QT_UNIFORM(clampRange)
		QT_UNIFORM(chromaMode) QT_UNIFORM(chromaLower) QT_UNIFORM(chromaUpper)
#undef QT_UNIFORM

		// The model view matrix of QuarkTex 0.53, built as glScalef and
		// glTranslatef built it: Warp3D pixels to -1..1, y downwards.
		GLfloat sx = 2.0f / static_cast<float>(width), sy = -2.0f / static_cast<float>(height);
		GLfloat tx = -(static_cast<float>(width) / 2.0f), ty = -(static_cast<float>(height) / 2.0f);
		const GLfloat matrix[16] = {sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, 0, sx * tx, sy * ty, 0, 1};
		memcpy(modelView, matrix, sizeof(modelView));
		gl3::UseProgram(depthProgram);
		gl3::UniformMatrix4fv(gl3::GetUniformLocation(depthProgram, "modelView"), 1, GL_FALSE, modelView);
		gl3::UseProgram(program);
		gl3::UniformMatrix4fv(uniforms.modelView, 1, GL_FALSE, modelView);

		gl3::GenVertexArrays(1, &vertexArray);
		gl3::BindVertexArray(vertexArray);
		gl3::GenBuffers(1, &vertexBuffer);
		gl3::BindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		gl3::VertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));
		gl3::VertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));
		gl3::VertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, texCoord)));
		gl3::EnableVertexAttribArray(0);
		gl3::EnableVertexAttribArray(1);
		gl3::EnableVertexAttribArray(2);

		State initial = {false, false, false, true, GL_ALWAYS, 0.0f, GL_EXP, 0.0f, 1.0f, 1.0f, {0, 0, 0, 0},
			GL_MODULATE, {0, 0, 0, 0}, false};
		state = initial;
		dirty = true;
		texturingSent = false;
		memset(textureSent, 0, sizeof(textureSent)); // not a set sendState sends
		const GLfloat white[4] = {1, 1, 1, 1}, origin[4] = {0, 0, 0, 1};
		memcpy(currentColor, white, sizeof(currentColor));
		memcpy(currentTexCoord, origin, sizeof(currentTexCoord));
		inside = false;
		vertices.clear();
		batch.clear();
		bound = 0;
		textures.clear();
		return true;
	}

	void shutdown() {
		if (vertexBuffer) gl3::DeleteBuffers(1, &vertexBuffer);
		if (vertexArray) gl3::DeleteVertexArrays(1, &vertexArray);
		if (program) gl3::DeleteProgram(program);
		if (depthProgram) gl3::DeleteProgram(depthProgram);
		vertexBuffer = vertexArray = program = depthProgram = 0;
		textures.clear();
		batch.clear();
	}

	void Begin(GLenum mode) {
		inside = true;
		primitive = mode;
		vertices.clear();
	}

	void End() {
		if (!inside) return;
		inside = false;
		size_t per = independent(primitive);
		if (!per) {
			flushBatch();
			if (!vertices.empty()) draw(primitive, &vertices[0], vertices.size());
			return;
		}
		if (batchMode != primitive) flushBatch();
		batchMode = primitive;
		// Only whole primitives, as OpenGL draws them.
		batch.insert(batch.end(), vertices.begin(), vertices.end() - vertices.size() % per);
		if (batch.size() >= batchLimit) flushBatch();
	}

	void flush() {
		flushBatch();
	}

	void Vertex2f(GLfloat x, GLfloat y) { addVertex(x, y, 0.0f); }
	void Vertex3f(GLfloat x, GLfloat y, GLfloat z) { addVertex(x, y, z); }

	void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		currentColor[0] = r;
		currentColor[1] = g;
		currentColor[2] = b;
		currentColor[3] = a;
	}
	void Color3f(GLfloat r, GLfloat g, GLfloat b) { Color4f(r, g, b, 1.0f); }
	void Color4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a) { Color4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }
	void Color3ub(GLubyte r, GLubyte g, GLubyte b) { Color4ub(r, g, b, 255); }

	void TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
		currentTexCoord[0] = s;
		currentTexCoord[1] = t;
		currentTexCoord[2] = r;
		currentTexCoord[3] = q;
	}
	void TexCoord2f(GLfloat s, GLfloat t) { TexCoord4f(s, t, 0.0f, 1.0f); }

	// glRect: a polygon in the current colour and texture coordinate.
	void Recti(GLint x1, GLint y1, GLint x2, GLint y2) {
		Begin(GL_TRIANGLE_FAN);
		Vertex2f(static_cast<GLfloat>(x1), static_cast<GLfloat>(y1));
		Vertex2f(static_cast<GLfloat>(x2), static_cast<GLfloat>(y1));
		Vertex2f(static_cast<GLfloat>(x2), static_cast<GLfloat>(y2));
		Vertex2f(static_cast<GLfloat>(x1), static_cast<GLfloat>(y2));
		End();
	}

	void Enable(GLenum cap) {
		flushBatch();
		switch (cap) {
		case GL_TEXTURE_2D: state.texture2D = true; dirty = true; break;
		case GL_FOG: state.fog = true; dirty = true; break;
		case GL_ALPHA_TEST: state.alphaTest = true; dirty = true; break;
		default: glEnable(cap); break;
		}
	}

	void Disable(GLenum cap) {
		flushBatch();
		switch (cap) {
		case GL_TEXTURE_2D: state.texture2D = false; dirty = true; break;
		case GL_FOG: state.fog = false; dirty = true; break;
		case GL_ALPHA_TEST: state.alphaTest = false; dirty = true; break;
		default: glDisable(cap); break;
		}
	}

	void ShadeModel(GLenum mode) {
		flushBatch();
		state.smooth = mode != GL_FLAT;
		dirty = true;
	}

	void AlphaFunc(GLenum function, GLclampf reference) {
		flushBatch();
		if (function < GL_NEVER || function > GL_ALWAYS) return;
		state.alphaFunction = function;
		state.alphaReference = clamp01(reference);
		dirty = true;
	}

	void Fogf(GLenum pname, GLfloat param) {
		flushBatch();
		switch (pname) {
		case GL_FOG_DENSITY: if (param >= 0.0f) state.fogDensity = param; break;
		case GL_FOG_START: state.fogStart = param; break;
		case GL_FOG_END: state.fogEnd = param; break;
		case GL_FOG_MODE: Fogi(pname, static_cast<GLint>(param)); return;
		}
		dirty = true;
	}

	void Fogi(GLenum pname, GLint param) {
		flushBatch();
		if (pname != GL_FOG_MODE) {
			Fogf(pname, static_cast<GLfloat>(param));
			return;
		}
		if (param == GL_LINEAR || param == GL_EXP || param == GL_EXP2) state.fogMode = static_cast<GLenum>(param);
		dirty = true;
	}

	void Fogfv(GLenum pname, const GLfloat* params) {
		flushBatch();
		if (pname != GL_FOG_COLOR) {
			Fogf(pname, params[0]);
			return;
		}
		for (int i = 0; i < 4; ++i) state.fogColor[i] = clamp01(params[i]);
		dirty = true;
	}

	void TexEnvi(GLenum target, GLenum pname, GLint param) {
		flushBatch();
		if (target != GL_TEXTURE_ENV || pname != GL_TEXTURE_ENV_MODE) return;
		if (param == GL_REPLACE || param == GL_MODULATE || param == GL_DECAL || param == GL_BLEND) state.envMode = static_cast<GLenum>(param);
		dirty = true;
	}

	void TexEnvfv(GLenum target, GLenum pname, const GLfloat* params) {
		flushBatch();
		if (target != GL_TEXTURE_ENV) return;
		if (pname == GL_TEXTURE_ENV_MODE) {
			TexEnvi(target, pname, static_cast<GLint>(params[0]));
			return;
		}
		if (pname != GL_TEXTURE_ENV_COLOR) return;
		for (int i = 0; i < 4; ++i) state.envColor[i] = clamp01(params[i]);
		dirty = true;
	}

	void ChromaTest(GLboolean enable) {
		flushBatch();
		state.chromaTest = enable != GL_FALSE;
	}

	void ChromaBounds(GLuint texture, GLint mode, GLuint lower, GLuint upper) {
		flushBatch();
		std::map<GLuint, TextureInfo>::iterator info = textures.find(texture);
		if (info == textures.end()) return;
		info->second.chromaMode = mode;
		for (int i = 0; i < 3; ++i) {
			info->second.chromaLower[i] = static_cast<int>((lower >> (16 - 8 * i)) & 0xFF);
			info->second.chromaUpper[i] = static_cast<int>((upper >> (16 - 8 * i)) & 0xFF);
		}
	}

	void GenTextures(GLsizei n, GLuint* names) {
		glGenTextures(n, names);
		for (GLsizei i = 0; i < n; ++i) {
			// GL_NEAREST_MIPMAP_LINEAR, GL_REPEAT, GL_LINEAR
			TextureInfo info = {false, true, false, false, true, 0, 0};
			textures[names[i]] = info;
		}
	}

	void DeleteTextures(GLsizei n, const GLuint* names) {
		flushBatch();
		glDeleteTextures(n, names);
		for (GLsizei i = 0; i < n; ++i) {
			textures.erase(names[i]);
			if (names[i] == bound) bound = 0;
		}
	}

	void BindTexture(GLenum target, GLuint texture) {
		flushBatch();
		glBindTexture(target, texture);
		if (target == GL_TEXTURE_2D) bound = texture;
	}

	void TexParameteri(GLenum target, GLenum pname, GLint param) {
		flushBatch();
		bool wrap = pname == GL_TEXTURE_WRAP_S || pname == GL_TEXTURE_WRAP_T;
		bool clamp = wrap && param == GL_CLAMP;
		glTexParameteri(target, pname, clamp ? GL_CLAMP_TO_BORDER : param);
		if (target != GL_TEXTURE_2D || !textures.count(bound)) return;
		TextureInfo& info = textures[bound];
		switch (pname) {
		case GL_TEXTURE_MIN_FILTER: info.mipmapFilter = param != GL_NEAREST && param != GL_LINEAR; break;
		case GL_TEXTURE_MAG_FILTER: info.magLinear = param == GL_LINEAR; break;
		case GL_TEXTURE_WRAP_S: info.clampS = clamp; break;
		case GL_TEXTURE_WRAP_T: info.clampT = clamp; break;
		}
	}

	void TexParameterfv(GLenum target, GLenum pname, const GLfloat* params) {
		flushBatch();
		glTexParameterfv(target, pname, params);
	}

	void TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border,
			GLenum format, GLenum type, const GLvoid* pixels) {
		flushBatch();
		GLenum upload = format;
		GLint swizzle[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
		if (!legacyFormat(format, upload, internalFormat, swizzle)) internalFormat = GL_RGBA8;
		glTexImage2D(target, level, internalFormat, width, height, border, upload, type, pixels);
		glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, swizzle[0]);
		glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, swizzle[1]);
		glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, swizzle[2]);
		glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, swizzle[3]);
		if (target == GL_TEXTURE_2D && level == 0 && textures.count(bound)) {
			TextureInfo& info = textures[bound];
			info.image = width > 0 && height > 0;
			info.width = width;
			info.height = height;
			dirty = true;
		}
	}

	void TexSubImage2D(GLenum target, GLint level, GLint x, GLint y, GLsizei width, GLsizei height,
			GLenum format, GLenum type, const GLvoid* pixels) {
		flushBatch();
		GLenum upload = format;
		GLint internalFormat, swizzle[4];
		legacyFormat(format, upload, internalFormat, swizzle);
		glTexSubImage2D(target, level, x, y, width, height, upload, type, pixels);
	}

	void DepthPoints(GLsizei count, const GLfloat* xyz) {
		flushBatch();
		if (count <= 0 || !program) return;
		GLboolean colorMask[4], depthMask;
		GLint depthFunction;
		GLfloat pointSize;
		GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
		glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
		glGetIntegerv(GL_DEPTH_FUNC, &depthFunction);
		glGetFloatv(GL_POINT_SIZE, &pointSize);

		std::vector<Vertex> points(static_cast<size_t>(count));
		for (GLsizei i = 0; i < count; ++i) {
			Vertex v = {{xyz[3 * i], xyz[3 * i + 1], xyz[3 * i + 2], 1.0f}, {0, 0, 0, 0}, {0, 0, 0, 1}};
			points[i] = v;
		}
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		glDepthMask(GL_TRUE);
		glPointSize(1.0f);
		gl3::UseProgram(depthProgram);
		gl3::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(points.size() * sizeof(Vertex)), &points[0], GL_STREAM_DRAW);
		glDrawArrays(GL_POINTS, 0, count);
		gl3::UseProgram(program);

		glPointSize(pointSize);
		glDepthMask(depthMask);
		glDepthFunc(static_cast<GLenum>(depthFunction));
		if (!depthTest) glDisable(GL_DEPTH_TEST);
		glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
	}

	// Points with the stencil test passing always and replacing with the
	// reference value; one draw call per run of equal values. The depth test
	// is off, so nothing else decides about the write (and no depth is
	// written).
	void StencilPoints(GLsizei count, const GLfloat* xy, const GLuint* values) {
		flushBatch();
		if (count <= 0 || !program) return;
		GLboolean colorMask[4];
		GLint function, reference, valueMask, sfail, dpfail, dppass;
		GLfloat pointSize;
		GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST), stencilTest = glIsEnabled(GL_STENCIL_TEST);
		glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
		glGetIntegerv(GL_STENCIL_FUNC, &function);
		glGetIntegerv(GL_STENCIL_REF, &reference);
		glGetIntegerv(GL_STENCIL_VALUE_MASK, &valueMask);
		glGetIntegerv(GL_STENCIL_FAIL, &sfail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &dpfail);
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &dppass);
		glGetFloatv(GL_POINT_SIZE, &pointSize);

		std::vector<Vertex> points(static_cast<size_t>(count));
		for (GLsizei i = 0; i < count; ++i) {
			Vertex v = {{xy[2 * i], xy[2 * i + 1], 0.0f, 1.0f}, {0, 0, 0, 0}, {0, 0, 0, 1}};
			points[i] = v;
		}
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glPointSize(1.0f);
		gl3::UseProgram(depthProgram);
		gl3::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(points.size() * sizeof(Vertex)), &points[0], GL_STREAM_DRAW);
		for (GLsizei first = 0, end; first < count; first = end) {
			for (end = first + 1; end < count && values[end] == values[first]; ++end) {}
			glStencilFunc(GL_ALWAYS, static_cast<GLint>(values[first]), ~0u);
			glDrawArrays(GL_POINTS, first, end - first);
		}
		gl3::UseProgram(program);

		glPointSize(pointSize);
		glStencilOp(static_cast<GLenum>(sfail), static_cast<GLenum>(dpfail), static_cast<GLenum>(dppass));
		glStencilFunc(static_cast<GLenum>(function), reference, static_cast<GLuint>(valueMask));
		if (!stencilTest) glDisable(GL_STENCIL_TEST);
		if (depthTest) glEnable(GL_DEPTH_TEST);
		glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
	}
}
