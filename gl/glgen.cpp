// Generates both ends of the OpenGL command buffer from glFuncs.txt.
//
// Every function gets an opcode (its 1-based line number in glFuncs.txt) and
// is encoded as one header word, (opcode << 16) | word count, followed by one
// 32-bit word per argument; doubles take two words, high word first. Pointers
// are Amiga addresses. Calls that take pointers or return a value are
// synchronous: the 68k side flushes the buffer right after writing them, and
// the value of the last command is returned from the flush.
//
// Usage: glgen <glFuncs.txt> <output directory>
// Writes:
//   gl/gldeclarations.auto.h      prototypes of the 68k _glXxx functions
//   gl/glencode.auto.c            68k encoders, included by gl/gl.c
//   host/gldecode.auto.inc        host dispatch, included by host/gldecode.cpp
//   tests/host/glstubs.auto.inc   recording OpenGL stubs for the unit test
//   tests/host/glcalls.auto.inc   one test call per function for the unit test

#include <cstdio>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using std::string;
using std::vector;

namespace {
	struct Arg {
		string type; // as in glFuncs.txt, without the GL prefix: "float", "uint*"
		string name;
		bool pointer() const { return type.find('*') != string::npos; }
		bool isDouble() const { return !pointer() && (type == "double" || type == "clampd"); }
		bool isFloat() const { return !pointer() && (type == "float" || type == "clampf"); }
		bool isSigned() const { return !pointer() && (type == "int" || type == "sizei" || type == "short" || type == "byte"); }
		int words() const { return isDouble() ? 2 : 1; }
		string glType() const { return "GL" + type; }
	};

	struct Function {
		int opcode;
		string returnType;
		string name;
		vector<Arg> args;

		bool returns() const { return returnType != "void"; }
		bool hasPointer() const {
			for (size_t i = 0; i < args.size(); ++i) if (args[i].pointer()) return true;
			return false;
		}
		// Finish and Flush wait for the host anyway.
		bool synchronous() const { return returns() || hasPointer() || name == "Finish" || name == "Flush"; }
		int words() const {
			int count = 1;
			for (size_t i = 0; i < args.size(); ++i) count += args[i].words();
			return count;
		}
		string glReturnType() const { return "GL" + returnType; }
		string parameters() const {
			if (args.empty()) return "void";
			string list;
			for (size_t i = 0; i < args.size(); ++i) {
				if (i) list += ", ";
				list += args[i].glType() + " " + args[i].name;
			}
			return list;
		}
	};

	// Implemented by hand in host/gldecode.cpp. The host keeps the Amiga
	// addresses of the client arrays and the feedback/selection buffers so
	// that GetPointerv can return them; GetString would return a host pointer.
	// DrawBuffer and ReadBuffer name the framebuffer object's colour buffer
	// for the front and back buffers when the context draws offscreen.
	const std::set<string> manual = {
		"GetPointerv", "VertexPointer", "NormalPointer", "ColorPointer", "IndexPointer",
		"TexCoordPointer", "EdgeFlagPointer", "InterleavedArrays", "FeedbackBuffer",
		"SelectBuffer", "GetString", "DrawBuffer", "ReadBuffer"
	};

	string trim(const string& s) {
		size_t begin = s.find_first_not_of(" \t\r");
		size_t end = s.find_last_not_of(" \t\r");
		return begin == string::npos ? string() : s.substr(begin, end - begin + 1);
	}

	// "type name" where the type may carry '*' on either side of the space.
	Arg parseArg(const string& text) {
		string s = trim(text);
		size_t split = s.find_last_of(" *");
		if (split == string::npos) throw std::runtime_error("argument without a name: " + s);
		Arg arg;
		arg.name = trim(s.substr(split + 1));
		arg.type = trim(s.substr(0, split + 1));
		string compact;
		for (size_t i = 0; i < arg.type.size(); ++i) if (arg.type[i] != ' ') compact += arg.type[i];
		arg.type = compact;
		return arg;
	}

	vector<Function> parse(const string& path) {
		std::ifstream in(path.c_str());
		if (!in) throw std::runtime_error("cannot open " + path);
		vector<Function> functions;
		string line;
		int number = 0;
		while (std::getline(in, line)) {
			++number;
			line = trim(line);
			if (line.empty()) throw std::runtime_error("empty line " + std::to_string(number) + ": opcodes are line numbers");
			size_t open = line.find('('), close = line.rfind(')');
			if (open == string::npos || close == string::npos || line.substr(close + 1).find(';') == string::npos)
				throw std::runtime_error("line " + std::to_string(number) + ": expected 'type Name(args);'");
			Function f;
			f.opcode = number;
			string head = trim(line.substr(0, open));
			size_t split = head.find_last_of(" *");
			f.name = trim(head.substr(split + 1));
			f.returnType = trim(head.substr(0, split + 1));
			string compact;
			for (size_t i = 0; i < f.returnType.size(); ++i) if (f.returnType[i] != ' ') compact += f.returnType[i];
			f.returnType = compact;
			string list = trim(line.substr(open + 1, close - open - 1));
			std::stringstream args(list);
			string item;
			while (!list.empty() && std::getline(args, item, ',')) f.args.push_back(parseArg(item));
			functions.push_back(f);
		}
		return functions;
	}

	std::ofstream create(const string& path) {
		std::ofstream out(path.c_str());
		if (!out) throw std::runtime_error("cannot write " + path);
		out << "/* Generated by gl/glgen.cpp from gl/glFuncs.txt - do not edit */\n";
		return out;
	}

	void declarations(const vector<Function>& functions, const string& path) {
		std::ofstream out = create(path);
		for (size_t i = 0; i < functions.size(); ++i) {
			const Function& f = functions[i];
			out << f.glReturnType() << " _gl" << f.name << "(" << f.parameters() << ");\n";
		}
	}

	// 68k side. gl/gl.c provides qt_reserve(), qt_flush(), qt_f2l(),
	// qt_dhi(), qt_dlo() and QT_ADDRESS().
	void encoders(const vector<Function>& functions, const string& path) {
		std::ofstream out = create(path);
		for (size_t i = 0; i < functions.size(); ++i) {
			const Function& f = functions[i];
			out << f.glReturnType() << " _gl" << f.name << "(" << f.parameters() << ") {\n";
			// qt_ prefixes keep the locals apart from parameters such as w or r.
			out << "\tULONG *qt_w = qt_reserve(" << f.words() << ");\n";
			out << "\tqt_w[0] = (" << f.opcode << "UL << 16) | " << f.words() << ";\n";
			int word = 1;
			for (size_t a = 0; a < f.args.size(); ++a) {
				const Arg& arg = f.args[a];
				if (arg.isDouble()) {
					out << "\tqt_w[" << word << "] = qt_dhi(" << arg.name << ");\n";
					out << "\tqt_w[" << word + 1 << "] = qt_dlo(" << arg.name << ");\n";
				}
				else if (arg.isFloat()) out << "\tqt_w[" << word << "] = qt_f2l(" << arg.name << ");\n";
				else if (arg.pointer()) out << "\tqt_w[" << word << "] = QT_ADDRESS(" << arg.name << ");\n";
				else out << "\tqt_w[" << word << "] = (ULONG) " << arg.name << ";\n";
				word += arg.words();
			}
			if (f.returns()) out << "\treturn (" << f.glReturnType() << ") qt_flush();\n";
			else if (f.synchronous()) out << "\tqt_flush();\n";
			out << "}\n";
		}
	}

	string decodeArg(const Arg& arg, int word) {
		std::ostringstream s;
		if (arg.isDouble()) s << "c.d(" << word << ")";
		else if (arg.isFloat()) s << "c.f(" << word << ")";
		else if (arg.pointer()) s << "c.p(" << word << ")";
		else if (arg.isSigned()) s << "(" << arg.glType() << ") (int32_t) c.u(" << word << ")";
		else s << "(" << arg.glType() << ") c.u(" << word << ")";
		return s.str();
	}

	// Host side: one case per opcode. host/gldecode.cpp provides the Command
	// accessors c.u/f/d/p and QT_GL(name), which is gl##name in the library and
	// a recording stub in the unit test.
	void decoder(const vector<Function>& functions, const string& path) {
		std::ofstream out = create(path);
		for (size_t i = 0; i < functions.size(); ++i) {
			const Function& f = functions[i];
			out << "case " << f.opcode << ": /* " << f.name << " */\n";
			out << "\tif (c.words != " << f.words() << ") return qt_bad_command(c);\n";
			if (manual.count(f.name)) {
				out << "\tresult = qt_manual_" << f.name << "(c);\n\tbreak;\n";
				continue;
			}
			out << "\t";
			if (f.returns()) out << "result = (int32_t) ";
			out << "QT_GL(" << f.name << ")(";
			int word = 1;
			for (size_t a = 0; a < f.args.size(); ++a) {
				if (a) out << ", ";
				out << decodeArg(f.args[a], word);
				word += f.args[a].words();
			}
			out << ");\n\tbreak;\n";
		}
	}

	// Test values: distinct per argument position and type, exactly
	// representable, negative for signed types.
	string testValue(const Arg& arg, int position, bool direct) {
		std::ostringstream s;
		if (arg.pointer()) {
			unsigned address = 0x10000 + 0x100 * position;
			if (direct) s << "(" << arg.glType() << ") (arena + 0x" << std::hex << address << ")";
			else s << "(" << arg.glType() << ") (uintptr_t) 0x" << std::hex << address;
		}
		else if (arg.isDouble()) s << "-(1e10 + " << position << " + 0.125)";
		else if (arg.isFloat()) s << "(GLfloat) (" << (0.5 + 1.25 * position) << ")";
		else if (arg.type == "int" || arg.type == "sizei") s << "-(1000 + " << position << ")";
		else if (arg.type == "short") s << "(GLshort) -(100 + " << position << ")";
		else if (arg.type == "byte") s << "(GLbyte) -(10 + " << position << ")";
		else if (arg.type == "ushort") s << "(GLushort) (60000 + " << position << ")";
		else if (arg.type == "ubyte") s << "(GLubyte) (200 + " << position << ")";
		else if (arg.type == "boolean") s << "(GLboolean) " << (position & 1);
		else s << "(" << arg.glType() << ") (0x1000 + 7 * " << position << ")";
		return s.str();
	}

	void stubs(const vector<Function>& functions, const string& path) {
		std::ofstream out = create(path);
		for (size_t i = 0; i < functions.size(); ++i) {
			const Function& f = functions[i];
			out << f.glReturnType() << " stub_gl" << f.name << "(" << f.parameters() << ") {\n";
			out << "\tRecord qt_record(\"" << f.name << "\");\n";
			for (size_t a = 0; a < f.args.size(); ++a) out << "\tqt_record << " << f.args[a].name << ";\n";
			if (f.returns()) out << "\treturn (" << f.glReturnType() << ") " << f.opcode << ";\n";
			out << "}\n";
		}
	}

	void calls(const vector<Function>& functions, const string& path) {
		std::ofstream out = create(path);
		for (size_t i = 0; i < functions.size(); ++i) {
			const Function& f = functions[i];
			if (manual.count(f.name)) continue;
			// The stubs return their opcode, converted to the return type.
			out << "{\"" << f.name << "\", " << (f.synchronous() ? "true" : "false") << ", ";
			if (f.returns()) out << "(int32_t) (" << f.glReturnType() << ") " << f.opcode << ",\n";
			else out << "0,\n";
			// The call through the encoder and decoder, then the same call made
			// directly on the stub; both must record the same arguments.
			for (int direct = 0; direct < 2; ++direct) {
				out << "\t[]() -> int32_t { " << (f.returns() ? "return (int32_t) " : "")
					<< (direct ? "stub_gl" : "_gl") << f.name << "(";
				for (size_t a = 0; a < f.args.size(); ++a) {
					if (a) out << ", ";
					out << testValue(f.args[a], static_cast<int>(a) + 1, direct != 0);
				}
				out << (f.returns() ? "); }" : "); return 0; }") << (direct ? "},\n" : ",\n");
			}
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "usage: glgen <glFuncs.txt> <repository root>" << std::endl;
		return 1;
	}
	try {
		vector<Function> functions = parse(argv[1]);
		string root = string(argv[2]) + "/";
		declarations(functions, root + "gl/gldeclarations.auto.h");
		encoders(functions, root + "gl/glencode.auto.c");
		decoder(functions, root + "host/gldecode.auto.inc");
		stubs(functions, root + "tests/host/glstubs.auto.inc");
		calls(functions, root + "tests/host/glcalls.auto.inc");
		int synchronous = 0;
		for (size_t i = 0; i < functions.size(); ++i) synchronous += functions[i].synchronous();
		std::cout << functions.size() << " functions, " << functions.size() - synchronous << " queued, "
			<< synchronous << " synchronous" << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "glgen: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
